#include "WinApi.hpp"
#include "Directx.hpp"
#include "VertexShaderClass.hpp"
#include "PixelShaderClass.hpp"

BOOL PRINT_FAILED(HRESULT hr) {
#if _DEBUG
	if (FAILED(hr))
	{
		_com_error err(hr);
		auto msg = err.ErrorMessage();
		OutputDebugString(msg);
		OutputDebugString(L"\r");
		return FALSE;
	}
#endif
	return TRUE;
}

DXGI_FORMAT EnsureNotTypeless(DXGI_FORMAT fmt)
{
	// Assumes UNORM or FLOAT; doesn't use UINT or SINT
	switch (fmt)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS: return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case DXGI_FORMAT_R32G32B32_TYPELESS:    return DXGI_FORMAT_R32G32B32_FLOAT;
	case DXGI_FORMAT_R16G16B16A16_TYPELESS: return DXGI_FORMAT_R16G16B16A16_UNORM;
	case DXGI_FORMAT_R32G32_TYPELESS:       return DXGI_FORMAT_R32G32_FLOAT;
	case DXGI_FORMAT_R10G10B10A2_TYPELESS:  return DXGI_FORMAT_R10G10B10A2_UNORM;
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:     return DXGI_FORMAT_R8G8B8A8_UNORM;
	case DXGI_FORMAT_R16G16_TYPELESS:       return DXGI_FORMAT_R16G16_UNORM;
	case DXGI_FORMAT_R32_TYPELESS:          return DXGI_FORMAT_R32_FLOAT;
	case DXGI_FORMAT_R8G8_TYPELESS:         return DXGI_FORMAT_R8G8_UNORM;
	case DXGI_FORMAT_R16_TYPELESS:          return DXGI_FORMAT_R16_UNORM;
	case DXGI_FORMAT_R8_TYPELESS:           return DXGI_FORMAT_R8_UNORM;
	case DXGI_FORMAT_BC1_TYPELESS:          return DXGI_FORMAT_BC1_UNORM;
	case DXGI_FORMAT_BC2_TYPELESS:          return DXGI_FORMAT_BC2_UNORM;
	case DXGI_FORMAT_BC3_TYPELESS:          return DXGI_FORMAT_BC3_UNORM;
	case DXGI_FORMAT_BC4_TYPELESS:          return DXGI_FORMAT_BC4_UNORM;
	case DXGI_FORMAT_BC5_TYPELESS:          return DXGI_FORMAT_BC5_UNORM;
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:     return DXGI_FORMAT_B8G8R8A8_UNORM;
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:     return DXGI_FORMAT_B8G8R8X8_UNORM;
	case DXGI_FORMAT_BC7_TYPELESS:          return DXGI_FORMAT_BC7_UNORM;
	default:                                return fmt;
	}
}

DXGI_FORMAT _imageFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;

ComPtr<ID3D11Device> _d3d11Device{ nullptr };
ComPtr<ID3D11DeviceContext> _d3d11DeviceCtx{ nullptr };
ComPtr<IDXGIFactory> _dxgiFactory{ nullptr };
bool Init_D3D11_DXGI()
{
	HRESULT hr;

	// Driver types supported
	D3D_DRIVER_TYPE DriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		//D3D_DRIVER_TYPE_WARP,
		//D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

	// Feature levels supported
	D3D_FEATURE_LEVEL FeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};
	UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);
	D3D_FEATURE_LEVEL FeatureLevel;
	// This flag adds support for surfaces with a different color channel ordering
	// than the default. It is required for compatibility with Direct2D.
	UINT creationFlags =
#if _DEBUG
		D3D11_CREATE_DEVICE_DEBUG |
#endif
		D3D11_CREATE_DEVICE_SINGLETHREADED |
		D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	UINT DriverTypeIndex = 0;
	for (; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
	{
		hr = D3D11CreateDevice(nullptr, DriverTypes[DriverTypeIndex], nullptr, creationFlags, FeatureLevels, NumFeatureLevels,
			D3D11_SDK_VERSION, _d3d11Device.GetAddressOf(), &FeatureLevel, _d3d11DeviceCtx.GetAddressOf());
		if (SUCCEEDED(hr))
		{
			// Device creation succeeded, no need to loop anymore
			break;
		}
	}
	if (FAILED(hr))
		return false;

	hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)_dxgiFactory.ReleaseAndGetAddressOf());

	return SUCCEEDED(hr);
}


RECT _windowRect{ 0 };
ComPtr<IDXGISwapChain> _swapChain{ nullptr };
ComPtr<ID3D11Texture2D> _texture{ nullptr };
ComPtr<ID3D11Texture2D> _textureCache{ nullptr };
//ComPtr<ID3D11Resource> _copyTexture{ nullptr };
//ComPtr<IDXGIResource> _res{ nullptr };
VertexShaderClass* _vs = new VertexShaderClass();
PixelShaderClass* _ps = new PixelShaderClass();
ComPtr<ID3D11ShaderResourceView> _resView{ nullptr };
HANDLE _shareHandle{ 0 };
void Init(HWND windowHandler)
{
	if (!Init_D3D11_DXGI())
		return;

	HRESULT hr{ 0 };

	HMONITOR hmonitor = MonitorFromWindow(windowHandler, MONITOR_DEFAULTTONEAREST);
	if (!GetWindowRect(windowHandler, &_windowRect))
		return;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = _windowRect.right - _windowRect.left;
	swapChainDesc.BufferDesc.Height = _windowRect.bottom - _windowRect.top;
	swapChainDesc.BufferDesc.Format = _imageFormat;
	//swapChainDesc.BufferDesc.RefreshRate.Numerator = 30;
	//swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = windowHandler;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SampleDesc.Count = 1;
	//swapChainDesc.SampleDesc.Quality = 0;
	//swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;
	//swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	//swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;
	//swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;//https://devblogs.microsoft.com/directx/dxgi-flip-model/
	hr = _dxgiFactory->CreateSwapChain(_d3d11Device.Get(), &swapChainDesc, _swapChain.ReleaseAndGetAddressOf());
	assert(PRINT_FAILED(hr));

	//ComPtr<IUnknown> deviceUnk;
	//ComPtr<IUnknown> device;
	//if (SUCCEEDED(_swapChain->GetDevice(__uuidof(IUnknown), (void**)deviceUnk.GetAddressOf())))
	//{
	//	if (SUCCEEDED(deviceUnk->QueryInterface(__uuidof(ID3D10Device), (void**)device.GetAddressOf())))
	//	{
	//		int a = 0;
	//	}
	//	else if (SUCCEEDED(deviceUnk->QueryInterface(__uuidof(ID3D10Device1), (void**)device.GetAddressOf())))
	//	{
	//		int a = 0;
	//	}
	//	else if (SUCCEEDED(deviceUnk->QueryInterface(__uuidof(ID3D11Device), (void**)device.GetAddressOf())))
	//	{
	//		int a = 0;
	//	}
	//	else
	//	{
	//		int a = 0;
	//	}
	//}


	_vs->Initialize(_d3d11Device.Get());
	_ps->Initialize(_d3d11Device.Get(), D3D11_FILTER::D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT);



	D3D11_TEXTURE2D_DESC texGameDesc;
	ZeroMemory(&texGameDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texGameDesc.Width = _windowRect.right - _windowRect.left;
	texGameDesc.Height = _windowRect.bottom - _windowRect.top;
	texGameDesc.MipLevels = 1;
	texGameDesc.ArraySize = 1;
	texGameDesc.Format = _imageFormat;
	texGameDesc.SampleDesc.Count = 1;
	texGameDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texGameDesc.Usage = D3D11_USAGE_DEFAULT;
	texGameDesc.MiscFlags = 0;
	hr = _d3d11Device->CreateTexture2D(&texGameDesc, NULL, _texture.ReleaseAndGetAddressOf());
	assert(PRINT_FAILED(hr));

	//texGameDesc.BindFlags = 0;
	//texGameDesc.Usage = D3D11_USAGE_STAGING;
	//texGameDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	//hr = _d3d11Device->CreateTexture2D(&texGameDesc, NULL, _textureCache.ReleaseAndGetAddressOf());
	//assert(PRINT_FAILED(hr));

	//hr = _texture->QueryInterface(__uuidof(ID3D11Resource), (void**)_copyTexture.ReleaseAndGetAddressOf());
	//assert(PRINT_FAILED(hr));

	//hr = _texture->QueryInterface(__uuidof(IDXGIResource), (void**)_res.ReleaseAndGetAddressOf());
	//assert(PRINT_FAILED(hr));

	//hr = _res->GetSharedHandle(&_shareHandle);
	//assert(PRINT_FAILED(hr));



	D3D11_SHADER_RESOURCE_VIEW_DESC const luminancePlaneDesc
		= CD3D11_SHADER_RESOURCE_VIEW_DESC(_texture.Get(), D3D11_SRV_DIMENSION_TEXTURE2D, _imageFormat);
	hr = _d3d11Device->CreateShaderResourceView(_texture.Get(), &luminancePlaneDesc, _resView.GetAddressOf());
	assert(PRINT_FAILED(hr));
}



ComPtr<ID3D11Texture2D> _pOutputResource;
ComPtr<ID3D11RenderTargetView> _renderTargetView{ nullptr };
UINT draw_W{ 0 };
UINT draw_H{ 0 };
BOOL Test(IUnknown* renderSurface, bool isNewSurface) {
	HRESULT hr;

	if (isNewSurface || _renderTargetView.Get() == nullptr)
	{
		ComPtr<IDXGIResource> pDXGIResource;
		hr = renderSurface->QueryInterface(__uuidof(IDXGIResource), (void**)pDXGIResource.GetAddressOf());
		assert(PRINT_FAILED(hr));

		HANDLE sharedHandle;
		hr = pDXGIResource->GetSharedHandle(&sharedHandle);
		assert(PRINT_FAILED(hr));

		ComPtr<ID3D11Resource> tempResource11;
		hr = _d3d11Device->OpenSharedResource(sharedHandle, __uuidof(ID3D11Resource), (void**)tempResource11.GetAddressOf());
		assert(PRINT_FAILED(hr));

		hr = tempResource11->QueryInterface(__uuidof(ID3D11Texture2D), (void**)_pOutputResource.ReleaseAndGetAddressOf());
		assert(PRINT_FAILED(hr));

		D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
		rtDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtDesc.Texture2D.MipSlice = 0;
		hr = _d3d11Device->CreateRenderTargetView(_pOutputResource.Get(), &rtDesc, _renderTargetView.ReleaseAndGetAddressOf());
		assert(PRINT_FAILED(hr));

		D3D11_TEXTURE2D_DESC outputResourceDesc;
		_pOutputResource->GetDesc(&outputResourceDesc);
		draw_W = outputResourceDesc.Width;
		draw_H = outputResourceDesc.Height;
	}


	_d3d11DeviceCtx->ClearState();

	//hr = _swapChain->Present(0, 0);
	//assert(PRINT_FAILED(hr));

	ComPtr<ID3D11Texture2D> backBuffer;
	hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf());
	assert(PRINT_FAILED(hr));

	D3D11_RESOURCE_DIMENSION resType = D3D11_RESOURCE_DIMENSION::D3D11_RESOURCE_DIMENSION_UNKNOWN;
	backBuffer->GetType(&resType);
	if (resType != D3D11_RESOURCE_DIMENSION::D3D11_RESOURCE_DIMENSION_TEXTURE2D)
		return FALSE;

	D3D11_TEXTURE2D_DESC desc;
	backBuffer->GetDesc(&desc);
	if (desc.ArraySize > 1 || desc.MipLevels > 1)
	{
		OutputDebugString(L"WARNING: ScreenGrab does not support 2D arrays, cubemaps, or mipmaps; only the first surface is written. Consider using DirectXTex instead.\n");
	}

	if (desc.SampleDesc.Count > 1)
	{
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;

		ComPtr<ID3D11Texture2D> pTemp;
		hr = _d3d11Device->CreateTexture2D(&desc, nullptr, pTemp.GetAddressOf());
		if (FAILED(hr))
			return FALSE;

		const DXGI_FORMAT fmt = EnsureNotTypeless(desc.Format);
		UINT support = 0;
		hr = _d3d11Device->CheckFormatSupport(fmt, &support);
		if (FAILED(hr))
			return FALSE;

		if (!(support & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RESOLVE))
			return FALSE;

		for (UINT item = 0; item < desc.ArraySize; ++item)
		{
			for (UINT level = 0; level < desc.MipLevels; ++level)
			{
				const UINT index = D3D11CalcSubresource(level, item, desc.MipLevels);
				_d3d11DeviceCtx->ResolveSubresource(pTemp.Get(), index, backBuffer.Get(), index, fmt);
			}
		}

		desc.BindFlags = 0;
		desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_STAGING;
		hr = _d3d11Device->CreateTexture2D(&desc, nullptr, _textureCache.ReleaseAndGetAddressOf());
		if (FAILED(hr))
			return FALSE;

	}
	else if ((desc.Usage == D3D11_USAGE_STAGING) && (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ))
	{
		// Handle case where the source is already a staging texture we can use directly
		_textureCache = backBuffer;
	}
	else
	{
		// Otherwise, create a staging texture from the non-MSAA source
		desc.BindFlags = 0;
		desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_STAGING;

		hr = _d3d11Device->CreateTexture2D(&desc, nullptr, _textureCache.ReleaseAndGetAddressOf());
		if (FAILED(hr))
			return FALSE;

		_d3d11DeviceCtx->CopyResource(_textureCache.Get(), backBuffer.Get());
	}

	_d3d11DeviceCtx->CopyResource(_texture.Get(), _textureCache.Get());


	//_d3d11DeviceCtx->CopyResource(_texture.Get(), tex_shared.Get());
	//_d3d11DeviceCtx->CopyResource(_textureCache.Get(), tex_shared.Get());
	//_d3d11DeviceCtx->ResolveSubresource(_texture.Get(), 0, tex_shared.Get(), 0, _imageFormat);
	//_d3d11DeviceCtx->ResolveSubresource(_textureCache.Get(), 0, tex_shared.Get(), 0, _imageFormat);

	//_d3d11DeviceCtx->Flush();

	//_d3d11DeviceCtx->CopyResource(_textureCache.Get(), _texture.Get());


	D3D11_MAPPED_SUBRESOURCE map;
	hr = _d3d11DeviceCtx->Map(_textureCache.Get(), 0, D3D11_MAP::D3D11_MAP_READ, 0, &map);
	assert(PRINT_FAILED(hr));

	UINT size = map.RowPitch * (_windowRect.bottom - _windowRect.top);
	BYTE* buff = new BYTE[size];
	memcpy(buff, map.pData, size);

	_d3d11DeviceCtx->Unmap(_textureCache.Get(), 0);

	for (UINT i = 0; i < size; i++)
	{
		if (buff[i] != 0)
		{
			//FILE* file = fopen("D:\\myfile.dds", "wb");
			//fwrite(buff, 1, size, file);
			//fclose(file);
			throw 0;
		}
	}


	_d3d11DeviceCtx->ClearState();

	_d3d11DeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_vs->Set(_d3d11DeviceCtx.Get());//VSSetShader IASetInputLayout IASetVertexBuffers

	_ps->Set(_d3d11DeviceCtx.Get(), _resView.Get());

	_d3d11DeviceCtx->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), NULL);

	D3D11_VIEWPORT VP;
	VP.Width = static_cast<FLOAT>(draw_W);
	VP.Height = static_cast<FLOAT>(draw_H);
	VP.MinDepth = 0.0f;
	VP.MaxDepth = 1.0f;
	VP.TopLeftX = 0;
	VP.TopLeftY = 0;
	_d3d11DeviceCtx->RSSetViewports(1, &VP);

	FLOAT color[4]{ 0,255,0,0 };
	_d3d11DeviceCtx->ClearRenderTargetView(_renderTargetView.Get(), &color[0]);

	_d3d11DeviceCtx->Draw(_vs->GetVertexCount(), 0);

	_d3d11DeviceCtx->Flush();


	return true;
}
