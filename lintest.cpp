#include <iostream>
#include <exception>
#include "lin.hpp"

static void main_body( int argc, char const* const* argv )
{
	std::vector< double > amat = {
		2, 9, 1, 2,
		2, 0, 7, 1,
		1, 2, 5, 6,
		8, 4, 0, 1 };
	std::vector< double > f = lin::matrixpolynomial( amat, 4 );
	std::cout << lin::vform( f );
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
