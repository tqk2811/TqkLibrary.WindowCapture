#include "BaseCapture.hpp"
#include <dwmapi.h>

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

//https://github.com/microsoft/Windows.UI.Composition-Win32-Samples/blob/a59e7586c0bd1a967e1e25f6ca0363e20151afe5/cpp/ScreenCaptureforHWND/ScreenCaptureforHWND/Win32WindowEnumeration.h#L45
BOOL BaseCapture::IsValidWindow(HWND hwnd)
{
	if (hwnd == INVALID_HANDLE_VALUE || hwnd == 0)
		return FALSE;

	HWND shellWindow = GetShellWindow();
	if (hwnd == shellWindow)
		return FALSE;

	if (!IsWindowVisible(hwnd))
		return FALSE;

	if (GetAncestor(hwnd, GA_ROOT) != hwnd)
		return FALSE;

	LONG style = GetWindowLong(hwnd, GWL_STYLE);
	if (!((style & WS_DISABLED) != WS_DISABLED))
		return FALSE;
	if ((style & WS_EX_TOOLWINDOW) != 0)
		return FALSE;
	if ((style & WS_EX_APPWINDOW) != WS_EX_APPWINDOW)
		return FALSE;

	DWORD cloaked = FALSE;
	HRESULT hrTemp = DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
	if (SUCCEEDED(hrTemp) && cloaked == DWM_CLOAKED_SHELL)
		return FALSE;

	auto len = GetWindowTextLength(hwnd);
	if (len <= 0)
		return FALSE;

	WCHAR* text = new WCHAR[len + 1];
	auto finalLength = GetWindowText(hwnd, text, len + 1);
	delete[] text;

	if (finalLength <= 0)
		return FALSE;

	return TRUE;
}
BOOL IsValidMonitor(HMONITOR HMONITOR)
{
	return TRUE;
}