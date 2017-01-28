#include <inttypes.h>
#include <vector>
#include "CryptoApi.h"
#include "test.h"

void testme(void)
{
    UINT16 result = 0;
    CPRI_HASH_STATE hashState = {0};
    std::vector<UINT8> hash(32);
    result = _cpri__StartHash(TPM_ALG_SHA256, false, &hashState);
    _cpri__UpdateHash(&hashState, sizeof(result), (BYTE*)&result);
    result = _cpri__CompleteHash(&hashState, hash.size(), &hash[0]);
}
