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

BOOL HdcCapture::InitCapture(HWND hWnd)
{
	if (!this->_hdc)
		return FALSE;

	if (!_renderToSurface.Init())
		return FALSE;

	if (!hWnd || hWnd == INVALID_HANDLE_VALUE)
		return FALSE;
	this->m_hWnd = hWnd;

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

		result = CopyBitmapToTexture(hBitmap, this->_hdc, device.Get(), deviceCtx.Get(), _renderTexture
#ifdef HashHelper_Enable
			, m_HashHelper, this->m_hash
#endif
		);

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

	hdcSource = GetDC(this->m_hWnd);
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
	if (!GetClientRect(this->m_hWnd, &rcClient))
		goto end;

	width = rcClient.right - rcClient.left;
	height = rcClient.bottom - rcClient.top;
#endif
	result = TRUE;

end:
	if (hdcSource)
		ReleaseDC(this->m_hWnd, hdcSource);
	return result;
}



HBITMAP HdcCapture::CaptureToHBitmap(HdcCaptureMode mode)
{
	BOOL isSuccess = FALSE;
	int width;
	int height;

	//release
	HDC hdcSource{ 0 };
	HDC hdcDest{ 0 };

	//return
	HBITMAP hBitmap{ 0 };


	hdcSource = GetDC(this->m_hWnd);
	if (!hdcSource)
		goto end;

	{
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
		if (!GetClientRect(this->m_hWnd, &rcClient))
			goto end;

		width = rcClient.right - rcClient.left;
		height = rcClient.bottom - rcClient.top;
#endif
	}

	hdcDest = CreateCompatibleDC(hdcSource);
	if (!hdcDest)
		goto end;

	hBitmap = CreateCompatibleBitmap(hdcSource, width, height);
	if (!hBitmap)
		goto end;

	if (!SelectObject(hdcDest, hBitmap))
		goto end;

	//SetMapMode(hdcDest, MM_LOENGLISH);
	//SetStretchBltMode(hdcDest, COLORONCOLOR);

	switch (mode)
	{
	case HdcCaptureMode::HdcCaptureMode_BitBlt:
		if (!BitBlt(hdcDest, 0, 0, width, height, hdcSource, 0, 0, SRCCOPY))
			goto end;
		isSuccess = TRUE;
		break;

	case HdcCaptureMode::HdcCaptureMode_PrintWindow:
		if (!PrintWindow(this->m_hWnd, hdcDest, 0))
			goto end;
		isSuccess = TRUE;
		break;

	default:
		goto end;
	}

	if (!SelectObject(hdcDest, hBitmap))
		goto end;

end:
	if (hdcDest)
		DeleteDC(hdcDest);
	if (hdcSource)
		ReleaseDC(this->m_hWnd, hdcSource);
	if (isSuccess)
	{
		return hBitmap;
	}
	else
	{
		if (hBitmap)
			DeleteObject(hBitmap);
		return NULL;
	}
}

BOOL HdcCapture::CopyBitmapToTexture(
	const HBITMAP hBitmap,
	const HDC hdc,
	ID3D11Device* device,
	ID3D11DeviceContext* deviceCtx,
	ComPtr<ID3D11Texture2D>& texture
#ifdef HashHelper_Enable
	, HashHelper* pHashHelper,
	BYTE* oldHash
#endif
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

#ifdef HashHelper_Enable
	if (pHashHelper && oldHash)
	{
		BYTE currentHash[HashHelper_HashSize];
		result = pHashHelper->CalcHash((const BYTE const*)pData, dwBmpSize, currentHash, HashHelper_HashSize) == HashHelper_HashSize;
		if (!result)
			goto end;

		result = memcmp(currentHash, oldHash, HashHelper_HashSize) != 0;//same hash => no new image
		if (result)
		{
			memcpy(oldHash, currentHash, HashHelper_HashSize);
		}
		else
		{
			goto end;
		}
	}
#endif // #ifdef HashHelper_HashSize

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