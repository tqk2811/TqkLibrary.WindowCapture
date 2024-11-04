#ifndef _WindowCapture_H_WindowGraphicCapture_H_
#define _WindowCapture_H_WindowGraphicCapture_H_

#include "BaseCapture.hpp"
#include "WinApi.hpp"
#include "Utils.hpp"

#include "WinrtGraphicCapture.hpp"

class WindowGraphicCapture : public BaseCapture
{
public:
	WindowGraphicCapture();
	~WindowGraphicCapture();

	BOOL InitCapture(HWND hWnd);
	BOOL Draw(ID3D11Device* device, ID3D11DeviceContext* deviceCtx, ComPtr<ID3D11Texture2D>& texture);
	HBITMAP Shoot();
	BOOL GetSize(UINT32& width, UINT32& height);

private:
	WinrtGraphicCapture* winrtCapture{ NULL };
};


TqkLibrary_WindowCapture_Export WindowGraphicCapture* WindowGraphicCapture_Alloc();



#endif