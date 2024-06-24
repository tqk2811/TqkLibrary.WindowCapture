#ifndef _WindowCapture_H_Md5_H_
#define _WindowCapture_H_Md5_H_

#include "WinApi.hpp"
#include <wincrypt.h>
#define Md5HashSize 16

class Md5Helper
{
public:
	Md5Helper();
	~Md5Helper();

	INT32 CalcHash(const BYTE const* data, const UINT32 dataSize, BYTE* hash, INT32 hashSize);

private:
	HCRYPTPROV _hProv{ NULL };
	HCRYPTPROV _hMd5Hash{ NULL };
};

#endif