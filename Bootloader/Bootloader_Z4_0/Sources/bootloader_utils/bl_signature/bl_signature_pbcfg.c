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
	0x64, 0x13, 0xe3, 0x70, 0x31, 0x8a, 0x92, 0x2c,  /* X-coordinate first 8 bytes  */
	0xec, 0xfa, 0xa9, 0x4b, 0xa2, 0x18, 0x8d, 0xd4,  /* X-coordinate next 8 bytes   */
	0x19, 0xf5, 0x86, 0x35, 0x6f, 0xa7, 0x74, 0xc7,  /* X-coordinate final 8 bytes  */
	0x66, 0xcd, 0x6c, 0x45, 0x02, 0x95, 0xfe, 0xe9,  /* Y-coordinate first 8 bytes  */
	0x5d, 0xce, 0x9c, 0xe0, 0x55, 0x7b, 0x0a, 0x8f,  /* Y-coordinate next 8 bytes   */
	0x1c, 0xef, 0x5c, 0x66, 0x3f, 0x36, 0x2c, 0xff,  /* Y-coordinate next 8 bytes   */
	0xfc, 0x91, 0x0e, 0x30, 0x94, 0xaf, 0xc8, 0x2b,  /* Y-coordinate final 8 bytes  */
	0xbb, 0xc7, 0xa0, 0xa9, 0x2b, 0x0b, 0x6b, 0xdb   /* Reserved for future use     */
};
