//#ifndef BOOTLOADER_H
//#define BOOTLOADER_H
//
//#include <stdint.h>
//#include <stdbool.h>
////#include <zlib.h>      // for crc32()
//
//
//// Define UDS Service Identifiers (SIDs)
//typedef enum {
//    UDS_SID_REQUEST_DOWNLOAD = 0x34,
//    UDS_SID_ERASE_MEMORY      = 0x31,
//    UDS_SID_TRANSFER_DATA     = 0x36,
//    UDS_SID_TRANSFER_EXIT     = 0x37
//} uds_sid_t;
//
//
//// Global instances defined in UDS.c
//extern BL_Req_Download_Data uds_download_req_data;
//extern Transferred_Data uds_transfer_data;
//
//// UDS interface functions (to be provided by UDS.c)
//// These functions are expected to read the SID and corresponding data from the UDS layer.
//uds_sid_t uds_get_sid(void);
//void uds_send_response(bool positive, uds_sid_t sid);  // Sends a positive/negative response for the given SID
//
//
//// ---------------------------------------------------------------------------
//// Bootloader State Machine
//// ---------------------------------------------------------------------------
//typedef enum {
//    BL_COMPLETE,
//    BL_WAIT_REQUEST_DOWNLOAD,
//    BL_WAIT_ERASE_MEMORY,
//    BL_RECEIVE_DATA,
//    BL_VERIFY_AND_FINALIZE,
//    BL_ERROR
//} BootloaderState;
//
///* Bootloader Context holding parameters and runtime state */
//
//typedef struct {
//    BootloaderState state;
//    uint32_t flash_address;
//    uint32_t expected_size;
//    uint32_t programmed_size;
//    uint16_t max_block_length;
//    uint8_t expected_seq_num;
//} BootloaderContext;
//
//
///* Bootloader API functions */
//void Bootloader_Init(BootloaderContext *ctx);
//void Bootloader_ProcessUDSMessage(BootloaderContext *ctx);
//
//#endif /* BOOTLOADER_H */
