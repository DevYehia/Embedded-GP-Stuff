/*
 * BootloaderFlashCRCTable.c
 *
 *  Created on: Feb 4, 2025
 *      Author: Ouda
 */
#include <stdint.h>

#include "crc_driver.h"

/*! @brief Configuration structure crc1_InitConfig0 */
const crc_user_config_t g_BootloaderFLashCRC_InitConfig = {
    .polynomial = CRC_BITS_32,
    .writeTranspose = CRC_TRANSPOSE_BITS,
    .complementChecksum = false,
    .readTranspose = false,
    .seed = 0xFFFFFFFFU
};
