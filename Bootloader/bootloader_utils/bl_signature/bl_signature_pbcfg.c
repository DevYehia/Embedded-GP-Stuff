/***************************************************************************************
 * Module Name  : Bootloader Signature                                                 *
 * File Name    : bl_signature_pbcfg.c                                                 *
 * Author       : Ouda                                                                 *
 * Purpose      : This module is a wrapper layer for signature verification of incoming*
 *                updates                                                              *
 ***************************************************************************************/

/***************************************************************************************
 *                                   -=[Includes]=-                                    *
 ***************************************************************************************/
#include "stdint.h"

/***************************************************************************************
 *                                -=[Configurations]=-                                 *
 ***************************************************************************************/

/***************************************************************************************
 * Global Variable Name : g_BLSig_PublicKey                                            *
 * Description          : ECDSA public key for signature verification in raw format    *
 *                        (64-byte uncompressed SECP256R1 public key)                  *
 * Format               : [32-byte X-coordinate][32-byte Y-coordinate]                 *
 ***************************************************************************************/
const uint8_t g_BLSig_PublicKey[64] = {
    0xe2, 0x5b, 0xa8, 0xc0, 0xda, 0x10, 0x1b, 0x12,  /* X-coordinate first 8 bytes  */
    0xa3, 0x01, 0x5f, 0xad, 0x41, 0x84, 0xbb, 0xf6,  /* X-coordinate next 8 bytes   */
    0xbc, 0xc4, 0x8a, 0x6c, 0xc5, 0xd5, 0x48, 0xa9,  /* X-coordinate final 8 bytes */
    0xb2, 0xac, 0x86, 0x11, 0x96, 0x9f, 0x2f, 0xb9,  /* Y-coordinate first 8 bytes  */
    0xea, 0x4d, 0xb8, 0xc7, 0x7a, 0xb4, 0x17, 0x5e,  /* Y-coordinate next 8 bytes   */
    0xcc, 0xdd, 0xba, 0xd6, 0x9f, 0x49, 0x22, 0xb3,  /* Y-coordinate next 8 bytes   */
    0x30, 0x47, 0xef, 0xf1, 0xfe, 0x7b, 0x2e, 0x3a,  /* Y-coordinate final 8 bytes */
    0x0c, 0x44, 0x7b, 0xe3, 0xe1, 0xf6, 0x21, 0x83   /* Reserved for future use      */
};
