#include "BitbltCapture.hpp"
#define __CheckBitmap__

BitbltCapture* BitbltCapture_Alloc()
{
	return new BitbltCapture();
}


BitbltCapture::BitbltCapture() {
	d3d = new D3DClass();
}
BitbltCapture::~BitbltCapture() {
	if (_hdc)
		DeleteDC(_hdc);

	if (d3d)
		delete d3d;
}

BOOL BitbltCapture::InitCapture(HWND hWnd) {
	this->m_hWnd = hWnd;

	this->_hdc = CreateCompatibleDC(NULL);

	return _hdc != 0;
}

HBITMAP CaptureToHBitmap(HWND hwnd)
{
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

	if (!BitBlt(hdcDest, 0, 0, width, height, hdcSource, 0, 0, SRCCOPY))
		goto end;

	if (!SelectObject(hdcDest, hBitmap))
		goto end;

end:
	if (hdcDest)
		DeleteDC(hdcDest);
	if (hdcSource)
		ReleaseDC(hwnd, hdcSource);

	return hBitmap;
}


BOOL BitbltCapture::Draw(ID3D11Device* device, ID3D11DeviceContext* deviceCtx, ComPtr<ID3D11Texture2D>& texture)
{
	BOOL result = FALSE;

	BITMAPINFOHEADER bi{ };
	DWORD dwBmpSize = 0;
	int res = 0;
	HRESULT hr{};

	//release
	BITMAP bitmap{};
	void* pData{ nullptr };
	HBITMAP hBitmap{};
	D3D11_MAPPED_SUBRESOURCE map{};

	hBitmap = CaptureToHBitmap(this->m_hWnd);
	if (!hBitmap)
		goto end;

	SelectObject(this->_hdc, hBitmap);
	GetObject(hBitmap, sizeof(BITMAP), &bitmap);

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

	res = GetDIBits(this->_hdc, hBitmap, 0,
		(UINT)bitmap.bmHeight,
		pData,
		(BITMAPINFO*)&bi,
		DIB_RGB_COLORS);

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
	if (hBitmap)
		DeleteObject(hBitmap);

	return result;
}

HBITMAP BitbltCapture::Shoot()
{
	return CaptureToHBitmap(this->m_hWnd);
}

BOOL BitbltCapture::GetSize(UINT32& width, UINT32& height)
{
	return FALSE;
}