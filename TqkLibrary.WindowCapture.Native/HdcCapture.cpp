#include "HdcCapture.hpp"

HdcCapture* HdcCapture_Alloc()
{
	return new HdcCapture();
}
HdcCaptureMode HdcCapture_GetMode(HdcCapture* hdcCapture)
{
	return hdcCapture->GetMode();
}
VOID HdcCapture_SetMode(HdcCapture* hdcCapture, HdcCaptureMode mode)
{
	hdcCapture->SetMode(mode);
}





HdcCapture::HdcCapture()
{
	this->_hdc = CreateCompatibleDC(NULL);
	assert(this->_hdc);
}
HdcCapture::~HdcCapture()
{
	if (_hdc)
		DeleteDC(_hdc);
	_hdc = NULL;
}

HdcCaptureMode HdcCapture::GetMode()
{
	return this->_mode;
}
VOID HdcCapture::SetMode(HdcCaptureMode mode)
{
	_mode = mode;
}

BOOL HdcCapture::InitWindowCapture(HWND hwnd)
{
	if (!this->_hdc)
		return FALSE;

	if (!_renderToSurface.Init())
		return FALSE;

	if (!IsValidWindow(hwnd))
		return FALSE;

	this->m_hwnd = hwnd;
	this->m_hmonitor = NULL;

	return TRUE;
}
BOOL HdcCapture::InitMonitorCapture(HMONITOR hmonitor)
{
	if (!this->_hdc)
		return FALSE;

	if (!this->_renderToSurface.Init())
		return FALSE;

	if (!this->IsValidMonitor(hmonitor))
		return FALSE;

	this->m_hmonitor = hmonitor;
	this->m_hwnd = NULL;
	return TRUE;
}

BOOL HdcCapture::Render(IDXGISurface* surface, bool isNewSurface, bool& isNewtargetView)
{
	BOOL result = _renderToSurface.InitializeSurface(surface, isNewSurface, isNewtargetView);

	BITMAP bitmap{};
	HBITMAP hBitmap = CaptureToHBitmap(this->_mode);
	if (!hBitmap || hBitmap == INVALID_HANDLE_VALUE)
		return FALSE;

	HGDIOBJ obj = SelectObject(this->_hdc, hBitmap);
	result = GetObject(hBitmap, sizeof(BITMAP), &bitmap) == sizeof(BITMAP);
	if (result)
	{
		ComPtr<ID3D11Device> device = _renderToSurface.GetDevive();
		ComPtr<ID3D11DeviceContext> deviceCtx = _renderToSurface.GetDeviceContext();

		result = CopyBitmapToTexture(hBitmap, this->_hdc, device.Get(), deviceCtx.Get(), _renderTexture);

		if (result)
			result = _renderToSurface.SendTexture(_renderTexture.Get());

		if (result)
			result = _renderToSurface.Render();
	}

	DeleteObject(hBitmap);

	return result;
}
BOOL HdcCapture::CaptureImage(void* data, UINT32 width, UINT32 height, UINT32 linesize)
{
	BOOL result = FALSE;
	HRESULT hr{};
	BITMAP bitmap{};
	BITMAPINFOHEADER bi{ };
	DWORD dwBmpSize = 0;


	HBITMAP hBitmap = CaptureToHBitmap(this->_mode);
	if (!hBitmap || hBitmap == INVALID_HANDLE_VALUE)
		return FALSE;

	HGDIOBJ obj = SelectObject(this->_hdc, hBitmap);

	result = GetObject(hBitmap, sizeof(BITMAP), &bitmap) == sizeof(BITMAP);
	if (!result)
		goto end;


	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bitmap.bmWidth;
	bi.biHeight = -bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	dwBmpSize = bitmap.bmWidth * 4 * bitmap.bmHeight;

	assert(width == bitmap.bmWidth);
	assert(height == bitmap.bmHeight);
	assert(dwBmpSize == height * linesize);

	result = GetDIBits(this->_hdc, hBitmap, 0,
		(UINT)bitmap.bmHeight,
		data,
		(BITMAPINFO*)&bi,
		DIB_RGB_COLORS) == bitmap.bmHeight;
	if (!result)
		goto end;

end:
	DeleteObject(hBitmap);
	return result;
}

BOOL HdcCapture::GetSize(UINT32& width, UINT32& height)
{
	BOOL result = FALSE;
	HDC hdcSource{ 0 };

	hdcSource = GetDC(this->m_hwnd);
	if (!hdcSource)
		goto end;

#ifdef __CheckBitmap__
	HGDIOBJ srcHbitmap;
	srcHbitmap = GetCurrentObject(hdcSource, OBJ_BITMAP);

	BITMAP srcBitmapHeader;
	memset(&srcBitmapHeader, 0, sizeof(BITMAP));
	GetObject(srcHbitmap, sizeof(BITMAP), &srcBitmapHeader);

	width = srcBitmapHeader.bmWidth;
	height = srcBitmapHeader.bmHeight;
#else
	RECT rcClient;
	if (!GetClientRect(this->m_hwnd, &rcClient))
		goto end;

	width = rcClient.right - rcClient.left;
	height = rcClient.bottom - rcClient.top;
#endif
	result = TRUE;

end:
	if (hdcSource)
		ReleaseDC(this->m_hwnd, hdcSource);
	return result;
}



HBITMAP HdcCapture::CaptureToHBitmap(HdcCaptureMode mode)
{
	BOOL isSuccess = FALSE;
	int width;
	int height;
	int srcX = 0, srcY = 0;

	//release
	HDC hdcSource{ 0 };
	HDC hdcDest{ 0 };

	//return
	HBITMAP hBitmap{ 0 };
	HGDIOBJ oldBitmap = NULL;

	DPI_AWARENESS_CONTEXT oldContext = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	if (this->m_hmonitor)
	{
		MONITORINFO mi = { sizeof(MONITORINFO) };
		if (!GetMonitorInfo(this->m_hmonitor, &mi))
			goto end;
		srcX = mi.rcMonitor.left;
		srcY = mi.rcMonitor.top;
		width = mi.rcMonitor.right - mi.rcMonitor.left;
		height = mi.rcMonitor.bottom - mi.rcMonitor.top;

		hdcSource = GetDC(NULL);
	}
	else if (this->m_hwnd)
	{
		hdcSource = GetDC(this->m_hwnd);

#ifdef __CheckBitmap__
		HGDIOBJ srcHbitmap;
		srcHbitmap = GetCurrentObject(hdcSource, OBJ_BITMAP);

		BITMAP srcBitmapHeader;
		memset(&srcBitmapHeader, 0, sizeof(BITMAP));
		GetObject(srcHbitmap, sizeof(BITMAP), &srcBitmapHeader);

		width = srcBitmapHeader.bmWidth;
		height = srcBitmapHeader.bmHeight;
#else
		RECT rcClient;
		if (!GetClientRect(this->m_hwnd, &rcClient))
			goto end;

		width = rcClient.right - rcClient.left;
		height = rcClient.bottom - rcClient.top;
#endif
	}
	else goto end;

	if (!hdcSource)
		goto end;

	hdcDest = CreateCompatibleDC(hdcSource);
	if (!hdcDest)
		goto end;

	hBitmap = CreateCompatibleBitmap(hdcSource, width, height);
	if (!hBitmap)
		goto end;

	oldBitmap = SelectObject(hdcDest, hBitmap);

	//SetMapMode(hdcDest, MM_LOENGLISH);
	//SetStretchBltMode(hdcDest, COLORONCOLOR);

	switch (mode)
	{
	case HdcCaptureMode::HdcCaptureMode_BitBlt:
		if (!BitBlt(hdcDest, 0, 0, width, height, hdcSource, srcX, srcY, SRCCOPY))
			goto end;
		isSuccess = TRUE;
		break;

	case HdcCaptureMode::HdcCaptureMode_PrintWindow://PrintWindow only work with HWND
		if (this->m_hwnd)
		{
			if (!PrintWindow(this->m_hwnd, hdcDest, PW_RENDERFULLCONTENT))
				if (!PrintWindow(this->m_hwnd, hdcDest, 0))
					goto end;
			isSuccess = TRUE;
		}
		break;

	default:
		goto end;
	}

	if (oldBitmap) SelectObject(hdcDest, oldBitmap);

end:
	SetThreadDpiAwarenessContext(oldContext);
	if (hdcDest) DeleteDC(hdcDest);
	if (hdcSource) 
	{
		if (this->m_hmonitor)
			ReleaseDC(NULL, hdcSource);
		else if (this->m_hwnd)
			ReleaseDC(this->m_hwnd, hdcSource);
	}
	if (isSuccess) return hBitmap;
	if (hBitmap) DeleteObject(hBitmap);
	return NULL;
}

BOOL HdcCapture::CopyBitmapToTexture(
	const HBITMAP hBitmap,
	const HDC hdc,
	ID3D11Device* device,
	ID3D11DeviceContext* deviceCtx,
	ComPtr<ID3D11Texture2D>& texture
)
{
	if (!hBitmap || hBitmap == INVALID_HANDLE_VALUE ||
		!hdc || hdc == INVALID_HANDLE_VALUE ||
		!device ||
		!deviceCtx)
		return FALSE;

	BOOL result = FALSE;

	BITMAPINFOHEADER bi{ };
	DWORD dwBmpSize = 0;
	HRESULT hr{};
	BITMAP bitmap{};
	D3D11_MAPPED_SUBRESOURCE map{};

	//release
	void* pData{ nullptr };

	{
		HGDIOBJ obj = SelectObject(hdc, hBitmap);
		result = obj != NULL;
		if (!result)
			goto end;
	}

	result = GetObject(hBitmap, sizeof(BITMAP), &bitmap) == sizeof(BITMAP);
	if (!result)
		goto end;


	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bitmap.bmWidth;
	bi.biHeight = -bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	dwBmpSize = bitmap.bmWidth * 4 * bitmap.bmHeight;
	pData = new BYTE[dwBmpSize];

	result = GetDIBits(hdc, hBitmap, 0,
		(UINT)bitmap.bmHeight,
		pData,
		(BITMAPINFO*)&bi,
		DIB_RGB_COLORS) == bitmap.bmHeight;
	if (!result)
		goto end;

	{
		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));

		if (texture.Get())
			texture->GetDesc(&texDesc);

		if (texDesc.Width != bitmap.bmWidth ||
			texDesc.Height != bitmap.bmHeight ||
			(texDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE) != D3D11_BIND_SHADER_RESOURCE &&
			texDesc.Usage != D3D11_USAGE_DYNAMIC ||
			texDesc.CPUAccessFlags != D3D11_CPU_ACCESS_WRITE)
		{
			//create new texture
			texDesc.Width = bitmap.bmWidth;
			texDesc.Height = bitmap.bmHeight;
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
			texDesc.SampleDesc.Count = 1;
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;//0
			texDesc.Usage = D3D11_USAGE_DYNAMIC;// D3D11_USAGE_DEFAULT;
			texDesc.MiscFlags = 0;
			texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			hr = device->CreateTexture2D(&texDesc, NULL, texture.ReleaseAndGetAddressOf());
			result = SUCCEEDED(hr);
			if (!result)
				goto end;
		}
	}

	hr = deviceCtx->Map(texture.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &map);
	result = SUCCEEDED(hr);
	if (result)
	{
		UINT64 bitmapLineSize = 4 * bitmap.bmWidth;
		if (map.RowPitch == bitmapLineSize)
		{
			memcpy(map.pData, pData, map.DepthPitch);
		}
		else
		{
			UINT64 sizeCopy = min(bitmapLineSize, map.RowPitch);
			for (int i = 0; i < bitmap.bmHeight; i++)
			{
				memcpy(
					(void*)((UINT64)map.pData + (UINT64)(map.RowPitch * i)),
					(void*)((UINT64)pData + (UINT64)(bitmapLineSize * i)),
					sizeCopy
				);
			}
		}
		deviceCtx->Unmap(texture.Get(), 0);
	}

end:
	if (pData)
		delete[] pData;
	return result;
}