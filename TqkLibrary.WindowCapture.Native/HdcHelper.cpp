#include "HdcHelper.hpp"
#include "Utils.hpp"
#define __CheckBitmap__

BOOL HDC_GetSize(const HWND hwnd, UINT32& width, UINT32& height)
{
	BOOL result = FALSE;
	HDC hdcSource{ 0 };

	hdcSource = GetDC(hwnd);
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
	if (!GetClientRect(hwnd, &rcClient))
		goto end;

	width = rcClient.right - rcClient.left;
	height = rcClient.bottom - rcClient.top;
#endif
	result = TRUE;

end:
	if (hdcSource)
		ReleaseDC(hwnd, hdcSource);
	return result;
}


HBITMAP HDC_CaptureToHBitmap(HWND hwnd, Hdc_Capture_Mode mode)
{
	BOOL isSuccess = FALSE;
	int width;
	int height;

	//release
	HDC hdcSource{ 0 };
	HDC hdcDest{ 0 };

	//return
	HBITMAP hBitmap{ 0 };


	hdcSource = GetDC(hwnd);
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
		if (!GetClientRect(hwnd, &rcClient))
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

	switch (mode)
	{
	case Hdc_Capture_Mode::Hdc_Capture_Mode_BitBlt:
		if (!BitBlt(hdcDest, 0, 0, width, height, hdcSource, 0, 0, SRCCOPY))
			goto end;
		isSuccess = TRUE;
		break;

	case Hdc_Capture_Mode::Hdc_Capture_Mode_PrintWindow:
		if (!PrintWindow(hwnd, hdcDest, 0))
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
		ReleaseDC(hwnd, hdcSource);
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


BOOL HDC_CopyBitmapToTexture(
	const HBITMAP hBitmap,
	const HDC hdc,
	ID3D11Device* device,
	ID3D11DeviceContext* deviceCtx,
	ComPtr<ID3D11Texture2D>& texture
#ifdef HashHelper_Enable
	,HashHelper* pHashHelper,
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
	bi.biHeight = bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	dwBmpSize = ((bitmap.bmWidth * 32 /*+ 31*/) / 32) * 4 * bitmap.bmHeight;
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
		if (FAILED(hr))
			goto end;
	}

	hr = deviceCtx->Map(texture.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &map);
	if (FAILED(hr))
		goto end;

	{
		UINT64 bitmapLineSize = 4 * bitmap.bmWidth;
		UINT64 sizeCopy = min(bitmapLineSize, map.RowPitch);

		//bit map flipped Y-axis
		for (int i = 0; i < bitmap.bmHeight; i++)
		{
			memcpy(
				(void*)((UINT64)map.pData + (UINT64)(map.RowPitch * (bitmap.bmHeight - 1 - i))),
				(void*)((UINT64)pData + (UINT64)(bitmapLineSize * i)),
				sizeCopy
			);
		}
	}
	deviceCtx->Unmap(texture.Get(), 0);
	result = TRUE;

end:
	if (pData)
		delete[] pData;
	return result;
}