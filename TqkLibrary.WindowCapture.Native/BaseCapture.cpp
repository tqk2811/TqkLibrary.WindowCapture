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