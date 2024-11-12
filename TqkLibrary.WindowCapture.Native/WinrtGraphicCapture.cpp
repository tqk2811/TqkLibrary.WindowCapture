#include "WinrtGraphicCapture.hpp"

#define FramePool_NUMBER_BUFFER 1
extern "C"
{
	HRESULT __stdcall CreateDirect3D11DeviceFromDXGIDevice(::IDXGIDevice* dxgiDevice,
		::IInspectable** graphicsDevice);
}

struct __declspec(uuid("A9B3D012-3DF2-4EE3-B8D1-8695F457D3C1"))
	IDirect3DDxgiInterfaceAccess : ::IUnknown
{
	virtual HRESULT __stdcall GetInterface(GUID const& id, void** object) = 0;
};

template <typename T>
auto GetDXGIInterfaceFromObject(winrt::Windows::Foundation::IInspectable const& object)
{
	auto access = object.as<IDirect3DDxgiInterfaceAccess>();
	winrt::com_ptr<T> result;
	winrt::check_hresult(access->GetInterface(winrt::guid_of<T>(), result.put_void()));
	return result;
}

WinrtGraphicCapture::WinrtGraphicCapture()
{

}
WinrtGraphicCapture::~WinrtGraphicCapture()
{
	Close();
	_tmpFrame.Reset();
}
INT32 WinrtGraphicCapture::GetDelay()
{
	return m_delay;
}
VOID WinrtGraphicCapture::SetDelay(INT32 delay)
{
	m_delay = max(delay, 0);
}
BOOL WinrtGraphicCapture::Init()
{
	BOOL result = _renderToSurface.Init();

	HRESULT hr{ 0 };
	if (result && !m_direct3d_device)
	{
		ComPtr<IDXGIDevice> dxgi_device = _renderToSurface.GetDXGIDevice();
		hr = CreateDirect3D11DeviceFromDXGIDevice(dxgi_device.Get(), (IInspectable**)&m_direct3d_device);
		winrt::check_hresult(hr);
		if (FAILED(hr))
			return FALSE;
	}

	return result;
}

BOOL WinrtGraphicCapture::InitCapture(HWND hwnd)
{
	BOOL result = Init();
	if (!result)
		return result;

	//clean old
	Close();

	if (hwnd == INVALID_HANDLE_VALUE || hwnd == 0)
		return FALSE;

	_mtx_lockInstance.lock();

	//create new
	auto activation_factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
	auto interop_factory = activation_factory.as<IGraphicsCaptureItemInterop>();

	interop_factory->CreateForWindow(
		hwnd,
		winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(),
		reinterpret_cast<void**>(winrt::put_abi(m_item))
	);

	m_lastSize = m_item.Size();

	m_framePool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::CreateFreeThreaded(
		m_direct3d_device,
		winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
		FramePool_NUMBER_BUFFER,
		m_lastSize);//not work with D3D11_CREATE_DEVICE_SINGLETHREADED
	m_session = m_framePool.CreateCaptureSession(m_item);
	m_frameArrived = m_framePool.FrameArrived(winrt::auto_revoke, { this, &WinrtGraphicCapture::OnFrameArrived });

	if (WinrtGraphicCapture_IsCaptureCursorToggleSupported() && m_isSetCursorState != TRUE)
		m_session.IsCursorCaptureEnabled(m_isSetCursorState);
	if (WinrtGraphicCapture_IsBorderToggleSupported() && m_isSetBorderState != TRUE)
		m_session.IsBorderRequired(m_isSetBorderState);

	m_session.StartCapture();

	_isCapturing = TRUE;
	_mtx_lockInstance.unlock();

	return TRUE;
}
BOOL WinrtGraphicCapture::IsSupported()
{
	return winrt::Windows::Foundation::Metadata::ApiInformation::IsApiContractPresent(
		L"Windows.Foundation.UniversalApiContract",
		8
	);
}
VOID WinrtGraphicCapture::OnFrameArrived(
	winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender,
	winrt::Windows::Foundation::IInspectable const&)
{
	_mtx_lockInstance.lock();
	if (_isCapturing)
	{
		auto frame = m_framePool.TryGetNextFrame();
		auto newSize = false;
		auto frameContentSize = frame.ContentSize();
		if (frameContentSize.Width != m_lastSize.Width ||
			frameContentSize.Height != m_lastSize.Height)
		{
			// The thing we have been capturing has changed size.
			// We need to resize our swap chain first, then blit the pixels.
			// After we do that, retire the frame and then recreate our frame pool.
			newSize = true;
			m_lastSize = frameContentSize;
		}

		auto frameSurface = GetDXGIInterfaceFromObject<ID3D11Texture2D>(frame.Surface());

		//copy frame to cache
		ComPtr<ID3D11Device> d3dDevice = this->_renderToSurface.GetDevive();
		ComPtr<ID3D11DeviceContext> d3dDeviceCtx = this->_renderToSurface.GetDeviceContext();

		HRESULT hr{ S_OK };
		bool isRecreate = false;
		D3D11_TEXTURE2D_DESC desc;
		isRecreate = !_tmpFrame.Get();
		if (!isRecreate)
		{
			_tmpFrame->GetDesc(&desc);
			isRecreate = desc.Width != m_lastSize.Width || desc.Height != m_lastSize.Height;
		}

		//_renderToSurface.SendTexture(frameSurface.get());

		if (isRecreate)
		{
			frameSurface->GetDesc(&desc);
			//desc.Width = m_lastSize.Width;
			//desc.Height = m_lastSize.Height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.BindFlags = 0;//D3D11_BIND_FLAG
			desc.Usage = D3D11_USAGE::D3D11_USAGE_STAGING;
			desc.MiscFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			ComPtr<ID3D11Texture2D> tmpFrame;
			hr = d3dDevice->CreateTexture2D(&desc, NULL, tmpFrame.GetAddressOf());

			_mtx_lockFrame.lock();

			_tmpFrame.Reset();
			_tmpFrame = tmpFrame.Get();

			_mtx_lockFrame.unlock();
		}
		if (SUCCEEDED(hr))
		{
			d3dDeviceCtx->CopyResource(_tmpFrame.Get(), frameSurface.get());
			m_lastTime = frame.SystemRelativeTime();
		}

		if (newSize)
		{
			m_framePool.Recreate(
				m_direct3d_device,
				winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
				FramePool_NUMBER_BUFFER,
				m_lastSize);
		}

		Sleep(m_delay);

		frame.Close();
	}
	_mtx_lockInstance.unlock();
}

BOOL WinrtGraphicCapture::Render(IDXGISurface* surface, bool isNewSurface, bool& isNewtargetView)
{
	BOOL result = FALSE;
	if (_isCapturing)
	{
		result = _renderToSurface.InitializeSurface(surface, isNewSurface, isNewtargetView);

		_mtx_lockFrame.lock();

		ComPtr<ID3D11Texture2D> tmpFrame = _tmpFrame.Get();
		winrt::Windows::Foundation::TimeSpan lastTime = m_lastTime;

		_mtx_lockFrame.unlock();


		if (tmpFrame.Get() &&
			lastTime != m_RenderedTime
			)
		{
			result = _renderToSurface.SendTexture(tmpFrame.Get());
			result = _renderToSurface.Render();
			m_RenderedTime = m_lastTime;
		}
		else if (isNewSurface || isNewtargetView)
		{
			result = _renderToSurface.Render();//render old frame
		}
		else
		{
			result = FALSE;
		}
	}
	return result;
}

BOOL WinrtGraphicCapture::CaptureImage(void* data, UINT32 width, UINT32 height, UINT32 linesize)
{
	_mtx_lockFrame.lock();

	ComPtr<ID3D11Texture2D> tmpFrame = _tmpFrame.Get();

	_mtx_lockFrame.unlock();

	BOOL result = FALSE;
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	if (tmpFrame.Get())
		tmpFrame->GetDesc(&desc);
	if (tmpFrame.Get() &&
		data &&
		width > 0 && height > 0 &&
		desc.Width == width && desc.Height == height)
	{
		ComPtr<ID3D11Device> d3dDevice = this->_renderToSurface.GetDevive();
		ComPtr<ID3D11DeviceContext> d3dDeviceCtx = this->_renderToSurface.GetDeviceContext();
		D3D11_MAPPED_SUBRESOURCE map;
		HRESULT hr = d3dDeviceCtx->Map(tmpFrame.Get(), 0, D3D11_MAP::D3D11_MAP_READ, 0, &map);
		if (SUCCEEDED(hr))
		{
			if (map.RowPitch == linesize)
			{
				memcpy(data, map.pData, map.DepthPitch);
			}
			else
			{
				int row = min(map.RowPitch, linesize);
				for (int i = 0; i < height; i++)
				{
					memcpy(
						(void*)((UINT64)data + linesize * i),
						(void*)((UINT64)map.pData + map.RowPitch * i),
						row
					);
				}
			}

			d3dDeviceCtx->Unmap(tmpFrame.Get(), 0);
			result = TRUE;
		}
	}
	return result;
}

VOID WinrtGraphicCapture::Close()
{
	_mtx_lockInstance.lock();

	if (_isCapturing)
	{
		m_frameArrived.revoke();
		if (m_session)
			m_session.Close();
		m_session = nullptr;
		m_item = nullptr;

		if (m_framePool)
			m_framePool.Close();
		m_framePool = nullptr;
	}
	_isCapturing = FALSE;

	_mtx_lockInstance.unlock();

	_mtx_lockFrame.lock();

	_tmpFrame.Reset();

	_mtx_lockFrame.unlock();
}

BOOL WinrtGraphicCapture::GetSize(UINT32& width, UINT32& height)
{
	width = m_lastSize.Width;
	height = m_lastSize.Height;
	return TRUE;
}

BOOL WinrtGraphicCapture::SetCursorState(BOOL isVisible)
{
	BOOL result = FALSE;
	_mtx_lockInstance.lock();

	if (_isCapturing)
	{
		m_session.IsCursorCaptureEnabled(isVisible);
		result = TRUE;
	}
	else
	{
		m_isSetCursorState = isVisible;
		result = TRUE;
	}

	_mtx_lockInstance.unlock();
	return result;
}
BOOL WinrtGraphicCapture::GetCursorState(BOOL& state)
{
	BOOL result = FALSE;
	_mtx_lockInstance.lock();

	if (_isCapturing)
	{
		state = m_session.IsCursorCaptureEnabled();
		result = TRUE;
	}
	else
	{
		state = m_isSetCursorState;
		result = TRUE;
	}

	_mtx_lockInstance.unlock();
	return result;
}

BOOL WinrtGraphicCapture::SetBorderState(BOOL isVisible)
{
	BOOL result = FALSE;
	_mtx_lockInstance.lock();

	if (_isCapturing)
	{
		m_session.IsCursorCaptureEnabled(isVisible);
		result = TRUE;
	}
	else
	{
		m_isSetBorderState = isVisible;
		result = TRUE;
	}

	_mtx_lockInstance.unlock();
	return result;
}
BOOL WinrtGraphicCapture::GetBorderState(BOOL& state)
{
	BOOL result = FALSE;
	_mtx_lockInstance.lock();

	if (_isCapturing)
	{
		state = m_session.IsCursorCaptureEnabled();
		result = TRUE;
	}
	else
	{
		state = m_isSetBorderState;
		result = TRUE;
	}

	_mtx_lockInstance.unlock();
	return result;
}

//BOOL WinrtGraphicCapture::GetMinUpdateInterval(BOOL& isVisible)
//{
//	BOOL result = FALSE;
//	_mtx_lockInstance.lock();
//
//	if (_isCapturing)
//	{
//		m_session.IsCursorCaptureEnabled(isVisible);
//		result = TRUE;
//	}
//	else
//	{
//		m_isSetBorderState = isVisible;
//		result = TRUE;
//	}
//
//	_mtx_lockInstance.unlock();
//	return result;
//}

//BOOL WinrtGraphicCapture::SetMinUpdateInterval(BOOL state)
//{
//	BOOL result = FALSE;
//	_mtx_lockInstance.lock();
//
//	if (_isCapturing)
//	{
//		state = m_session.;
//		result = TRUE;
//	}
//	else
//	{
//		state = m_isSetBorderState;
//		result = TRUE;
//	}
//
//	_mtx_lockInstance.unlock();
//	return result;
//}








WinrtGraphicCapture* WinrtGraphicCapture_Alloc()
{
	return new WinrtGraphicCapture();
}
INT32 WinrtGraphicCapture_GetDelay(WinrtGraphicCapture* p)
{
	if (p)
		return p->GetDelay();
	return -1;
}
VOID WinrtGraphicCapture_SetDelay(WinrtGraphicCapture* p, INT32 delay)
{
	if (p)
		return p->SetDelay(delay);
}


BOOL WinrtGraphicCapture_IsCaptureCursorToggleSupported()
{
	return winrt::Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent(
		L"Windows.Graphics.Capture.GraphicsCaptureSession",
		L"IsCursorCaptureEnabled"
	);
}
BOOL WinrtGraphicCapture_SetCursorState(WinrtGraphicCapture* p, BOOL isVisible)
{
	if (p)
		return p->SetCursorState(isVisible);
	return FALSE;
}
BOOL WinrtGraphicCapture_GetCursorState(WinrtGraphicCapture* p, BOOL& state)
{
	if (p)
		return p->GetCursorState(state);
	return FALSE;
}


BOOL WinrtGraphicCapture_IsBorderToggleSupported()
{
	return winrt::Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent(
		L"Windows.Graphics.Capture.GraphicsCaptureSession",
		L"IsBorderRequired"
	);
}
BOOL WinrtGraphicCapture_SetBorderState(WinrtGraphicCapture* p, BOOL isVisible)
{
	if (p)
		return p->SetBorderState(isVisible);
	return FALSE;
}
BOOL WinrtGraphicCapture_GetBorderState(WinrtGraphicCapture* p, BOOL& state)
{
	if (p)
		return p->GetBorderState(state);
	return FALSE;
}