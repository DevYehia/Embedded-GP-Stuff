/***************************************************************************************
 * Module Name  : Bootloader Decompress                                                *
 * File Name    : bl_decompress_pbcfg                                                  *
 * Author       : Ouda                                                                 *
 * Purpose      : This module is a wrapper layer for decompression of incoming updates *
 ***************************************************************************************/
#include "stdint.h"
/***************************************************************************************
 *                                  -=[Configurations]=-                               *
 ***************************************************************************************/

/***************************************************************************************
 * Global Variable Name : g_BL_Decompress_UpdateBlockSize                              *
 * Description          : Size of uncompressed update blocks in bytes (before          *
 *                        compression) that will be received for processing            *
 ***************************************************************************************/
uint32_t g_BL_Decompress_UpdateBlockSize=4096;
