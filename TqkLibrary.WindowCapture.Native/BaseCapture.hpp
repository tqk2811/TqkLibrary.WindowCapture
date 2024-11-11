#ifndef _WindowCapture_H_BaseCapture_H_
#define _WindowCapture_H_BaseCapture_H_

#include "Directx.hpp"
#include "D3DClass.hpp"
#include "Exports.hpp"
#include "HashHelper.hpp"


class BaseCapture
{
public:
	BaseCapture();
	virtual ~BaseCapture();
	virtual BOOL InitCapture(HWND hWnd) = 0;
	virtual BOOL GetSize(UINT32& width, UINT32& height) = 0;
	virtual BOOL Render(IDXGISurface* surface, bool isNewSurface, bool& isNewtargetView) = 0;
	virtual BOOL CaptureImage(void* data, UINT32 width, UINT32 height, UINT32 linesize) = 0;
	virtual BOOL IsSupported() = 0;
protected:
	HWND m_hWnd{ 0 };

#ifdef HashHelper_Enable
	HashHelper* m_HashHelper;
	BYTE m_hash[HashHelper_HashSize];
#endif

};


TqkLibrary_WindowCapture_Export VOID BaseCapture_Free(BaseCapture** ppBaseCapture);

TqkLibrary_WindowCapture_Export BOOL BaseCapture_InitCapture(BaseCapture* pBaseCapture, HWND hWnd);
TqkLibrary_WindowCapture_Export BOOL BaseCapture_GetSize(BaseCapture* pBaseCapture, UINT32& width, UINT32& height);
TqkLibrary_WindowCapture_Export BOOL BaseCapture_IsSupported(BaseCapture* pBaseCapture);


TqkLibrary_WindowCapture_Export BOOL BaseCapture_Render(BaseCapture* pBaseCapture, IDXGISurface* surface, bool isNewSurface, bool& isNewtargetView);
TqkLibrary_WindowCapture_Export BOOL BaseCapture_CaptureImage(BaseCapture* pBaseCapture, void* data, UINT32 width, UINT32 height, UINT32 lineSize);


TqkLibrary_WindowCapture_Export BOOL HBITMAP_Release(HBITMAP hbitmap);

#endif


