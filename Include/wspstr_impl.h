#pragma once


#include	"wspstr.h"


std::wstring wsp::to( const char* arg )
{
	std::wstringstream interpreter;
	std::wstring result;

	if ( !( interpreter << arg ) )
		throw std::runtime_error{ "std::wstringstream::operator<<() in to<std::wstring, const char*>() failed" };

	interpreter >> std::noskipws;
	for( std::wstring::value_type c ; interpreter >> c ; )
		result += c;

	if ( !( interpreter >> std::ws ).eof() )
		throw std::runtime_error{ "It isn't eof() in to<std::wstring, const char*>() failed" };

	return result;
}

std::wstring wsp::to( const wchar_t* arg )
{
	std::wstringstream interpreter;
	std::wstring result;

	if ( !( interpreter << arg ) )
		throw std::runtime_error{ "std::wstringstream::operator<<() in to<std::wstring, const wchar_t*>() failed" };

	interpreter >> std::noskipws;
	for( std::wstring::value_type c ; interpreter >> c ; )
		result += c;

	if ( !( interpreter >> std::ws ).eof() )
		throw std::runtime_error{ "It isn't eof() in to<std::wstring, const wchar_t*>() failed" };

	return result;
}
