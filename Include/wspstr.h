#pragma once


#include	<string>
#include	<sstream>
#include	<stdexcept>


namespace wsp
{
#if defined( UNICODE ) || defined( _UNICODE )
	using tstring = std::wstring;
	using tstringstream = std::wstringstream;
#else	// defined( UNICODE ) || defined( _UNICODE )
	using tstring = std::string;
	using tstringstream = std::stringstream;
#endif	// defined( UNICODE ) || defined( _UNICODE )

	using tchar = tstring::value_type;

	template< typename Target = tstring, typename Source = tstring >
	Target to( Source arg )
	{
		tstringstream interpreter;
		Target result;
		if ( !( interpreter << arg ) || !( interpreter >> result ) || !( interpreter >> std::ws ).eof() )
			throw std::runtime_error{ "to<>() failed" };
		return result;
	}

	std::wstring to( const char* arg );
	std::wstring to( const wchar_t* arg );
};

using wsp::tstring;
using wsp::tstringstream;
using wsp::tchar;
