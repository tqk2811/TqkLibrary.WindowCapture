#ifndef _WindowCapture_H_BitbltCapture_H_
#define _WindowCapture_H_BitbltCapture_H_

#include "BaseCapture.hpp"
#include "Gdi.hpp"
#include "Utils.hpp"

class BitbltCapture : public BaseCapture
{
public:
	BitbltCapture();
	~BitbltCapture();

	BOOL InitCapture(HWND hWnd);

	BOOL Draw(ID3D11Device* device, ID3D11DeviceContext* deviceCtx, ComPtr<ID3D11Texture2D>& texture);
	HBITMAP Shoot();
	BOOL GetSize(UINT32& width, UINT32& height);

private:
	HDC _hdc{ 0 };
};

TqkLibrary_WindowCapture_Export BitbltCapture* BitbltCapture_Alloc();

#endif // !_H_BitbltCapture_H_
