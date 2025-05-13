/***************************************************************************************
 * Module Name  : Bootloader Signature                                                 *
 * File Name    : bl_signature.h                                                       *
 * Author       : Ouda                                                                 *
 * Purpose      : This module is a wrapper layer for signature verification of incoming*
 *                updates                                                              *
 ***************************************************************************************/

/***************************************************************************************
 *                                   -=[Includes]=-                                     *
 ***************************************************************************************/
#include "../status.h"
#include "bl_hash/sha256.h"
#include "bl_ecdsa/uECC.h"

/***************************************************************************************
 *                                   -=[Definitions]=-                                  *
 ***************************************************************************************/

/***************************************************************************************
 *                                 -=[Function IDs]=-                                   *
 ***************************************************************************************/
#define BLSIG_INIT_HASH                     (1U)
#define BLSIG_UPDATE_HASH                   (2U)
#define BLSIG_FINALIZE_HASH                 (3U)
#define BLSIG_VERIFY_SIGNATURE              (4U)

/***************************************************************************************
 *                               -=[Function Prototypes]=-                              *
 ***************************************************************************************/

/***************************************************************************************
 * Function Name: BLSig_InitHash                                                        *
 * Inputs       : None                                                                  *
 * Outputs      : status_t - Initialization status                                      *
 * Reentrancy   : Non-Reentrant                                                         *
 * Synchronous  : Synchronous                                                           *
 * Description  : Initializes hash computation context for signature verification.      *
 ***************************************************************************************/
status_t BLSig_InitHash(void);

/***************************************************************************************
 * Function Name: BLSig_UpdateHash                                                      *
 * Inputs       : pa_u8source - Pointer to data stream for hash computation             *
 *                a_u32size   - Size of input data in bytes                             *
 * Outputs      : status_t    - Hash update status                                      *
 * Reentrancy   : Non-Reentrant                                                         *
 * Synchronous  : Synchronous                                                           *
 * Description  : Updates hash computation with new data chunk.                        *
 ***************************************************************************************/
status_t BLSig_UpdateHash(const uint8_t *pa_u8source, uint32_t a_u32size);

/***************************************************************************************
 * Function Name: BLSig_FinalizeHash                                                    *
 * Inputs       : pa_u8resultHash - Pointer to buffer for final hash value              *
 * Outputs      : status_t        - Hash finalization status                            *
 * Reentrancy   : Non-Reentrant                                                         *
 * Synchronous  : Synchronous                                                           *
 * Description  : Completes hash computation and returns the final digest.              *
 ***************************************************************************************/
status_t BLSig_FinalizeHash(uint8_t *pa_u8resultHash);

/***************************************************************************************
 * Function Name: BLSig_VerifySignature                                                 *
 * Inputs       : pa_u8msgHash    - Pointer to message hash to verify                   *
 *                a_u32sizeHash   - Size of message hash in bytes                       *
 *                pa_u8signature  - Pointer to signature data                           *
 * Outputs      : status_t        - Signature verification result                       *
 * Reentrancy   : Non-Reentrant                                                         *
 * Synchronous  : Synchronous                                                           *
 * Description  : Verifies ECDSA signature against computed message hash.               *
 ***************************************************************************************/
status_t BLSig_VerifySignature(const uint8_t *pa_u8msgHash,
                               uint32_t a_u32sizeHash,
                               const uint8_t *pa_u8signature);
