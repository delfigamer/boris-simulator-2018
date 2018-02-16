#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <random>
#include <ctime>
#include "lin.hpp"
#include "parser.hpp"

struct linear_model_tf
{
	std::vector< double > ssmat;
	std::vector< double > simat;
	std::vector< double > osmat;
	std::vector< double > oimat;

	void step(
		std::vector< double > const& input,
		std::vector< double >& state,
		std::vector< double >& temp )
	{
		lin::inner( ssmat, state, temp );
		lin::inneradd( simat, input, temp );
		std::swap( state, temp );
	}

	void step(
		std::vector< double > const& input,
		std::vector< double >& state,
		std::vector< double >& output,
		std::vector< double >& temp )
	{
		lin::inner( osmat, state, output );
		lin::inneradd( oimat, input, output );
		step( input, state, temp );
	}
};

struct linear_model_inst
{
	linear_model_tf& tf;
	std::vector< double > state;
	std::vector< double > temp;

	void step(
		std::vector< double > const& input )
	{
		tf.step( input, state, temp );
	}

	void step(
		std::vector< double > const& input,
		std::vector< double >& output )
	{
		tf.step( input, state, output, temp );
	}
};

static void getnumber(
	parser::table_t& params,
	char const* varname, char const* fullname,
	double& numberv )
{
	params.get( varname,
		parser::vtag::number, [ & ]( parser::value_t& value )
		{
			numberv = value.number();
		},
		[ & ]( parser::value_t& value )
		{
			throw std::runtime_error(
				std::string( fullname ) + " (" +
				std::string( varname ) + ") must be provided as a number" );
		} );
}

static void getsignal(
	parser::table_t& params,
	char const* varname, char const* fullname,
	parser::matrix_t& signalv )
{
	params.get( varname,
		parser::vtag::matrix, [ & ]( parser::value_t& value )
		{
			signalv = std::move( value.matrix() );
		},
		[ & ]( parser::value_t& value )
		{
			throw std::runtime_error(
				std::string( fullname ) + " (" +
				std::string( varname ) + ") must be provided as a matrix" );
		} );
}

static void getmatrix(
	parser::table_t& params,
	char const* varname, char const* fullname,
	size_t rows, size_t cols,
	parser::matrix_t& matrixv )
{
	static std::mt19937 randsource( time( nullptr ) );
	params.get( varname,
		parser::vtag::matrix, [ & ]( parser::value_t& value )
		{
			matrixv = std::move( value.matrix() );
			if( matrixv.rows != rows || matrixv.cols != cols )
			{
				throw std::runtime_error(
					std::string( fullname ) + " (" +
					std::string( varname ) + ") has invalid dimensions" );
			}
		},
		parser::vtag::generator, [ & ]( parser::value_t& value )
		{
			matrixv = parser::matrix_t{
				std::vector< double >( rows * cols ),
				rows, cols };
			parser::generator_t& gen = value.generator();
			if( gen.name == "muniform" )
			{
				double a, b;
				if( gen.args.size() == 0 )
				{
					a = 0;
					b = 1;
				}
				else if( gen.args.size() == 1 )
				{
					a = 0;
					b = gen.args[ 0 ];
				}
				else if( gen.args.size() == 2 )
				{
					a = gen.args[ 0 ];
					b = gen.args[ 1 ];
				}
				else
				{
					throw std::runtime_error(
						"muniform() generator expects up to 2 arguments" );
				}
				std::uniform_real_distribution< double > dist( a, b );
				std::generate(
					matrixv.elems.begin(), matrixv.elems.end(),
					[ & ]()
					{
						return dist( randsource );
					} );
			}
			else if( gen.name == "mnormal" )
			{
				double mean, stddev;
				if( gen.args.size() == 0 )
				{
					mean = 0;
					stddev = 1;
				}
				else if( gen.args.size() == 1 )
				{
					mean = 0;
					stddev = gen.args[ 0 ];
				}
				else if( gen.args.size() == 2 )
				{
					mean = gen.args[ 0 ];
					stddev = gen.args[ 1 ];
				}
				else
				{
					throw std::runtime_error(
						"mnormal() generator expects up to 2 arguments" );
				}
				std::normal_distribution< double > dist( mean, stddev );
				std::generate(
					matrixv.elems.begin(), matrixv.elems.end(),
					[ & ]()
					{
						return dist( randsource );
					} );
			}
			else
			{
				throw std::runtime_error(
					std::string( fullname ) + " (" +
					std::string( varname ) + ") is given an invalid generator" );
			}
		},
		[ & ]( parser::value_t& value )
		{
			throw std::runtime_error(
				std::string( fullname ) + " (" +
				std::string( varname ) + ") must be provided" );
		} );
}

template< typename TA >
TA mmax( TA const& a )
{
	return a;
}

template< typename TA, typename... Ts >
TA mmax( TA const& a, TA const& b, Ts const&... rest )
{
	return mmax( std::max( a, b ), rest... );
}

static void main_body( int argc, char const* const* argv )
{
	using namespace parser;

	double timev;
	matrix_t inputv, noisev, statev, outputv;
	size_t modelcount;
	matrix_t ssmatv, simatv, snmatv, osmatv, oimatv, onmatv;

	table_t params;
	params.loadfile( argc >= 2 ? argv[ 1 ] : "input.txt" );

	getsignal( params, "U", "control input", inputv );
	getsignal( params, "N", "noise input", noisev );
	getsignal( params, "X", "initial state", statev );
	getsignal( params, "Y", "initial output", outputv );
	modelcount = mmax( inputv.rows, noisev.rows, statev.rows, outputv.rows );
	if(
		( inputv.rows != 0 && inputv.rows != modelcount ) ||
		( noisev.rows != 0 && noisev.rows != modelcount ) ||
		( statev.rows != 0 && statev.rows != modelcount ) ||
		( outputv.rows != 0 && outputv.rows != modelcount ) )
	{
		throw std::runtime_error(
			"signal matrices (U, N, X, Y) must have the same number of rows" );
	}
	getnumber( params, "T", "simulation step count", timev );
	getmatrix(
		params, "A", "state-to-state matrix",
		statev.cols, statev.cols, ssmatv );
	getmatrix(
		params, "B", "input-to-state matrix",
		statev.cols, inputv.cols, simatv );
	getmatrix(
		params, "E", "noise-to-state matrix",
		statev.cols, noisev.cols, snmatv );
	getmatrix(
		params, "C", "state-to-output matrix",
		outputv.cols, statev.cols, osmatv );
	getmatrix(
		params, "D", "input-to-output matrix",
		outputv.cols, inputv.cols, oimatv );
	getmatrix(
		params, "F", "noise-to-output matrix",
		outputv.cols, noisev.cols, onmatv );
	linear_model_tf ltf{
		ssmatv.elems,
		lin::matrixconcat(
			simatv.elems, snmatv.elems, simatv.cols, snmatv.cols ),
		osmatv.elems,
		lin::matrixconcat(
			oimatv.elems, onmatv.elems, oimatv.cols, onmatv.cols ) };
	std::cout
		<< "A = "
		<< lin::vform( ssmatv.elems, ssmatv.cols ) << "\n"
		<< "B = "
		<< lin::vform( simatv.elems, simatv.cols ) << "\n"
		<< "E = "
		<< lin::vform( snmatv.elems, snmatv.cols ) << "\n"
		<< "C = "
		<< lin::vform( osmatv.elems, ssmatv.cols ) << "\n"
		<< "D = "
		<< lin::vform( oimatv.elems, simatv.cols ) << "\n"
		<< "F = "
		<< lin::vform( onmatv.elems, snmatv.cols ) << "\n";
	matrix_t resultv{
		std::vector< double >( outputv.rows * outputv.cols ),
		outputv.rows, outputv.cols };
	std::fstream historyfile(
		argc >= 4 ? argv[ 3 ] : "history.txt",
		std::ios::out | std::ios::trunc );
	for( size_t modelindex = 0; modelindex < modelcount; ++modelindex )
	{
		std::vector< double > state(
			statev.elems.data() + statev.cols * modelindex,
			statev.elems.data() + statev.cols * ( modelindex + 1 ) );
		std::vector< double > input(
			inputv.elems.data() + inputv.cols * modelindex,
			inputv.elems.data() + inputv.cols * ( modelindex + 1 ) );
		input.insert( input.end(),
			noisev.elems.data() + noisev.cols * modelindex,
			noisev.elems.data() + noisev.cols * ( modelindex + 1 ) );
		linear_model_inst inst{ ltf, std::move( state ) };
		inst.temp.resize( inst.state.size() );
		std::vector< double > output( outputv.cols );
		for( int time = 1; time <= timev; ++time )
		{
			inst.step( input, output );
			historyfile
				<< modelindex << "\t" << time
				<< lin::tform( output )
				<< lin::tform( inst.state ) << "\n";
		}
		std::copy(
			output.begin(), output.end(),
			resultv.elems.data() + resultv.cols * modelindex );
	}
	{
		std::fstream outputfile(
			argc >= 3 ? argv[ 2 ] : "output.txt",
			std::ios::out | std::ios::trunc );
		outputfile << "Y = " << lin::vform( resultv.elems, resultv.cols ) << "\n";
	}
}

int main( int argc, char const* const* argv )
{
	try
	{
		main_body( argc, argv );
		return 0;
	}
	catch( std::exception const& e )
	{
		std::cout << e.what() << "\n";
		return 1;
	}
}
