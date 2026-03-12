#include "DesktopDuplicationCapture.hpp"

DesktopDuplicationCapture* DesktopDuplication_Alloc()
{
	return new DesktopDuplicationCapture();
}



DesktopDuplicationCapture::DesktopDuplicationCapture()
{
}

DesktopDuplicationCapture::~DesktopDuplicationCapture()
{
	std::lock_guard<std::recursive_mutex> lock(_mtx);
	CloseDuplication();
}

BOOL DesktopDuplicationCapture::IsSupported()
{
	// Desktop Duplication API requires DXGI 1.2 (Windows 8+)
	ComPtr<IDXGIFactory2> factory;
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory2), (void**)factory.GetAddressOf());
	return SUCCEEDED(hr) ? TRUE : FALSE;
}

BOOL DesktopDuplicationCapture::InitMonitorCapture(HMONITOR hmonitor)
{
	if (!hmonitor)
		return FALSE;

	if (!this->IsValidMonitor(hmonitor))
		return FALSE;

	if (!_renderToSurface.Init())
		return FALSE;

	MONITORINFO mi = { sizeof(MONITORINFO) };
	if (!GetMonitorInfo(hmonitor, &mi))
		return FALSE;

	m_hmonitor = hmonitor;
	_width = mi.rcMonitor.right - mi.rcMonitor.left;
	_height = mi.rcMonitor.bottom - mi.rcMonitor.top;

	return InitDuplication();
}

BOOL DesktopDuplicationCapture::InitDuplication()
{
	std::lock_guard<std::recursive_mutex> lock(_mtx);

	CloseDuplication();

	ComPtr<ID3D11Device> device = _renderToSurface.GetDevive();
	if (!device)
		return FALSE;

	ComPtr<IDXGIDevice> dxgiDevice;
	HRESULT hr = device->QueryInterface(__uuidof(IDXGIDevice), (void**)dxgiDevice.GetAddressOf());
	if (FAILED(hr))
		return FALSE;

	ComPtr<IDXGIAdapter> dxgiAdapter;
	hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)dxgiAdapter.GetAddressOf());
	if (FAILED(hr))
		return FALSE;

	// Find the DXGI output matching our monitor
	UINT outputIndex = 0;
	ComPtr<IDXGIOutput> output;
	while (SUCCEEDED(dxgiAdapter->EnumOutputs(outputIndex++, output.ReleaseAndGetAddressOf())))
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);
		if (desc.Monitor == m_hmonitor)
			break;
		output.Reset();
	}

	if (!output)
		return FALSE;

	ComPtr<IDXGIOutput1> output1;
	hr = output->QueryInterface(__uuidof(IDXGIOutput1), (void**)output1.GetAddressOf());
	if (FAILED(hr))
		return FALSE;

	hr = output1->DuplicateOutput(device.Get(), _duplication.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return FALSE;

	_lastPresentTime = {};
	_renderedTime = {};

	return TRUE;
}

VOID DesktopDuplicationCapture::CloseDuplication()
{
	// No need to call ReleaseFrame - we always release frames immediately in AcquireFrame
	_duplication.Reset();
	_tmpFrame.Reset();
}

BOOL DesktopDuplicationCapture::AcquireFrame()
{
	// Caller must hold _mtx
	if (!_duplication)
		return FALSE;

	ComPtr<IDXGIResource> resource;
	DXGI_OUTDUPL_FRAME_INFO frameInfo{};
	HRESULT hr = _duplication->AcquireNextFrame(0, &frameInfo, resource.GetAddressOf());

	if (hr == DXGI_ERROR_WAIT_TIMEOUT)
		return FALSE; // No new frame available

	if (hr == DXGI_ERROR_ACCESS_LOST)
	{
		// Desktop mode or GPU changed - attempt to recreate duplication
		// CloseDuplication inside InitDuplication is safe because _mtx is recursive
		InitDuplication();
		return FALSE;
	}

	if (FAILED(hr))
		return FALSE;

	// Skip frames that only contain mouse pointer updates (no new desktop content)
	if (frameInfo.LastPresentTime.QuadPart == 0)
	{
		_duplication->ReleaseFrame();
		return FALSE;
	}

	ComPtr<ID3D11Texture2D> frameTexture;
	hr = resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)frameTexture.GetAddressOf());
	if (FAILED(hr))
	{
		_duplication->ReleaseFrame();
		return FALSE;
	}

	ComPtr<ID3D11Device> device = _renderToSurface.GetDevive();
	ComPtr<ID3D11DeviceContext> deviceCtx = _renderToSurface.GetDeviceContext();

	// Create or recreate staging texture if needed
	D3D11_TEXTURE2D_DESC desc;
	frameTexture->GetDesc(&desc);

	bool recreate = !_tmpFrame.Get();
	if (!recreate)
	{
		D3D11_TEXTURE2D_DESC curDesc;
		_tmpFrame->GetDesc(&curDesc);
		recreate = curDesc.Width != desc.Width || curDesc.Height != desc.Height;
	}

	if (recreate)
	{
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BindFlags = 0;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.MiscFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

		hr = device->CreateTexture2D(&desc, nullptr, _tmpFrame.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			_duplication->ReleaseFrame();
			return FALSE;
		}

		// Update stored size from actual frame
		_width = desc.Width;
		_height = desc.Height;
	}

	deviceCtx->CopyResource(_tmpFrame.Get(), frameTexture.Get());
	_lastPresentTime = frameInfo.LastPresentTime;

	_duplication->ReleaseFrame();
	return TRUE;
}

BOOL DesktopDuplicationCapture::GetSize(UINT32& width, UINT32& height)
{
	if (!m_hmonitor || _width == 0 || _height == 0)
		return FALSE;

	width = _width;
	height = _height;
	return TRUE;
}

BOOL DesktopDuplicationCapture::Render(IDXGISurface* surface, bool isNewSurface, bool& isNewtargetView)
{
	std::lock_guard<std::recursive_mutex> lock(_mtx);

	if (!_duplication)
		return FALSE;

	BOOL result = _renderToSurface.InitializeSurface(surface, isNewSurface, isNewtargetView);

	bool newFrame = AcquireFrame();

	if (!_tmpFrame.Get())
		return FALSE;

	if (newFrame || isNewSurface || isNewtargetView || _lastPresentTime.QuadPart != _renderedTime.QuadPart)
	{
		result = _renderToSurface.SendTexture(_tmpFrame.Get());
		if (result)
		{
			result = _renderToSurface.Render();
			_renderedTime = _lastPresentTime;
		}
	}
	else
	{
		result = FALSE;
	}

	return result;
}

BOOL DesktopDuplicationCapture::CaptureImage(void* data, UINT32 width, UINT32 height, UINT32 linesize)
{
	std::lock_guard<std::recursive_mutex> lock(_mtx);

	if (!_duplication)
		return FALSE;

	AcquireFrame(); // Try to get new frame; ok to fail (use cached _tmpFrame)

	if (!_tmpFrame.Get())
		return FALSE;

	D3D11_TEXTURE2D_DESC desc;
	_tmpFrame->GetDesc(&desc);

	if (desc.Width != width || desc.Height != height)
		return FALSE;

	ComPtr<ID3D11DeviceContext> deviceCtx = _renderToSurface.GetDeviceContext();
	D3D11_MAPPED_SUBRESOURCE map{};
	HRESULT hr = deviceCtx->Map(_tmpFrame.Get(), 0, D3D11_MAP_READ, 0, &map);
	if (FAILED(hr))
		return FALSE;

	if (map.RowPitch == linesize)
	{
		memcpy(data, map.pData, (size_t)height * linesize);
	}
	else
	{
		UINT rowBytes = min(map.RowPitch, linesize);
		for (UINT32 i = 0; i < height; i++)
		{
			memcpy(
				(void*)((UINT64)data + (UINT64)linesize * i),
				(void*)((UINT64)map.pData + (UINT64)map.RowPitch * i),
				rowBytes
			);
		}
	}

	deviceCtx->Unmap(_tmpFrame.Get(), 0);
	return TRUE;
}
