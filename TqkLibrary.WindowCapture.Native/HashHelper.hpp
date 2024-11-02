#ifndef _WindowCapture_H_HashHelper_H_
#define _WindowCapture_H_HashHelper_H_

#include "WinApi.hpp"
//#define HashHelper_Enable
#define HashHelper_HashSize 8

class HashHelper
{
public:
	HashHelper();
	~HashHelper();

	INT32 CalcHash(const BYTE * const data, const UINT32 dataSize, BYTE* hash, INT32 hashSize);

private:
};

#endif