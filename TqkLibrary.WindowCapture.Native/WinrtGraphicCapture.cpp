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
	m_delay = max(delay,0);
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
	m_session.StartCapture();

	_isCapturing = TRUE;
	_mtx_lockInstance.unlock();

	return TRUE;
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

		_renderToSurface.SendTexture(frameSurface.get());
		//_mtx_lockFrame.lock();

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
			_mtx_lockFrame.lock();
			hr = d3dDevice->CreateTexture2D(&desc, NULL, _tmpFrame.ReleaseAndGetAddressOf());
			_mtx_lockFrame.unlock();
		}
		if (SUCCEEDED(hr))
		{
			d3dDeviceCtx->CopyResource(_tmpFrame.Get(), frameSurface.get());
			m_lastTime = frame.SystemRelativeTime();
		}

		//_mtx_lockFrame.unlock();

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
		if (_tmpFrame.Get() &&
			m_lastTime != m_RenderedTime
			)
		{
			//_mtx_lockFrame.lock();

			//result = _renderToSurface.SendTexture(_tmpFrame.Get());
			m_RenderedTime = m_lastTime;

			//_mtx_lockFrame.unlock();

			result = _renderToSurface.Render();
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
	BOOL result = FALSE;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc,sizeof(D3D11_TEXTURE2D_DESC));	
	if (_tmpFrame.Get())
		_tmpFrame->GetDesc(&desc);
	if (_tmpFrame.Get() &&
		data &&
		width > 0 && height > 0 &&
		desc.Width == width && desc.Height == height)
	{
		ComPtr<ID3D11Device> d3dDevice = this->_renderToSurface.GetDevive();
		ComPtr<ID3D11DeviceContext> d3dDeviceCtx = this->_renderToSurface.GetDeviceContext();
		D3D11_MAPPED_SUBRESOURCE map;
		HRESULT hr = d3dDeviceCtx->Map(_tmpFrame.Get(), 0, D3D11_MAP::D3D11_MAP_READ, 0, &map);
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

			d3dDeviceCtx->Unmap(_tmpFrame.Get(), 0);
			result = TRUE;
		}
	}
	_mtx_lockFrame.unlock();
	return result;
}

VOID WinrtGraphicCapture::Close()
{
	_mtx_lockInstance.lock();

	if (_isCapturing)
	{
		//if (m_frameArrived) 
		m_frameArrived.revoke();
		if (m_framePool) m_framePool.Close();
		if (m_session) m_session.Close();

		m_framePool = nullptr;
		m_session = nullptr;
		m_item = nullptr;
		//m_frameArrived = nullptr;
	}
	_isCapturing = FALSE;

	_mtx_lockInstance.unlock();
}

BOOL WinrtGraphicCapture::GetSize(UINT32& width, UINT32& height)
{
	width = m_lastSize.Width;
	height = m_lastSize.Height;
	return TRUE;
}