#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "CryptoApi.h"
#include "test.h"
#include "main.h"

void TestSha(void)
{
    CPRI_HASH_STATE hashState = {0};
    std::vector<UINT8> hash;
    char* hashdata = "abc";

    try
    {
        hash.resize(_cpri__StartHash(TPM_ALG_SHA256, false, &hashState));
        MX_ASSERT(hash.size(), 32, "Hash size wrong!");
        _cpri__UpdateHash(&hashState, sizeof(hashdata) - 1, (BYTE*)hashdata);
        _cpri__CompleteHash(&hashState, hash.size(), &hash[0]);
    }
    MX_WRAP(0, "Hashtest failed!");
}


void testme(void)
{
    MX_ENTRY(TestSha());
}
