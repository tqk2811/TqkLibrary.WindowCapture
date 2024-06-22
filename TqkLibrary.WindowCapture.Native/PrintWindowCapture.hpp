#ifndef _WindowCapture_H_DesktopDuplicationCapture_H_
#define _WindowCapture_H_DesktopDuplicationCapture_H_

#include "BaseCapture.hpp"
class PrintWindowCapture : public BaseCapture
{
public:
	PrintWindowCapture();
	~PrintWindowCapture();

	BOOL InitCapture(HWND hWnd);
	BOOL Draw(ID3D11Device* device, ID3D11DeviceContext* deviceCtx, ComPtr<ID3D11Texture2D>& texture);
	HBITMAP Shoot();
	BOOL GetSize(UINT32& width, UINT32& height);
private:
	HDC _hdc{ 0 };
};

TqkLibrary_WindowCapture_Export PrintWindowCapture* PrintWindow_Alloc();


#endif 