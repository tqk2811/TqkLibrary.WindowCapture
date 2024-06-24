#ifndef _WindowCapture_H_HdcHelper_H_
#define _WindowCapture_H_HdcHelper_H_

#include "WinApi.hpp"
#include "Directx.hpp"
#include "Gdi.hpp"
#include "Md5Helper.hpp"

enum Hdc_Capture_Mode
{
	Hdc_Capture_Mode_BitBlt,
	Hdc_Capture_Mode_PrintWindow
};
BOOL HDC_GetSize(const HWND hwnd, UINT32& width, UINT32& height);

HBITMAP HDC_CaptureToHBitmap(HWND hwnd, Hdc_Capture_Mode mode);

BOOL HDC_CopyBitmapToTexture(
	const HBITMAP hBitmap,
	const HDC hdc,
	ID3D11Device* device,
	ID3D11DeviceContext* deviceCtx,
	ComPtr<ID3D11Texture2D>& texture,
	Md5Helper* pMd5Helper,
	BYTE* hash = nullptr,
	const BYTE const* oldHash = nullptr
);
#endif