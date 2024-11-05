#include "WinrtGraphicCapture.hpp"

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

WinrtGraphicCapture::WinrtGraphicCapture()
{

}
WinrtGraphicCapture::~WinrtGraphicCapture()
{
	Close();
	_cpuCaptureTexture.Reset();
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

	//create new
	auto activation_factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
	auto interop_factory = activation_factory.as<IGraphicsCaptureItemInterop>();

	interop_factory->CreateForWindow(
		hwnd,
		winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(),
		reinterpret_cast<void**>(winrt::put_abi(m_item))
	);

	m_lastSize = m_item.Size();

	m_framePool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::Create(
		m_direct3d_device,
		winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
		2,
		m_lastSize);//not work with D3D11_CREATE_DEVICE_SINGLETHREADED
	m_session = m_framePool.CreateCaptureSession(m_item);
	m_session.StartCapture();

	return TRUE;
}

BOOL WinrtGraphicCapture::Render(IDXGISurface* surface, bool isNewSurface, bool& isNewtargetView)
{
	BOOL result = _renderToSurface.InitializeSurface(surface, isNewSurface, isNewtargetView);
	auto frame = m_framePool.TryGetNextFrame();
	if (frame != NULL)
	{
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

		result = _renderToSurface.RenderTexture(frameSurface.get());

		if (newSize)
		{
			m_framePool.Recreate(
				m_direct3d_device,
				winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
				2,
				m_lastSize);
		}
	}
	else if (isNewSurface || isNewtargetView)
	{
		result = _renderToSurface.RenderTexture(nullptr);//render old frame
	}

	return result;
}

BOOL WinrtGraphicCapture::CaptureImage(void* data, UINT32 width, UINT32 height, UINT32 linesize)
{
	auto frame = m_framePool.TryGetNextFrame();
	if (frame != NULL)
	{
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

		ComPtr<ID3D11Device> d3dDevice = this->_renderToSurface.GetDevive();
		ComPtr<ID3D11DeviceContext> d3dDeviceCtx = this->_renderToSurface.GetDeviceContext();

		bool isRecreate = false;
		D3D11_TEXTURE2D_DESC desc;
		isRecreate = !_cpuCaptureTexture.Get();
		if (!isRecreate)
		{
			_cpuCaptureTexture->GetDesc(&desc);
			isRecreate = desc.Width != m_lastSize.Width || desc.Height != m_lastSize.Height;
		}
		if (isRecreate)
		{
			frameSurface->GetDesc(&desc);
			//desc.Width = m_lastSize.Width;
			//desc.Height = m_lastSize.Height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;//0
			desc.Usage = D3D11_USAGE_DYNAMIC;// D3D11_USAGE_DEFAULT;
			desc.MiscFlags = 0;
			desc.CPUAccessFlags = D3D10_CPU_ACCESS_READ;
			HRESULT hr = d3dDevice->CreateTexture2D(&desc, NULL, _cpuCaptureTexture.ReleaseAndGetAddressOf());
			if (FAILED(hr))
				return FALSE;
		}
		d3dDeviceCtx->CopyResource(_cpuCaptureTexture.Get(), frameSurface.get());

		if (newSize)
		{
			m_framePool.Recreate(
				m_direct3d_device,
				winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
				2,
				m_lastSize);
		}

		D3D11_MAPPED_SUBRESOURCE map;
		d3dDeviceCtx->Map(_cpuCaptureTexture.Get(), 0, D3D11_MAP::D3D11_MAP_READ, 0, &map);

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
		d3dDeviceCtx->Unmap(_cpuCaptureTexture.Get(), 0);
	}
	return FALSE;
}

VOID WinrtGraphicCapture::Close()
{
	if (m_framePool) m_framePool.Close();
	if (m_session) m_session.Close();

	m_framePool = nullptr;
	m_session = nullptr;
	m_item = nullptr;
}

BOOL WinrtGraphicCapture::GetSize(UINT32& width, UINT32& height)
{
	width = m_lastSize.Width;
	height = m_lastSize.Height;
	return TRUE;
}