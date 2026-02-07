#ifndef _WindowCapture_H_HdcCapture_H_
#define _WindowCapture_H_HdcCapture_H_

#include "BaseCapture.hpp"
#include "Utils.hpp"

#include "WinApi.hpp"
#include "Directx.hpp"
#include "Gdi.hpp"
#include "RenderToSurface.hpp"

enum HdcCaptureMode: BYTE
{
	HdcCaptureMode_BitBlt,
	HdcCaptureMode_PrintWindow,
};

class HdcCapture : public BaseCapture
{
public:
	HdcCapture();
	~HdcCapture();

	HdcCaptureMode GetMode();
	VOID SetMode(HdcCaptureMode mode);

	BOOL InitWindowCapture(HWND hwnd);
	BOOL InitMonitorCapture(HMONITOR HMONITOR);
	BOOL Render(IDXGISurface* surface, bool isNewSurface, bool& isNewtargetView) ;
	BOOL CaptureImage(void* data, UINT32 width, UINT32 height, UINT32 linesize);
	BOOL GetSize(UINT32& width, UINT32& height);
	BOOL IsSupported() { return TRUE; }

private:
	HWND m_hwnd{ 0 };
	HMONITOR m_hmonitor{ 0 };

	HDC _hdc{ 0 };
	HDC _hdcDest{ 0 };  // Cached compatible DC for reuse
	HdcCaptureMode _mode{ HdcCaptureMode_BitBlt };
	RenderToSurface _renderToSurface;
	ComPtr<ID3D11Texture2D> _renderTexture{ NULL };

	// Buffer pooling for bitmap data
	BYTE* _pBitmapBuffer{ nullptr };
	DWORD _bufferSize{ 0 };
	
	// Track size changes
	UINT32 _lastWidth{ 0 };
	UINT32 _lastHeight{ 0 };

	HBITMAP CaptureToHBitmap(HdcCaptureMode mode);

	BOOL CopyBitmapToTexture(
		const HBITMAP hBitmap,
		const HDC hdc,
		ID3D11Device* device,
		ID3D11DeviceContext* deviceCtx,
		ComPtr<ID3D11Texture2D>& textureBGRA
	);
};

TqkLibrary_WindowCapture_Export HdcCapture* HdcCapture_Alloc();
TqkLibrary_WindowCapture_Export HdcCaptureMode HdcCapture_GetMode(HdcCapture* hdcCapture);
TqkLibrary_WindowCapture_Export VOID HdcCapture_SetMode(HdcCapture* hdcCapture, HdcCaptureMode mode);

#endif