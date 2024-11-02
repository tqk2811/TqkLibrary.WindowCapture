#include "HashHelper.hpp"
#include <xxh3.h>

HashHelper::HashHelper()
{
}
HashHelper::~HashHelper()
{
}

INT32 HashHelper::CalcHash(const BYTE * const data, const UINT32 dataSize, BYTE* hash, INT32 hashSize)
{
	INT32 size = -1;
	if (hashSize >= HashHelper_HashSize)
	{
		XXH32_hash_t seed = 0;
		XXH64_hash_t xxh64_hash = XXH64(data, dataSize, seed);
		memcpy((void*)hash, &xxh64_hash, HashHelper_HashSize);
		size = HashHelper_HashSize;
	}
	return size;
}