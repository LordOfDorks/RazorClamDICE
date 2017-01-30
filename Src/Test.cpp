#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "UrchinLib.h"
#include "UrchinPlatform.h"
#include "test.h"
#include "main.h"

ANY_OBJECT ekObject = {0};
SESSION ekSeededSession = {0};
static UINT32
ReadEkObjectUntrusted(
    void
    )
{
    DEFINE_CALL_BUFFERS;
    UINT32 result = TPM_RC_SUCCESS;
    union
    {
        ReadPublic_In readPublic;
    } in;
    union
    {
        ReadPublic_Out readPublic;
    } out;
    TPM2B_PUBLIC publicArea = {0};
    TPM2B_DIGEST name = {0};

    // Read the EK public - This data is untrusted until we verify this against the EKName stored in the MCU
    INITIALIZE_CALL_BUFFERS(TPM2_ReadPublic, &in.readPublic, &out.readPublic);
    parms.objectTableIn[TPM2_ReadPublic_HdlIn_PublicKey].generic.handle = TPM_20_EK_HANDLE;
    EXECUTE_TPM_CALL(FALSE, TPM2_ReadPublic);
    ekObject = parms.objectTableIn[0];

    // Make sure the key properties are correct
    SetEkTemplate(&publicArea);
    if((!Memory2BEqual((TPM2B*)&publicArea.t.publicArea.authPolicy, (TPM2B*)&ekObject.obj.publicArea.t.publicArea.authPolicy)) ||
       (publicArea.t.publicArea.nameAlg != ekObject.obj.publicArea.t.publicArea.nameAlg) ||
       (*((UINT32*)&publicArea.t.publicArea.objectAttributes) != *((UINT32*)&ekObject.obj.publicArea.t.publicArea.objectAttributes)) ||
       (publicArea.t.publicArea.parameters.rsaDetail.exponent != ekObject.obj.publicArea.t.publicArea.parameters.rsaDetail.exponent) ||
       (publicArea.t.publicArea.parameters.rsaDetail.keyBits != ekObject.obj.publicArea.t.publicArea.parameters.rsaDetail.keyBits) ||
       (publicArea.t.publicArea.parameters.rsaDetail.scheme.scheme != ekObject.obj.publicArea.t.publicArea.parameters.rsaDetail.scheme.scheme) ||
       (publicArea.t.publicArea.parameters.rsaDetail.symmetric.algorithm != ekObject.obj.publicArea.t.publicArea.parameters.rsaDetail.symmetric.algorithm) ||
       (publicArea.t.publicArea.parameters.rsaDetail.symmetric.keyBits.aes != ekObject.obj.publicArea.t.publicArea.parameters.rsaDetail.symmetric.keyBits.aes) ||
       (publicArea.t.publicArea.parameters.rsaDetail.symmetric.mode.aes != ekObject.obj.publicArea.t.publicArea.parameters.rsaDetail.symmetric.mode.aes) ||
       (publicArea.t.publicArea.unique.rsa.t.size != ekObject.obj.publicArea.t.publicArea.unique.rsa.t.size) ||
       (publicArea.t.publicArea.type != ekObject.obj.publicArea.t.publicArea.type))
    {
        result = TPM_RC_FAILURE;
        goto Cleanup;
    }

    // Make sure the EKName the TPM sent matches the public portion of the key
    buffer = pbCmd;
    size = sizeof(pbCmd);
    UINT16_TO_BYTE_ARRAY(ekObject.obj.publicArea.t.publicArea.nameAlg, name.t.buffer);
    name.t.size = sizeof(TPM_ALG_ID);
    if((cbCmd = TPMT_PUBLIC_Marshal(&ekObject.obj.publicArea.t.publicArea, &buffer, &size)) <= 0)
    {
        result = TPM_RC_FAILURE;
        goto Cleanup;
    }
    name.t.size += CryptHashBlock(ekObject.obj.publicArea.t.publicArea.nameAlg, cbCmd, pbCmd, sizeof(name.t.buffer) - name.t.size, &name.t.buffer[name.t.size]);
    if(Memory2BEqual((TPM2B*)&name, (TPM2B*)&ekObject.obj.name) == FALSE)
    {
        result = TPM_RC_FAILURE;
        goto Cleanup;
    }

Cleanup:
    return result;
}

UINT32
StartEkSeededSession(
        void
        )
{
    DEFINE_CALL_BUFFERS;
    UINT32 result = TPM_RC_SUCCESS;
    union
    {
        StartAuthSession_In startAuthSession;
    } in;
    union
    {
        StartAuthSession_Out startAuthSession;
    } out;

#ifdef NTZTPM
    for(UINT32 n = 0; n < 10; n++)
    {
#endif
        // Start EK salted session
        INITIALIZE_CALL_BUFFERS(TPM2_StartAuthSession, &in.startAuthSession, &out.startAuthSession);
        parms.objectTableIn[TPM2_StartAuthSession_HdlIn_TpmKey] = ekObject;  // Encrypt salt to EK
        parms.objectTableIn[TPM2_StartAuthSession_HdlIn_Bind].obj.handle = TPM_RH_NULL;
        in.startAuthSession.nonceCaller.t.size = CryptGenerateRandom(SHA256_DIGEST_SIZE, in.startAuthSession.nonceCaller.t.buffer);
        in.startAuthSession.sessionType = TPM_SE_HMAC;
        in.startAuthSession.symmetric.algorithm = TPM_ALG_AES;
        in.startAuthSession.symmetric.keyBits.aes = 128;
        in.startAuthSession.symmetric.mode.aes = TPM_ALG_CFB;
        in.startAuthSession.authHash = TPM_ALG_SHA256;
#ifndef NTZTPM
        EXECUTE_TPM_CALL(FALSE, TPM2_StartAuthSession);
#else
        TRY_TPM_CALL(FALSE, TPM2_StartAuthSession);
        if(result == TPM_RC_SUCCESS)
        {
            break;
        }
        else
        {
            // The NTZ TPm has some issues here. Give the TPM some time to get the RSA engine ready
            HAL_Delay(250);
        }
    }
#endif
    if(result != TPM_RC_SUCCESS)
    {
        goto Cleanup;
    }
    // Copy the session out
    ekSeededSession = parms.objectTableOut[TPM2_StartAuthSession_HdlOut_SessionHandle].session;

Cleanup:
    return result;
}

static UINT32
ReSeedRng(
    void
    )
{
    DEFINE_CALL_BUFFERS;
    UINT32 result = TPM_RC_SUCCESS;
    union
    {
        GetRandom_In getRandom;
    } in;
    union
    {
        GetRandom_Out getRandom;
    } out;

    // Read some entropy from the TPM in a way that we are sure that it really
    // came from the TPM and nobody is feeding us stale random numbers
    sessionTable[0] = ekSeededSession;
    sessionTable[0].attributes.audit = SET;
    INITIALIZE_CALL_BUFFERS(TPM2_GetRandom, &in.getRandom, &out.getRandom);
    sessionCnt += 1; // Add the EK session for auditing
    in.getRandom.bytesRequested = SHA256_DIGEST_SIZE;
    EXECUTE_TPM_CALL(FALSE, TPM2_GetRandom);
    sessionTable[0].attributes = ekSeededSession.attributes;
    ekSeededSession = sessionTable[0];

    // Re-seed the internal RNG with the entropy
    CryptStirRandom(out.getRandom.randomBytes.t.size, out.getRandom.randomBytes.t.buffer);

Cleanup:
    if(result != TPM_RC_SUCCESS)
    {
        // Copy the EKSeeded session back out in case of an error
        sessionTable[0].attributes = ekSeededSession.attributes;
        ekSeededSession = sessionTable[0];
    }
    return result;
}

void TestUrchin()
{
    try
    {
        MX_CALL(ReadEkObjectUntrusted(), TPM_RC_SUCCESS, "Reading EK");
        MX_CALL(StartEkSeededSession(), TPM_RC_SUCCESS, "Starting seeded session");
        MX_CALL(ReSeedRng(), TPM_RC_SUCCESS, "Get Random");
    }
    MX_WRAP(0, "Hashtest failed!");
}

void RunTests(void)
{
    TestUrchin();
}

void testme(void)
{
    MX_ENTRY(RunTests());
}
