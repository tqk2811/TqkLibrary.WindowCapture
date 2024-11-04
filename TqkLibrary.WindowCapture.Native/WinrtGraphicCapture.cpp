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



WinrtGraphicCapture::WinrtGraphicCapture()
{

}
WinrtGraphicCapture::~WinrtGraphicCapture()
{
	Close();
}
BOOL WinrtGraphicCapture::Init()
{
	HRESULT hr{ 0 };
	if (!m_d3d11_device.get())
	{
		UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			flags,
			nullptr, 0,
			D3D11_SDK_VERSION,
			m_d3d11_device.put(),
			nullptr,
			nullptr
		);
		if (DXGI_ERROR_UNSUPPORTED == hr)
			hr = D3D11CreateDevice(
				nullptr,
				D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_WARP,
				nullptr,
				flags,
				nullptr, 0,
				D3D11_SDK_VERSION,
				m_d3d11_device.put(),
				nullptr,
				nullptr
			);
		if (FAILED(hr))
			return FALSE;
	}

	if (!m_dxgi_device.get())
	{
		hr = m_d3d11_device->QueryInterface<IDXGIDevice>(
			m_dxgi_device.put()
		);
		if (FAILED(hr))
			return FALSE;
	}

	if (!m_direct3d_device)
	{
		hr = CreateDirect3D11DeviceFromDXGIDevice(m_dxgi_device.get(), (IInspectable**)&m_direct3d_device);
		winrt::check_hresult(hr);
		if (FAILED(hr))
			return FALSE;
	}

	auto d3dDevice = GetDXGIInterfaceFromObject<ID3D11Device>(m_direct3d_device);
	d3dDevice->GetImmediateContext(m_d3dContext.put());

	return TRUE;
}
BOOL WinrtGraphicCapture::SetCapture(HWND hwnd)
{
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

	auto dxgiDevice = this->m_d3d11_device.as<IDXGIDevice2>();
	winrt::com_ptr<IDXGIAdapter> adapter;
	winrt::check_hresult(dxgiDevice->GetParent(winrt::guid_of<IDXGIAdapter>(), adapter.put_void()));
	winrt::com_ptr<IDXGIFactory2> factory;
	winrt::check_hresult(adapter->GetParent(winrt::guid_of<IDXGIFactory2>(), factory.put_void()));

	DXGI_SWAP_CHAIN_DESC1 desc = {};
	desc.Width = static_cast<uint32_t>(m_lastSize.Width);
	desc.Height = static_cast<uint32_t>(m_lastSize.Height);
	desc.Format = static_cast<DXGI_FORMAT>(winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized);
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BufferCount = 2;
	desc.Scaling = DXGI_SCALING_STRETCH;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
	winrt::check_hresult(factory->CreateSwapChainForComposition(
		m_d3d11_device.get(),
		&desc,
		nullptr,
		m_swapChain.put())
	);

	m_framePool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::Create(
		m_direct3d_device,
		winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
		2,
		m_lastSize);

	m_session = m_framePool.CreateCaptureSession(m_item);
	m_frameArrived = m_framePool.FrameArrived(winrt::auto_revoke, { this, &WinrtGraphicCapture::OnFrameArrived });
	m_session.StartCapture();

	return TRUE;
}
VOID WinrtGraphicCapture::Close()
{
	auto expected = false;
	if (m_closed.compare_exchange_strong(expected, true))
	{
		if (m_frameArrived) m_frameArrived.revoke();
		if (m_framePool) m_framePool.Close();
		if (m_session) m_session.Close();

		m_swapChain = nullptr;
		m_framePool = nullptr;
		m_session = nullptr;
		m_item = nullptr;
	}
}
VOID WinrtGraphicCapture::OnFrameArrived(
	winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender,
	winrt::Windows::Foundation::IInspectable const&)
{
	auto newSize = false;

	auto frame = sender.TryGetNextFrame();
	auto frameContentSize = frame.ContentSize();

	if (frameContentSize.Width != m_lastSize.Width ||
		frameContentSize.Height != m_lastSize.Height)
	{
		// The thing we have been capturing has changed size.
		// We need to resize our swap chain first, then blit the pixels.
		// After we do that, retire the frame and then recreate our frame pool.
		newSize = true;
		m_lastSize = frameContentSize;
		m_swapChain->ResizeBuffers(
			2,
			static_cast<uint32_t>(m_lastSize.Width),
			static_cast<uint32_t>(m_lastSize.Height),
			static_cast<DXGI_FORMAT>(winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized),
			0);
	}


	auto frameSurface = GetDXGIInterfaceFromObject<ID3D11Texture2D>(frame.Surface());

	winrt::com_ptr<ID3D11Texture2D> backBuffer;
	winrt::check_hresult(m_swapChain->GetBuffer(0, winrt::guid_of<ID3D11Texture2D>(), backBuffer.put_void()));

	m_d3dContext->CopyResource(backBuffer.get(), frameSurface.get());

	DXGI_PRESENT_PARAMETERS presentParameters = { 0 };
	m_swapChain->Present1(1, 0, &presentParameters);

	if (newSize)
	{
		m_framePool.Recreate(
			m_direct3d_device,
			winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
			2,
			m_lastSize);
	}
}
BOOL WinrtGraphicCapture::GetSize(UINT32& width, UINT32& height)
{
	width = m_lastSize.Width;
	height = m_lastSize.Height;
	return TRUE;
}
BOOL WinrtGraphicCapture::Draw(ID3D11Device* device, ID3D11DeviceContext* deviceCtx, ComPtr<ID3D11Texture2D>& texture)
{
	winrt::com_ptr<ID3D11Texture2D> backBuffer;
	D3D11_TEXTURE2D_DESC backBufferDESC;
	winrt::check_hresult(m_swapChain->GetBuffer(0, winrt::guid_of<ID3D11Texture2D>(), backBuffer.put_void()));

	backBuffer->GetDesc(&backBufferDESC);

	backBufferDESC.Usage = D3D11_USAGE::D3D11_USAGE_STAGING;
	backBufferDESC.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_READ;
	backBufferDESC.BindFlags = 0;// D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	winrt::com_ptr<ID3D11Texture2D> cloneOfBackBuffer;
	winrt::check_hresult(this->m_d3d11_device->CreateTexture2D(&backBufferDESC, NULL, cloneOfBackBuffer.put()));

	m_d3dContext->CopyResource(cloneOfBackBuffer.get(), backBuffer.get());


	backBuffer->GetDesc(&backBufferDESC);
	D3D11_MAPPED_SUBRESOURCE backBufferMap;
	winrt::check_hresult(this->m_d3dContext->Map(cloneOfBackBuffer.get(), 0, D3D11_MAP_READ, 0, &backBufferMap));

	bool isCreateNewTexture = false;
	if (texture.Get())
	{
		D3D11_TEXTURE2D_DESC textureDESC;
		texture->GetDesc(&textureDESC);

		if (backBufferDESC.Width != textureDESC.Width || backBufferDESC.Height != textureDESC.Height)
			isCreateNewTexture = true;

		if (textureDESC.Usage != D3D11_USAGE::D3D11_USAGE_DYNAMIC ||
			textureDESC.BindFlags != D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE ||
			textureDESC.CPUAccessFlags != D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE)
			isCreateNewTexture = true;
	}
	else
	{
		isCreateNewTexture = true;
	}
	if (isCreateNewTexture)
	{
		backBufferDESC.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;// D3D11_USAGE_DEFAULT;
		backBufferDESC.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
		backBufferDESC.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
		winrt::check_hresult(device->CreateTexture2D(&backBufferDESC, NULL, texture.ReleaseAndGetAddressOf()));
	}

	D3D11_MAPPED_SUBRESOURCE copyMap;
	winrt::check_hresult(deviceCtx->Map(texture.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &copyMap));

	memcpy(copyMap.pData, backBufferMap.pData, backBufferMap.DepthPitch);

	this->m_d3dContext->Unmap(cloneOfBackBuffer.get(), 0);
	deviceCtx->Unmap(texture.Get(), 0);

	return TRUE;
}