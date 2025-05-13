/***************************************************************************************
 * Module Name  : Bootloader Flash CRC Configuration                                   *
 * File Name    : BootloaderFlashCRCConfig.c                                           *
 * Author       : Ouda                                                                 *
 * Purpose      : This module provides CRC configuration for bootloader flash          *
 *                operations.                                                          *
 ***************************************************************************************/

/****************************************************************************************
 *                                    -=[Includes]=-                                    *
 ****************************************************************************************/
#include <stdint.h>
#include "crc_driver.h"

/****************************************************************************************
 *                                  -=[Global Variables]=-                              *
 ****************************************************************************************/

/****************************************************************
 * Global Variable Name : g_BootloaderFLashCRC_InitConfig       *
 *--------------------------------------------------------------*
 * Description          : CRC module configuration structure    *
 *                        for bootloader flash operations.      *
 *--------------------------------------------------------------*
 * Fields Description   : - polynomial       : CRC polynomial   *
 *                                             (32-bit format). *
 *                        - writeTranspose   : Bit transpose    *
 *                                             during writes.   *
 *                        - complementChecksum: Disable checksum*
 *                                              complement.     *
 *                        - readTranspose    : Disable bit      *
 *                                            transpose on read.*
 *                        - seed             : Initial CRC seed *
 *                                             value.           *
 ****************************************************************/
const crc_user_config_t g_BootloaderFLashCRC_InitConfig = {
    .polynomial = CRC_BITS_32,           /* 32-bit CRC polynomial */
    .writeTranspose = CRC_TRANSPOSE_BITS,/* Transpose bits during write */
    .complementChecksum = false,         /* No checksum complement */
    .readTranspose = false,              /* No read transpose */
    .seed = 0xFFFFFFFFU                  /* Initial seed value */
};