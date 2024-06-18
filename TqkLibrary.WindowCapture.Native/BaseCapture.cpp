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
BOOL BaseCapture_InitCapture(BaseCapture* pBaseCapture, HWND hwnd)
{
	if (pBaseCapture)
		return pBaseCapture->InitCapture(hwnd);
	return FALSE;
}
BOOL BaseCapture_GetSize(BaseCapture* pBaseCapture, UINT32& width, UINT32& height)
{
	if (pBaseCapture)
		return pBaseCapture->GetSize(width, height);
	return FALSE;

}
HBITMAP BaseCapture_Shoot(BaseCapture* pBitbltCapture)
{
	if (pBitbltCapture)
		return pBitbltCapture->Shoot();
	return NULL;
}

BOOL HBITMAP_Release(HBITMAP hbitmap)
{
	return DeleteObject(hbitmap);
}