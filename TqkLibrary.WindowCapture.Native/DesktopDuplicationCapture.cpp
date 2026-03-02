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

// NOT_IMPLEMENTED: Desktop Duplication API capture is not yet implemented
BOOL DesktopDuplicationCapture::Draw(ID3D11Device* device, ID3D11DeviceContext* deviceCtx, ComPtr<ID3D11Texture2D>& textureBGRA)
{
	return FALSE; // NOT_IMPLEMENTED
}
HBITMAP DesktopDuplicationCapture::Shoot()
{
	return NULL; // NOT_IMPLEMENTED
}


BOOL DesktopDuplicationCapture::InitMonitorCapture(HMONITOR hmonitor)
{
	if (!hmonitor)
		return FALSE;

	if (!this->IsValidMonitor(hmonitor))
		return FALSE;

	this->m_hmonitor = hmonitor;
	return TRUE;
}
BOOL DesktopDuplicationCapture::GetSize(UINT32& width, UINT32& height)
{
	return FALSE; // NOT_IMPLEMENTED
}
BOOL DesktopDuplicationCapture::Render(IDXGISurface* surface, bool isNewSurface, bool& isNewtargetView)
{
	return FALSE; // NOT_IMPLEMENTED
}
BOOL DesktopDuplicationCapture::CaptureImage(void* data, UINT32 width, UINT32 height, UINT32 linesize)
{
	return FALSE; // NOT_IMPLEMENTED
}
BOOL DesktopDuplicationCapture::IsSupported()
{
	return FALSE; // NOT_IMPLEMENTED
}