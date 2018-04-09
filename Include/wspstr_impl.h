#pragma once


#include	"wspstr.h"


std::wstring wsp::to( const char* arg )
{
	std::wstringstream interpreter;
	std::wstring result;

	if ( !( interpreter << arg ) )
		throw std::runtime_error{ "to<>() failed" };

	interpreter >> std::noskipws;
	for( std::wstring::value_type c ; interpreter >> c ; )
		result += c;

	if ( !( interpreter >> std::ws ).eof() )
		throw std::runtime_error{ "to<>() failed" };

	return result;
}

std::wstring wsp::to( const wchar_t* arg )
{
	std::wstringstream interpreter;
	std::wstring result;

	if ( !( interpreter << arg ) )
		throw std::runtime_error{ "to<>() failed" };

	interpreter >> std::noskipws;
	for( std::wstring::value_type c ; interpreter >> c ; )
		result += c;

	if ( !( interpreter >> std::ws ).eof() )
		throw std::runtime_error{ "to<>() failed" };

	return result;
}
