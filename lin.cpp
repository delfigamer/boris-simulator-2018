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
		auto abegin = a.begin();
		auto bbegin = b.begin();
		auto rbegin = r.begin();
		auto rend = r.end();
		while( rbegin != rend )
		{
			*rbegin = *abegin + *bbegin;
			++abegin;
			++bbegin;
			++rbegin;
		}
	}

	void add(
		std::vector< double > const& a,
		std::vector< double >& r )
	{
		assert( a.size() == r.size() );
		auto abegin = a.begin();
		auto rbegin = r.begin();
		auto rend = r.end();
		while( rbegin != rend )
		{
			*rbegin += *abegin;
			++abegin;
			++rbegin;
		}
	}

	inline void inner_common(
		std::vector< double > const& mat,
		std::vector< double > const& v,
		std::vector< double >& r,
		bool clearold )
	{
		assert( v.size() * r.size() == mat.size() );
		auto matbegin = mat.begin();
		auto vbegin = v.begin();
		auto vend = v.end();
		auto rbegin = r.begin();
		auto rend = r.end();
		while( rbegin != rend )
		{
			if( clearold )
			{
				*rbegin = 0;
			}
			auto vit = vbegin;
			while( vit != vend )
			{
				*rbegin += *matbegin * *vit;
				++matbegin;
				++vit;
			}
			++rbegin;
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
