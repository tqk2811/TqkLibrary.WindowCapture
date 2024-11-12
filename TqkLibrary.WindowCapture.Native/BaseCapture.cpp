#include "BaseCapture.hpp"


VOID BaseCapture_Free(BaseCapture** ppBaseCapture)
{
	if (ppBaseCapture)
	{
		BaseCapture* pBaseCapture = *ppBaseCapture;
		if (pBaseCapture)
		{
			delete pBaseCapture;
			*ppBaseCapture = nullptr;
		}
	}
}
BOOL BaseCapture_InitWindowCapture(BaseCapture* pBaseCapture, HWND hwnd)
{
	if (pBaseCapture)
		return pBaseCapture->InitWindowCapture(hwnd);
	return FALSE;
}
BOOL BaseCapture_InitMonitorCapture(BaseCapture* pBaseCapture, HMONITOR HMONITOR)
{
	if (pBaseCapture)
		return pBaseCapture->InitMonitorCapture(HMONITOR);
	return FALSE;
}
BOOL BaseCapture_GetSize(BaseCapture* pBaseCapture, UINT32& width, UINT32& height)
{
	if (pBaseCapture)
		return pBaseCapture->GetSize(width, height);
	return FALSE;

}
BOOL BaseCapture_IsSupported(BaseCapture* pBaseCapture)
{
	if (pBaseCapture)
		return pBaseCapture->IsSupported();
	return FALSE;
}

BOOL BaseCapture_Render(BaseCapture* pBaseCapture, IDXGISurface* surface, bool isNewSurface, bool& isNewtargetView)
{
	if (!pBaseCapture)
		return FALSE;
	return pBaseCapture->Render(surface, isNewSurface, isNewtargetView);
}
BOOL BaseCapture_CaptureImage(BaseCapture* pBaseCapture, void* data, UINT32 width, UINT32 height, UINT32 lineSize)
{
	if (!pBaseCapture)
		return FALSE;
	return pBaseCapture->CaptureImage(data, width, height, lineSize);
}



BaseCapture::BaseCapture()
{
#ifdef HashHelper_Enable
	ZeroMemory(this->m_hash, HashHelper_HashSize);
	this->m_HashHelper = new HashHelper();
#endif // HashHelper_HashSize
}

BaseCapture::~BaseCapture()
{
#ifdef HashHelper_Enable
	if (this->m_HashHelper)
		delete this->m_HashHelper;
	this->m_HashHelper = nullptr;
#endif // HashHelper_HashSize
}

BOOL BaseCapture::IsValidWindow(HWND hWnd)
{
	return TRUE;
}
BOOL IsValidMonitor(HMONITOR HMONITOR)
{
	return TRUE;
}