#ifndef _WindowCapture_H_HdcCapture_H_
#define _WindowCapture_H_HdcCapture_H_

#include "BaseCapture.hpp"
#include "Utils.hpp"

#include "WinApi.hpp"
#include "Directx.hpp"
#include "Gdi.hpp"
#include "HashHelper.hpp"

enum HdcCaptureMode: BYTE
{
	HdcCaptureMode_BitBlt,
	HdcCaptureMode_PrintWindow,
};

class HdcCapture : public BaseCapture
{
public:
	HdcCapture();
	~HdcCapture();

	HdcCaptureMode GetMode();
	VOID SetMode(HdcCaptureMode mode);

	BOOL InitCapture(HWND hWnd);
	BOOL Draw(ID3D11Device* device, ID3D11DeviceContext* deviceCtx, ComPtr<ID3D11Texture2D>& texture);
	HBITMAP Shoot();
	BOOL GetSize(UINT32& width, UINT32& height);
private:
	HDC _hdc{ 0 };
	HdcCaptureMode _mode{ HdcCaptureMode_BitBlt };

	HBITMAP CaptureToHBitmap(HdcCaptureMode mode);

	BOOL CopyBitmapToTexture(
		const HBITMAP hBitmap,
		const HDC hdc,
		ID3D11Device* device,
		ID3D11DeviceContext* deviceCtx,
		ComPtr<ID3D11Texture2D>& texture
#ifdef HashHelper_Enable
		, HashHelper* pHashHelper,
		BYTE* oldHash
#endif
	);
};

TqkLibrary_WindowCapture_Export HdcCapture* HdcCapture_Alloc();
TqkLibrary_WindowCapture_Export HdcCaptureMode HdcCapture_GetMode(HdcCapture* hdcCapture);
TqkLibrary_WindowCapture_Export VOID HdcCapture_SetMode(HdcCapture* hdcCapture, HdcCaptureMode mode);

#endif