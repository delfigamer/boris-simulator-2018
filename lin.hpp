#pragma once

#include <vector>
#include <iostream>

namespace lin
{
	void add(
		std::vector< double > const& a,
		std::vector< double > const& b,
		std::vector< double >& r );
	void add(
		std::vector< double > const& a,
		std::vector< double >& r );
	void zero(
		std::vector< double >& r );
	void inner(
		std::vector< double > const& mat,
		std::vector< double > const& v,
		std::vector< double >& r );
	void inneradd(
		std::vector< double > const& mat,
		std::vector< double > const& v,
		std::vector< double >& r );
	std::vector< double > matrixconcat(
		std::vector< double > const& a,
		std::vector< double > const& b,
		size_t acols, size_t bcols );

	struct vform
	{
		std::vector< double > const& v;
		size_t cols;

		vform( std::vector< double > const& v, size_t cols = size_t( -1 ) )
			: v( v )
			, cols( cols )
		{
		}

		void write( std::ostream& stream );
	};

	template< typename stream_t >
	stream_t& operator<<( stream_t& stream, vform&& f )
	{
		f.write( stream );
		return stream;
	}

	struct tform
	{
		std::vector< double > const& v;

		tform( std::vector< double > const& v )
			: v( v )
		{
		}

		void write( std::ostream& stream );
	};

	template< typename stream_t >
	stream_t& operator<<( stream_t& stream, tform&& f )
	{
		f.write( stream );
		return stream;
	}
}
