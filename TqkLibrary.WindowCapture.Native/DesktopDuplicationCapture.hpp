#ifndef _WindowCapture_H_DesktopDuplicationCapture_H_
#define _WindowCapture_H_DesktopDuplicationCapture_H_

//https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/desktop-dup-api

#include "BaseCapture.hpp"


class DesktopDuplicationCapture : public BaseCapture
{
public:
	DesktopDuplicationCapture();
	~DesktopDuplicationCapture();

	BOOL Draw(ID3D11Device* device, ID3D11DeviceContext* deviceCtx, ComPtr<ID3D11Texture2D>& textureBGRA);
	HBITMAP Shoot();

	BOOL InitMonitorCapture(HMONITOR hmonitor);
	BOOL GetSize(UINT32& width, UINT32& height);
	BOOL Render(IDXGISurface* surface, bool isNewSurface, bool& isNewtargetView);
	BOOL CaptureImage(void* data, UINT32 width, UINT32 height, UINT32 linesize);
	BOOL IsSupported();
private:
	HMONITOR m_hmonitor{ 0 };
};

TqkLibrary_WindowCapture_Export DesktopDuplicationCapture* DesktopDuplication_Alloc();

#endif // !_WindowCapture_H_DesktopDuplicationCapture_H_
