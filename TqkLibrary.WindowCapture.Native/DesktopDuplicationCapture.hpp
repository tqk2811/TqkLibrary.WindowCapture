#ifndef _WindowCapture_H_DesktopDuplicationCapture_H_
#define _WindowCapture_H_DesktopDuplicationCapture_H_

//https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/desktop-dup-api

#include "BaseCapture.hpp"
#include "Directx.hpp"
#include "RenderToSurface.hpp"
#include <dxgi1_2.h>
#include <mutex>

class DesktopDuplicationCapture : public BaseCapture
{
public:
	DesktopDuplicationCapture();
	~DesktopDuplicationCapture();

	BOOL InitMonitorCapture(HMONITOR hmonitor);
	BOOL GetSize(UINT32& width, UINT32& height);
	BOOL Render(IDXGISurface* surface, bool isNewSurface, bool& isNewtargetView);
	BOOL CaptureImage(void* data, UINT32 width, UINT32 height, UINT32 linesize);
	BOOL IsSupported();

private:
	BOOL InitDuplication();
	BOOL AcquireFrame();  // Must be called under _mtx; returns TRUE if new frame acquired
	VOID CloseDuplication();

	HMONITOR m_hmonitor{ 0 };
	RenderToSurface _renderToSurface;

	ComPtr<IDXGIOutputDuplication> _duplication;
	ComPtr<ID3D11Texture2D> _tmpFrame;  // Staging texture (CPU_ACCESS_READ)

	UINT32 _width{ 0 };
	UINT32 _height{ 0 };

	LARGE_INTEGER _lastPresentTime{};
	LARGE_INTEGER _renderedTime{};

	std::recursive_mutex _mtx;
};

TqkLibrary_WindowCapture_Export DesktopDuplicationCapture* DesktopDuplication_Alloc();

#endif // !_WindowCapture_H_DesktopDuplicationCapture_H_
