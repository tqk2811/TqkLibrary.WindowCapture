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
#include <winrt/Windows.Foundation.Metadata.h>
#include <Windows.Graphics.Capture.Interop.h>
#ifdef NDEBUG
//https://github.com/microsoft/STL/releases/tag/vs-2022-17.10
//#3824 #4000 #4339
#define _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR
#endif
#include <mutex>

class WinrtGraphicCapture : public BaseCapture
{
public:
	WinrtGraphicCapture();
	~WinrtGraphicCapture();

	INT32 GetDelay();
	VOID SetDelay(INT32 delay);

	// BaseCapture
	BOOL InitCapture(HWND hwnd);
	BOOL GetSize(UINT32& width, UINT32& height);
	BOOL CaptureImage(void* data, UINT32 width, UINT32 height, UINT32 linesize);
	BOOL Render(IDXGISurface* surface, bool isNewSurface, bool& isNewtargetView);
	BOOL IsSupported();



private:
	VOID OnFrameArrived(
		winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender,
		winrt::Windows::Foundation::IInspectable const&);
	BOOL Init();
	VOID Close();

	RenderToSurface _renderToSurface;
	winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice m_direct3d_device{ nullptr };

	
	winrt::Windows::Graphics::SizeInt32 m_lastSize{ NULL };
	winrt::Windows::Foundation::TimeSpan m_lastTime{ NULL };
	winrt::Windows::Foundation::TimeSpan m_RenderedTime{ NULL };

	winrt::Windows::Graphics::Capture::GraphicsCaptureItem m_item{ NULL };
	winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool m_framePool{ NULL };
	winrt::Windows::Graphics::Capture::GraphicsCaptureSession m_session{ NULL };
	winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::FrameArrived_revoker m_frameArrived;

	BOOL _isCapturing = FALSE;
	std::recursive_mutex _mtx_lockInstance;
	std::recursive_mutex _mtx_lockFrame;

	UINT32 m_delay{ 0 };
	ComPtr<ID3D11Texture2D> _tmpFrame{ nullptr };
};

TqkLibrary_WindowCapture_Export WinrtGraphicCapture* WinrtGraphicCapture_Alloc();
TqkLibrary_WindowCapture_Export INT32 WinrtGraphicCapture_GetDelay(WinrtGraphicCapture* p);
TqkLibrary_WindowCapture_Export VOID WinrtGraphicCapture_SetDelay(WinrtGraphicCapture* p, INT32 delay);


#endif // !_WindowCapture_H_WinrtGraphicCapture_H_

