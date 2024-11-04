#ifndef _WindowCapture_H_WinrtGraphicCapture_H_
#define _WindowCapture_H_WinrtGraphicCapture_H_

#include "WinApi.hpp"
#include "Directx.hpp"
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

class WinrtGraphicCapture
{
public:
	WinrtGraphicCapture();
	~WinrtGraphicCapture();

	BOOL Init();
	BOOL SetCapture(HWND hwnd);
	VOID Close();

	BOOL GetSize(UINT32& width, UINT32& height);
	BOOL Draw(ID3D11Device* device, ID3D11DeviceContext* deviceCtx, ComPtr<ID3D11Texture2D>& texture);

private:
	VOID OnFrameArrived(
		winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender,
		winrt::Windows::Foundation::IInspectable const&
	);

private:
	winrt::com_ptr<ID3D11Device> m_d3d11_device{ nullptr };
	winrt::com_ptr<IDXGIDevice> m_dxgi_device{ nullptr };
	winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice m_direct3d_device{ nullptr };
	winrt::com_ptr<ID3D11DeviceContext> m_d3dContext{ nullptr };

	winrt::Windows::Graphics::SizeInt32 m_lastSize{ NULL };

	winrt::Windows::Graphics::Capture::GraphicsCaptureItem m_item{ NULL };
	winrt::com_ptr<IDXGISwapChain1> m_swapChain;
	winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool m_framePool{ NULL };
	winrt::Windows::Graphics::Capture::GraphicsCaptureSession m_session{ NULL };
	winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::FrameArrived_revoker m_frameArrived;

	std::atomic<bool> m_closed = false;
};
#endif // !_WindowCapture_H_WinrtGraphicCapture_H_

