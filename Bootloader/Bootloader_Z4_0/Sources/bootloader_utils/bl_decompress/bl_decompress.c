/***************************************************************************************
 * Module Name  : Bootloader Decompress                                                *
 * File Name    : bl_decompress.c                                                      *
 * Author       : Ouda                                                                 *
 * Purpose      : This module is a wrapper layer for decompression of incoming updates *
 ***************************************************************************************/

/****************************************************************************************
 *                                    -=[Includes]=-                                    *
 ****************************************************************************************/
#include "bl_decompress.h"
/***************************************************************************************
 *                                  -=[Functions]=-                                    *
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
uint32_t BLDecomp_Decompress(const uint8_t *pa_u8source, uint8_t *pa_u8dest, uint32_t a_u32compressedSize)
{
    uint32_t decomp_result = LZ4_decompress_safe(
        (const char*)pa_u8source,
        (char*)pa_u8dest,
        a_u32compressedSize,
        g_BL_Decompress_UpdateBlockSize);

    return decomp_result;
}
