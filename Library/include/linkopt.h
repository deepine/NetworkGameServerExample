#pragma once


#define NGS_LIBRARY_EXPORTS


#ifdef NGS_LIBRARY_EXPORTS
#define NETLIB_API __declspec( dllexport )
#define DLLEXTERN
#else	// NGS_LIBRARY_EXPORTS
#define NETLIB_API __declspec( dllimport )
#define DLLEXTERN extern
#endif	// NGS_LIBRARY_EXPORTS
