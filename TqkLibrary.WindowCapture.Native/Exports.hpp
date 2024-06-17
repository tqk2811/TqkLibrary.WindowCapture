#ifndef _H_Exports_H_
#define _H_Exports_H_

#include "WinApi.hpp"

#ifdef TQKLIBRARYWINDOWCAPTURENATIVE_EXPORTS
#define TqkLibrary_WindowCapture_Export extern "C" __declspec( dllexport )
#else
#define TqkLibrary_WindowCapture_Export extern "C" __declspec( dllimport )
#endif

#endif // !_H_Exports_H_
