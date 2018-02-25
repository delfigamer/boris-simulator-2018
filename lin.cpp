#include "lin.hpp"
#include <cassert>

namespace lin
{
	void add(
		std::vector< double > const& a,
		std::vector< double > const& b,
		std::vector< double >& r )
	{
		assert( a.size() == b.size() && a.size() == r.size() );
		auto ait = a.begin();
		auto bit = b.begin();
		auto rit = r.begin();
		while( rit != r.end() )
		{
			*rit = *ait + *bit;
			++ait;
			++bit;
			++rit;
		}
	}

	void add(
		std::vector< double > const& a,
		std::vector< double >& r )
	{
		assert( a.size() == r.size() );
		auto ait = a.begin();
		auto rit = r.begin();
		while( rit != r.end() )
		{
			*rit += *ait;
			++ait;
			++rit;
		}
	}

	void zero(
		std::vector< double >& r )
	{
		for( double& v : r )
		{
			v = 0;
		}
	}

	void identity(
		double scale,
		size_t size,
		std::vector< double >& r )
	{
		assert( r.size() == size * size );
		auto rit = r.begin();
		if( rit == r.end() )
		{
			return;
		}
		*rit = scale;
		++rit;
		while( rit != r.end() )
		{
			for( size_t i = 0; i < size; ++i )
			{
				*rit = 0;
				++rit;
			}
			*rit = scale;
			++rit;
		}
	}

	void identityadd(
		double scale,
		size_t size,
		std::vector< double >& r )
	{
		assert( r.size() == size * size );
		auto rit = r.begin();
		if( rit == r.end() )
		{
			return;
		}
		*rit += scale;
		++rit;
		while( rit != r.end() )
		{
			rit += size;
			*rit += scale;
			++rit;
		}
	}

	double trace(
		std::vector< double > const& a,
		size_t size )
	{
		assert( a.size() == size * size );
		auto ait = a.begin();
		if( ait == a.end() )
		{
			return 0;
		}
		double result = *ait;
		++ait;
		while( ait != a.end() )
		{
			ait += size;
			result += *ait;
			++ait;
		}
		return result;
	}

	inline void inner_common(
		std::vector< double > const& mat,
		std::vector< double > const& v,
		std::vector< double >& r,
		bool clearold )
	{
		assert( v.size() * r.size() == mat.size() );
		auto mit = mat.begin();
		auto rit = r.begin();
		while( rit != r.end() )
		{
			if( clearold )
			{
				*rit = 0;
			}
			auto vit = v.begin();
			while( vit != v.end() )
			{
				*rit += *mit * *vit;
				++mit;
				++vit;
			}
			++rit;
		}
	}

	void inner(
		std::vector< double > const& mat,
		std::vector< double > const& v,
		std::vector< double >& r )
	{
		inner_common( mat, v, r, true );
	}

	void inneradd(
		std::vector< double > const& mat,
		std::vector< double > const& v,
		std::vector< double >& r )
	{
		inner_common( mat, v, r, false );
	}

	void matrixinner(
		std::vector< double > const& amat,
		std::vector< double > const& bmat,
		size_t rrows, size_t rcols,
		std::vector< double >& r )
	{
		assert( amat.size() % rrows == 0 );
		assert( bmat.size() % rcols == 0 );
		assert( ( amat.size() / rrows ) == ( bmat.size() / rcols ) );
		assert( r.size() == rrows * rcols );
		auto ait = amat.begin();
		auto rrow = r.begin();
		while( ait != amat.end() )
		{
			auto rrowend = rrow + rcols;
			{
				auto rit = rrow;
				while( rit != rrowend )
				{
					*rit = 0;
					++rit;
				}
			}
			auto bit = bmat.begin();
			while( bit != bmat.end() )
			{
				auto rit = rrow;
				while( rit != rrowend )
				{
					*rit += *ait * *bit;
					++rit;
					++bit;
				}
				++ait;
			}
			rrow = rrowend;
		}
	}

	std::vector< double > matrixconcat(
		std::vector< double > const& a,
		std::vector< double > const& b,
		size_t acols, size_t bcols )
	{
		if( acols == 0 )
		{
			return b;
		}
		else if( bcols == 0 )
		{
			return a;
		}
		assert( a.size() % acols == 0 && b.size() % bcols == 0 );
		assert( a.size() / acols == b.size() / bcols );
		std::vector< double > r( a.size() + b.size() );
		auto abegin = a.begin();
		auto bbegin = b.begin();
		auto rbegin = r.begin();
		auto rend = r.end();
		while( rbegin != rend )
		{
			for( size_t i = 0; i < acols; ++i )
			{
				*rbegin = *abegin;
				++rbegin;
				++abegin;
			}
			for( size_t i = 0; i < bcols; ++i )
			{
				*rbegin = *bbegin;
				++rbegin;
				++bbegin;
			}
		}
		return r;
	}

	std::vector< double > matrixpolynomial(
		std::vector< double > const& mat,
		size_t size )
	{
		assert( mat.size() == size * size );
		if( size == 0 )
		{
			return std::vector< double >{ 0 };
		}
		std::vector< double > factors( size + 1 );
		auto factorit = factors.rbegin();
		*factorit = 1;
		++factorit;
		int currentindex = 1;
		double currentfactor = 1;
		std::vector< double > currentmat( size * size );
		zero( currentmat );
		std::vector< double > tempmat( size * size );
		while( factorit != factors.rend() )
		{
			identityadd( currentfactor, size, currentmat );
			matrixinner( mat, currentmat, size, size, tempmat );
			currentfactor = - trace( tempmat, size ) / currentindex;
			std::swap( currentmat, tempmat );
			currentindex += 1;
			*factorit = currentfactor;
			++factorit;
		}
		return factors;
	}

	void vform::write( std::ostream& stream )
	{
		stream << "[";
		size_t col = 0;
		for( double a: v )
		{
			if( col == cols )
			{
				stream << "; ";
				col = 0;
			}
			else if( col != 0 )
			{
				stream << ", ";
			}
			stream << a;
			col += 1;
		}
		stream << "]";
	}

	void tform::write( std::ostream& stream )
	{
		for( double a: v )
		{
			stream << "\t" << a;
		}
	}
}
