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

}

BOOL DesktopDuplicationCapture::InitCapture(HWND hWnd)
{
	if (!hWnd)
		return FALSE;
	this->m_hWnd = hWnd;

	return TRUE;
}
BOOL DesktopDuplicationCapture::Draw(ID3D11Device* device, ID3D11DeviceContext* deviceCtx, ComPtr<ID3D11Texture2D>& texture)
{
	return NULL;
}
HBITMAP DesktopDuplicationCapture::Shoot()
{
	return NULL;
}
BOOL DesktopDuplicationCapture::GetSize(UINT32& width, UINT32& height)
{
	return NULL;
}