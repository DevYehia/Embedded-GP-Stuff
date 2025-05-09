/***************************************************************************************
 * Module Name  : Bootloader Decompress                                                *
 * File Name    : bl_decompress.h                                                      *
 * Author       : Ouda                                                                 *
 * Purpose      : This module is a wrapper layer for decompression of incoming updates *
 ***************************************************************************************/

/***************************************************************************************
 *                                   -=[Includes]=-                                    *
 ***************************************************************************************/
#include "../status.h"
#include "lz4.h"

/***************************************************************************************
 *                                   -=[Definitions]=-                                 *
 ***************************************************************************************/

/***************************************************************************************
 *                                    -=[Externs]=-                                    *
 ***************************************************************************************/
extern uint32_t g_BL_Decompress_UpdateBlockSize;
/***************************************************************************************
 *                                 -=[Function IDs]=-                                  *
 ***************************************************************************************/
#define BLDECOMP_DECOMP                 (1U)

/***************************************************************************************
 *                               -=[Function Prototypes]=-                             *
 ***************************************************************************************/

/***************************************************************************************
 * Function Name: BLDecomp_Decompress                                                  *
 * Inputs       : pa_u8source       - Pointer to compressed source data                *
 *                pa_u8dest         - Pointer to destination buffer                    *
 *                a_u32compressedSize - Size of compressed data in bytes               *
 * Outputs      : uint32_t          - Number of bytes decompressed                     *
 * Reentrancy   : Non-Reentrant                                                        *
 * Synchronous  : Synchronous                                                          *
 * Description  : Decompresses received update data using LZ4 algorithm.               *
 ***************************************************************************************/
uint32_t BLDecomp_Decompress(const uint8_t* pa_u8source,
                             uint8_t* pa_u8dest,
                             uint32_t a_u32compressedSize);
