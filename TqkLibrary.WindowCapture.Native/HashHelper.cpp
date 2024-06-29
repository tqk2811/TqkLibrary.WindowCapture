#include "HashHelper.hpp"


HashHelper::HashHelper()
{
	BOOL result = CryptAcquireContext(&this->_hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
	assert(result);

	result = CryptCreateHash(_hProv, CALG_MD5, 0, 0, &this->_hMd5Hash);
	assert(result);
}
HashHelper::~HashHelper()
{
	if (this->_hMd5Hash)
		CryptDestroyHash(this->_hMd5Hash);
	this->_hMd5Hash = NULL;
	if (this->_hProv)
		CryptReleaseContext(this->_hProv, 0);
	this->_hProv = NULL;
}

INT32 HashHelper::CalcHash(const BYTE const* data, const UINT32 dataSize, BYTE* hash, INT32 hashSize)
{
	INT32 size = -1;
	BOOL result = FALSE;

	result = CryptHashData(this->_hMd5Hash, data, dataSize, 0);

	DWORD cbHashSize = 0;
	DWORD dwCount = sizeof(DWORD);
	if (result)
		result = CryptGetHashParam(this->_hMd5Hash, HP_HASHSIZE, (BYTE*)&cbHashSize, &dwCount, 0);
	if (result)
	{
		size = cbHashSize;
		if (hashSize < cbHashSize)
			result = FALSE;
	}

	if (result)
		result = CryptGetHashParam(this->_hMd5Hash, HP_HASHVAL, hash, &cbHashSize, 0);

	return size;
}