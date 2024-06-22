#include "BitbltCapture.hpp"
#include "HdcHelper.hpp"

BitbltCapture* BitbltCapture_Alloc()
{
	return new BitbltCapture();
}


BitbltCapture::BitbltCapture() {
}
BitbltCapture::~BitbltCapture() {
	if (_hdc)
		DeleteDC(_hdc);
}

BOOL BitbltCapture::InitCapture(HWND hWnd)
{
	if (!hWnd || hWnd == INVALID_HANDLE_VALUE)
		return FALSE;
	this->m_hWnd = hWnd;

	this->_hdc = CreateCompatibleDC(NULL);
	if (!this->_hdc)
		return FALSE;

	return TRUE;
}

BOOL BitbltCapture::Draw(ID3D11Device* device, ID3D11DeviceContext* deviceCtx, ComPtr<ID3D11Texture2D>& texture)
{
	if (!device || !deviceCtx)
		return FALSE;

	HBITMAP hBitmap = HDC_CaptureToHBitmap(this->m_hWnd, Hdc_Capture_Mode_BitBlt);
	if (!hBitmap || hBitmap == INVALID_HANDLE_VALUE)
		return FALSE;

	BOOL result = HDC_CopyBitmapToTexture(hBitmap, this->_hdc, device, deviceCtx, texture);

	DeleteObject(hBitmap);

	return result;
}

HBITMAP BitbltCapture::Shoot()
{
	return HDC_CaptureToHBitmap(this->m_hWnd, Hdc_Capture_Mode_BitBlt);
}

BOOL BitbltCapture::GetSize(UINT32& width, UINT32& height)
{
	return HDC_GetSize(this->m_hWnd, width, height);
}