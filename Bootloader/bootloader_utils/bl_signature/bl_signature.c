/***************************************************************************************
 * Module Name  : Bootloader Signature                                                 *
 * File Name    : bl_signature.h                                                       *
 * Author       : Ouda                                                                 *
 * Purpose      : This module is a wrapper layer for signature verification of incoming*
 *                updates                                                              *
 ***************************************************************************************/

/****************************************************************************************
 *                                  -=[Includes]=-                                      *
 ****************************************************************************************/
#include "bl_signature.h"

/**************************************************************************************
 *                              -=[Global Variables]=-                                *
 **************************************************************************************/

/***************************************************************************************
 *                                -=[Global Variables]=-                               *
 ***************************************************************************************/

/***************************************************************************************
 * Global Variable Name : g_BLSig_PublicKey [EXTERN]                                   *
 * Reentrancy           : Reentrant                                                    *
 * Description          : ECDSA public key for signature verification in raw format    *
 *                        (64-byte uncompressed SECP256R1 public key)                  *
 * Format               : [32-byte X-coordinate][32-byte Y-coordinate]                 *
 ***************************************************************************************/
extern const uint8_t g_BLSig_PublicKey[64];

/***************************************************************************************
 * Global Variable Name : g_BLSig_sha256Ctx [Static]                                   *
 * Reentrancy           : Non-Reentrant                                                *
 * Description          : SHA-256 context for maintaining hash computation state       *
 *                        during incremental hashing operations                        *
 ***************************************************************************************/
static SHA256_CTX g_BLSig_sha256Ctx;

/***************************************************************************************
 * Global Variable Name : pg_BLSig_ecdsacCurve [Static]                                *
 * Reentrancy           : Reentrant                                                    *
 * Description          : Pointer to ECC curve parameters used for ECDSA verification  *
 * Initialization       : Set to secp256r1 curve during startup initialization         *
 ***************************************************************************************/
static struct uECC_Curve_t *pg_BLSig_ecdsaCurve = NULL;

/***************************************************************************************
 *                                  -=[Functions]=-                                    *
 ***************************************************************************************/

/***************************************************************************************
 * Function Name: BLSig_InitHash                                                       *
 * Inputs       : None                                                                 *
 * Outputs      : status_t - Initialization status                                     *
 * Reentrancy   : Non-Reentrant                                                        *
 * Synchronous  : Synchronous                                                          *
 * Description  : Initializes hash computation context for signature verification.     *
 ***************************************************************************************/
status_t BLSig_InitHash(void)
{
    status_t returnCode = STATUS_SUCCESS;
    sha256_init(&g_BLSig_sha256Ctx);
    return returnCode;
}

/***************************************************************************************
 * Function Name: BLSig_UpdateHash                                                     *
 * Inputs       : pa_u8source - Pointer to data stream for hash computation            *
 *                a_u32size   - Size of input data in bytes                            *
 * Outputs      : status_t    - Hash update status                                     *
 * Reentrancy   : Non-Reentrant                                                        *
 * Synchronous  : Synchronous                                                          *
 * Description  : Updates hash computation with new data chunk.                        *
 ***************************************************************************************/
status_t BLSig_UpdateHash(const uint8_t *pa_u8source, uint32_t a_u32size)
{
    status_t returnCode = STATUS_SUCCESS;
    uint32_t msglen=a_u32size-1;
    if (pa_u8source == NULL)
    {
        returnCode = STATUS_ERROR;
    }
    else
    {
        sha256_update(&g_BLSig_sha256Ctx, pa_u8source, msglen);
    }
    return returnCode;
}

/***************************************************************************************
 * Function Name: BLSig_FinalizeHash                                                   *
 * Inputs       : pa_u8resultHash - Pointer to buffer for final hash value             *
 * Outputs      : status_t        - Hash finalization status                           *
 * Reentrancy   : Non-Reentrant                                                        *
 * Synchronous  : Synchronous                                                          *
 * Description  : Completes hash computation and returns the final digest.             *
 ***************************************************************************************/
status_t BLSig_FinalizeHash(uint8_t *pa_u8resultHash)
{
    status_t returnCode = STATUS_SUCCESS;
    if (pa_u8resultHash == NULL)
    {
        returnCode = STATUS_ERROR;
    }
    else
    {
        sha256_final(&g_BLSig_sha256Ctx, pa_u8resultHash);
    }
    return returnCode;
}

/***************************************************************************************
 * Function Name: BLSig_VerifySignature                                                *
 * Inputs       : pa_u8msgHash    - Pointer to message hash to verify                  *
 *                a_u32sizeHash   - Size of message hash in bytes                      *
 *                pa_u8signature  - Pointer to signature data                          *
 * Outputs      : status_t        - Signature verification result                      *
 * Reentrancy   : Non-Reentrant                                                        *
 * Synchronous  : Synchronous                                                          *
 * Description  : Verifies ECDSA signature against computed message hash.              *
 ***************************************************************************************/
status_t BLSig_VerifySignature(const uint8_t *pa_u8msgHash, uint32_t a_u32sizeHash, const uint8_t *pa_u8signature)
{
	pg_BLSig_ecdsaCurve = uECC_secp256r1();
    status_t returnCode = STATUS_SUCCESS;
    if((pa_u8msgHash == NULL)||
       (pa_u8signature == NULL)||
	   (pg_BLSig_ecdsaCurve == NULL)){
        returnCode = STATUS_ERROR;
    }
    else{

        uint8_t valid = uECC_verify(g_BLSig_PublicKey, pa_u8msgHash, a_u32sizeHash,pa_u8signature, pg_BLSig_ecdsaCurve);
        if(valid){
        	returnCode = STATUS_SUCCESS;
        }
        else{
        	returnCode = STATUS_ERROR;
        }
    }
    return returnCode;
}
