#ifndef _WindowCapture_H_HashHelper_H_
#define _WindowCapture_H_HashHelper_H_

#include "WinApi.hpp"
#include <wincrypt.h>
//#define HashHelper_HashSize 16

class HashHelper
{
public:
	HashHelper();
	~HashHelper();

	INT32 CalcHash(const BYTE const* data, const UINT32 dataSize, BYTE* hash, INT32 hashSize);

private:
	HCRYPTPROV _hProv{ NULL };
	HCRYPTPROV _hMd5Hash{ NULL };
};

#endif