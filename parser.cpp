#include "parser.hpp"
#include <memory>
#include <stdexcept>
#include <cstdio>
#include <cassert>

namespace parser
{
	value_t::value_t( vtag inittag )
		: m_tag( vtag::nil )
	{
		if( inittag != vtag::nil )
		{
			settag( inittag );
		}
	}

	value_t::value_t( value_t const& other )
		: value_t()
	{
		*this = other;
	}

	value_t::value_t( value_t&& other )
		: value_t()
	{
		*this = std::move( other );
	}

	value_t& value_t::operator=( value_t const& other )
	{
		settag( other.m_tag );
		switch( m_tag )
		{
		case vtag::nil:
			break;

		case vtag::number:
			m_number = other.m_number;
			break;

		case vtag::matrix:
			m_matrix = other.m_matrix;
			break;

		case vtag::generator:
			m_generator = other.m_generator;
			break;
		}
		return *this;
	}

	value_t& value_t::operator=( value_t&& other )
	{
		settag( other.m_tag );
		switch( m_tag )
		{
		case vtag::nil:
			break;

		case vtag::number:
			m_number = std::move( other.m_number );
			break;

		case vtag::matrix:
			m_matrix = std::move( other.m_matrix );
			break;

		case vtag::generator:
			m_generator = std::move( other.m_generator );
			break;
		}
		return *this;
	}

	value_t::~value_t()
	{
		settag( vtag::nil );
	}

	void value_t::settag( vtag newtag )
	{
		if( newtag == m_tag )
		{
			return;
		}
		switch( m_tag )
		{
		case vtag::nil:
		case vtag::number:
			break;

		case vtag::matrix:
			m_matrix.~matrix_t();
			break;

		case vtag::generator:
			m_generator.~generator_t();
			break;
		}
		m_tag = newtag;
		switch( m_tag )
		{
		case vtag::nil:
		case vtag::number:
			break;

		case vtag::matrix:
			new( &m_matrix )matrix_t();
			break;

		case vtag::generator:
			new( &m_generator )generator_t();
			break;
		}
	}

	vtag value_t::gettag()
	{
		return m_tag;
	}

	double& value_t::number()
	{
		assert( m_tag == vtag::number );
		return m_number;
	}

	matrix_t& value_t::matrix()
	{
		assert( m_tag == vtag::matrix );
		return m_matrix;
	}

	generator_t& value_t::generator()
	{
		assert( m_tag == vtag::generator );
		return m_generator;
	}

	value_t::operator bool()
	{
		return m_tag != vtag::nil;
	}

	struct token_t
	{
		enum
		{
			name = -1,
			number = -2,
			eof = -3,
		};
		int tag;
		std::string payload;

		double n() const
		{
			double ret;
			if( sscanf( payload.c_str(), "%lf", &ret ) == 1 )
			{
				return ret;
			}
			else
			{
				throw std::runtime_error( "invalid number token" );
			}
		}
	};

	class tokenstream_t
	{
	private:
		FILE* m_file;
		int m_ch;
		token_t m_token;

		void advancech();

	public:
		tokenstream_t( char const* path );
		~tokenstream_t() = default;

		void advance();
		token_t const& current();
	};

	void tokenstream_t::advancech()
	{
		m_ch = fgetc( m_file );
	}

	tokenstream_t::tokenstream_t( char const* path )
	{
		m_file = fopen( path, "r" );
		if( !m_file )
		{
			perror( "" );
		}
		advancech();
		advance();
	}

	void tokenstream_t::advance()
	{
		m_token.payload.clear();
		while( m_ch >= 0 && m_ch <= ' ' )
		{
			advancech();
		}
		if( m_ch < 0 )
		{
			m_token.tag = token_t::eof;
		}
		else if(
			( m_ch >= '0' && m_ch <= '9' ) ||
			m_ch == '.' || m_ch == '-' )
		{
			m_token.tag = token_t::number;
			while(
				( m_ch >= '0' && m_ch <= '9' ) ||
				( m_ch >= 'A' && m_ch <= 'Z' ) ||
				( m_ch >= 'a' && m_ch <= 'z' ) ||
				m_ch == '.' || m_ch == '+' || m_ch == '-' )
			{
				m_token.payload.push_back( m_ch );
				advancech();
			}
		}
		else if(
			( m_ch >= 'A' && m_ch <= 'Z' ) ||
			( m_ch >= 'a' && m_ch <= 'z' ) ||
			m_ch == '_' )
		{
			m_token.tag = token_t::name;
			while(
				( m_ch >= '0' && m_ch <= '9' ) ||
				( m_ch >= 'A' && m_ch <= 'Z' ) ||
				( m_ch >= 'a' && m_ch <= 'z' ) ||
				m_ch == '_' )
			{
				m_token.payload.push_back( m_ch );
				advancech();
			}
		}
		else
		{
			m_token.tag = m_ch;
			advancech();
		}
	}

	token_t const& tokenstream_t::current()
	{
		return m_token;
	}

	value_t read_value( tokenstream_t& ts )
	{
		switch( ts.current().tag )
		{
		case token_t::number:
		{
			value_t value( vtag::number );
			value.number() = ts.current().n();
			ts.advance();
			return value;
		}

		case token_t::name:
		{
			value_t value( vtag::generator );
			generator_t& gen = value.generator();
			gen.name = std::move( ts.current().payload );
			ts.advance();
			if( ts.current().tag != '(' )
			{
				throw std::runtime_error(
					"expected ( after a generator name" );
			}
			ts.advance();
			if( ts.current().tag == ')' )
			{
				ts.advance();
				return value;
			}
			while( true )
			{
				if( ts.current().tag != token_t::number )
				{
					throw std::runtime_error(
						"expected a number as the generator argument" );
				}
				gen.args.push_back( ts.current().n() );
				ts.advance();
				if( ts.current().tag == ')' )
				{
					ts.advance();
					break;
				}
				else if( ts.current().tag == ',' )
				{
					ts.advance();
				}
				else
				{
					throw std::runtime_error(
						"expected ) after the generator arguments" );
				}
			}
			return value;
		}

		case '[':
		{
			value_t value( vtag::matrix );
			matrix_t& matrix = value.matrix();
			ts.advance();
			matrix.rows = 0;
			matrix.cols = 0;
			size_t col = 0;
			while( true )
			{
				switch( ts.current().tag )
				{
				case token_t::number:
					matrix.elems.push_back( ts.current().n() );
					col += 1;
					ts.advance();
					break;

				case ';':
				case ']':
					if( col != 0 )
					{
						if( matrix.rows == 0 )
						{
							matrix.rows = 1;
							matrix.cols = col;
						}
						else
						{
							if( col != matrix.cols )
							{
								throw std::runtime_error( "matrix rows are uneven" );
							}
							matrix.rows += 1;
						}
					}
					if( ts.current().tag == ']' )
					{
						ts.advance();
						return value;
					}
					else
					{
						col = 0;
						ts.advance();
						break;
					}

				case ',':
					ts.advance();
					break;

				default:
					throw std::runtime_error( "failed to parse a matrix element" );
				}
			}
		}

		default:
			throw std::runtime_error( "failed to parse a value" );
		}
	}

	void table_t::loadfile( char const* path )
	{
		tokenstream_t ts( path );
		while( ts.current().tag != token_t::eof )
		{
			if( ts.current().tag != token_t::name )
			{
				throw std::runtime_error( "failed to parse a statement" );
			}
			std::string target = ts.current().payload;
			ts.advance();
			if( ts.current().tag == '=' )
			{
				ts.advance();
				m_names[ std::move( target ) ] = read_value( ts );
			}
			else
			{
				throw std::runtime_error( "failed to parse a statement" );
			}
		}
	}
}
