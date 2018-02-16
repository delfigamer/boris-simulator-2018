#pragma once

#include <map>
#include <string>
#include <vector>

namespace parser
{
	struct matrix_t
	{
		std::vector< double > elems;
		size_t rows;
		size_t cols;
	};

	struct generator_t
	{
		std::string name;
		std::vector< double > args;
	};

	enum class vtag
	{
		nil,
		number,
		matrix,
		generator,
	};

	class value_t
	{
	private:
		vtag m_tag;
		union
		{
			double m_number;
			matrix_t m_matrix;
			generator_t m_generator;
		};

	public:
		value_t( vtag inittag = vtag::nil );
		value_t( value_t const& other );
		value_t( value_t&& other );
		value_t& operator=( value_t const& other );
		value_t& operator=( value_t&& other );
		~value_t();

		void settag( vtag newtag );
		vtag gettag();

		double& number();
		matrix_t& matrix();
		generator_t& generator();

		operator bool();
	};

	class table_t
	{
	private:
		std::map< std::string, value_t > m_names;

		template< typename onget_t, typename... rest_t >
		void get_dispatch(
			value_t& value, vtag tag, onget_t onget, rest_t... rest );
		template< typename onget_t >
		void get_dispatch(
			value_t& value, onget_t onget );
		void get_dispatch(
			value_t& value );

	public:
		void loadfile( char const* path );

		template< typename... Ts >
		void get( char const* name, Ts... args );
	};

	template< typename onget_t, typename... rest_t >
	void table_t::get_dispatch(
		value_t& value, vtag tag, onget_t onget, rest_t... rest )
	{
		if( value.gettag() == tag )
		{
			onget( value );
		}
		else
		{
			get_dispatch( value, rest... );
		}
	}

	template< typename onget_t >
	void table_t::get_dispatch(
		value_t& value, onget_t onget )
	{
		onget( value );
	}

	inline void table_t::get_dispatch(
		value_t& value )
	{
	}

	template< typename... Ts >
	void table_t::get( char const* name, Ts... args )
	{
		auto it = m_names.find( name );
		if( it != m_names.end() )
		{
			get_dispatch( it->second, args... );
		}
		else
		{
			value_t nilvalue;
			get_dispatch( nilvalue, args... );
		}
	}
}
