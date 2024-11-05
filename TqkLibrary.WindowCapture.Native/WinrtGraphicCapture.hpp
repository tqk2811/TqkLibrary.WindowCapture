#ifndef _WindowCapture_H_WinrtGraphicCapture_H_
#define _WindowCapture_H_WinrtGraphicCapture_H_

#include "BaseCapture.hpp"
#include "WinApi.hpp"
#include "Directx.hpp"
#include "RenderToSurface.hpp"
#include <dxgi.h>
#include <dxgi1_2.h>

#include <Unknwn.h>
#include <inspectable.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Media.Capture.h>
#include <Windows.Graphics.Capture.Interop.h>

class WinrtGraphicCapture: public BaseCapture
{
public:
	WinrtGraphicCapture();
	~WinrtGraphicCapture();

	// BaseCapture
	BOOL InitCapture(HWND hwnd);
	BOOL GetSize(UINT32& width, UINT32& height);
	BOOL CaptureImage(void* data, UINT32 width, UINT32 height, UINT32 linesize);
	BOOL Render(IDXGISurface* surface, bool isNewSurface, bool& isNewtargetView);

private:
	BOOL Init();
	VOID Close();

	RenderToSurface _renderToSurface;
	winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice m_direct3d_device{ nullptr };

	ComPtr<ID3D11Texture2D> _cpuCaptureTexture{ nullptr };

	winrt::Windows::Graphics::SizeInt32 m_lastSize{ NULL };
	winrt::Windows::Graphics::Capture::GraphicsCaptureItem m_item{ NULL };
	winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool m_framePool{ NULL };
	winrt::Windows::Graphics::Capture::GraphicsCaptureSession m_session{ NULL };

};

TqkLibrary_WindowCapture_Export WinrtGraphicCapture* WinrtGraphicCapture_Alloc();


#endif // !_WindowCapture_H_WinrtGraphicCapture_H_

