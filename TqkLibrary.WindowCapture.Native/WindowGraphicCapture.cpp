#include "WindowGraphicCapture.hpp"
#include <windows.graphics.capture.interop.h>

WindowGraphicCapture* WindowGraphicCapture_Alloc()
{
	return new WindowGraphicCapture();
}

WindowGraphicCapture::WindowGraphicCapture()
{

}

WindowGraphicCapture::~WindowGraphicCapture()
{
	if (winrtCapture)
	{
		winrtCapture->Close();
		delete winrtCapture;
		winrtCapture = nullptr;
	}
}
BOOL WindowGraphicCapture::InitCapture(HWND hwnd)
{
	if (!winrtCapture)
		winrtCapture = new WinrtGraphicCapture();

	if (!winrtCapture->Init())
		return FALSE;

	if (!winrtCapture->SetCapture(hwnd))
		return FALSE;



	return TRUE;
}
BOOL WindowGraphicCapture::Draw(ID3D11Device* device, ID3D11DeviceContext* deviceCtx, ComPtr<ID3D11Texture2D>& texture)
{
	if (winrtCapture)
	{
		return winrtCapture->Draw(device, deviceCtx, texture);
	}
	return FALSE;
}
HBITMAP WindowGraphicCapture::Shoot()
{
	return NULL;
}
BOOL WindowGraphicCapture::GetSize(UINT32& width, UINT32& height)
{
	if (winrtCapture)
	{
		return winrtCapture->GetSize(width, height);
	}
	return FALSE;
}