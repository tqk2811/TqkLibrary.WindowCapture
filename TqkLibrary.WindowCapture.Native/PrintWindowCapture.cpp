#include "PrintWindowCapture.hpp"
#include "HdcHelper.hpp"

PrintWindowCapture* PrintWindow_Alloc()
{
	return new PrintWindowCapture();
}


PrintWindowCapture::PrintWindowCapture()
{
	this->_hdc = CreateCompatibleDC(NULL);
	assert(this->_hdc);
}
PrintWindowCapture::~PrintWindowCapture()
{
	if (_hdc)
		DeleteDC(_hdc);
	_hdc = NULL;
}

BOOL PrintWindowCapture::InitCapture(HWND hWnd)
{
	if (!this->_hdc)
		return FALSE;

	if (!hWnd || hWnd == INVALID_HANDLE_VALUE)
		return FALSE;
	this->m_hWnd = hWnd;

	return TRUE;
}
BOOL PrintWindowCapture::Draw(ID3D11Device* device, ID3D11DeviceContext* deviceCtx, ComPtr<ID3D11Texture2D>& texture)
{
	if (!device || !deviceCtx)
		return FALSE;

	HBITMAP hBitmap = HDC_CaptureToHBitmap(this->m_hWnd, Hdc_Capture_Mode_PrintWindow);
	if (!hBitmap || hBitmap == INVALID_HANDLE_VALUE)
		return FALSE;

	BOOL result = HDC_CopyBitmapToTexture(hBitmap, this->_hdc, device, deviceCtx, texture
#ifdef HashHelper_HashSize
		, m_HashHelper, this->m_hash
#endif
	);

	DeleteObject(hBitmap);

	return result;
}
HBITMAP PrintWindowCapture::Shoot()
{
	return HDC_CaptureToHBitmap(this->m_hWnd, Hdc_Capture_Mode_PrintWindow);
}
BOOL PrintWindowCapture::GetSize(UINT32& width, UINT32& height)
{
	return HDC_GetSize(this->m_hWnd, width, height);
}