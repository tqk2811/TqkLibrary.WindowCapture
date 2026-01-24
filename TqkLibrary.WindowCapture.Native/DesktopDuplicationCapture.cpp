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

BOOL DesktopDuplicationCapture::Draw(ID3D11Device* device, ID3D11DeviceContext* deviceCtx, ComPtr<ID3D11Texture2D>& texture)
{
	return NULL;
}
HBITMAP DesktopDuplicationCapture::Shoot()
{
	return NULL;
}


BOOL DesktopDuplicationCapture::InitMonitorCapture(HMONITOR hmonitor)
{
	if (!hmonitor)
		return FALSE;
	this->m_hmonitor = hmonitor;
	return TRUE;
}
BOOL DesktopDuplicationCapture::GetSize(UINT32& width, UINT32& height)
{
	return NULL;
}
BOOL DesktopDuplicationCapture::Render(IDXGISurface* surface, bool isNewSurface, bool& isNewtargetView)
{
	return NULL;
}
BOOL DesktopDuplicationCapture::CaptureImage(void* data, UINT32 width, UINT32 height, UINT32 linesize)
{
	return NULL;
}
BOOL DesktopDuplicationCapture::IsSupported()
{
	return NULL;
}