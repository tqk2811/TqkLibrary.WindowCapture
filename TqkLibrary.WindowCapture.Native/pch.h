#ifndef _H_TqkLibrary_WindowCapture_Native_H_
#define _H_TqkLibrary_WindowCapture_Native_H_

#ifdef TQKLIBRARYWINDOWCAPTURENATIVE_EXPORTS
#define TqkLibrary_WindowCapture_Export extern "C" __declspec( dllexport )
#else
#define TqkLibrary_WindowCapture_Export extern "C" __declspec( dllimport )
#endif

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <d3d11.h>

#include <d3d9.h>
#include <D3D9Types.h>

#include <directxmath.h>
#include <wrl/client.h>
#include <dxgi.h>

using namespace Microsoft::WRL;
using namespace DirectX;


#endif

TqkLibrary_WindowCapture_Export void Test(HWND windowHandler);