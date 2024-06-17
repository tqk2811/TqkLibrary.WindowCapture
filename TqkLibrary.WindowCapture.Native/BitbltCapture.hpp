#ifndef _WindowCapture_H_BitbltCapture_H_
#define _WindowCapture_H_BitbltCapture_H_

#include "BaseCapture.hpp"
#include "Gdi.hpp"

class BitbltCapture : public BaseCapture
{
public:
	BitbltCapture();
	~BitbltCapture();

	BOOL InitCapture(HWND hWnd);

	BOOL Draw(ID3D11Device* device, ID3D11DeviceContext* deviceCtx, ComPtr<ID3D11Texture2D>& texture);
	HBITMAP Shoot();

private:
	HWND m_hWnd{ 0 };


	D3DClass* d3d{ nullptr };
};


TqkLibrary_WindowCapture_Export BitbltCapture* BitbltCapture_Alloc();
TqkLibrary_WindowCapture_Export VOID BitbltCapture_Free(BitbltCapture** ppBitbltCapture);

TqkLibrary_WindowCapture_Export BOOL BitbltCapture_InitCapture(BitbltCapture* pBitbltCapture, HWND hWnd);
TqkLibrary_WindowCapture_Export HBITMAP BitbltCapture_Shoot(BitbltCapture* pBitbltCapture);
TqkLibrary_WindowCapture_Export BOOL HBITMAP_Release(HBITMAP hbitmap);

#endif // !_H_BitbltCapture_H_
