//#include "Bootloader.h"
////#include "flash.h"
//#include "UDS.h"
//#include <string.h>
//
//// /*---------------------------------------------------------------------------
////   Helper Macros & Definitions
//// ---------------------------------------------------------------------------*/
//
///*---------------------------------------------------------------------------
//  Bootloader Initialization
//---------------------------------------------------------------------------*/
//void Bootloader_Init(BootloaderContext *ctx) {
//    ctx->state = BL_WAIT_REQUEST_DOWNLOAD;
//    ctx->flash_address = 0;
//    ctx->expected_size = 0;
//    ctx->programmed_size = 0;
//    ctx->max_block_length = 0;
//    ctx->expected_seq_num = 1;
//}
//
///*---------------------------------------------------------------------------
//  Bootloader ProcessUDSMessage
// This function is called whenever a UDS message is received
// (via the UDS/CanTP layer). It retrieves the UDS service
// identifier and then uses a state machine to process the
// bootloader commands. Depending on the command, it accesses the
// UDS-provided data structures directly and calls the bootloader
// flash APIs to perform the requested operations.
//---------------------------------------------------------------------------*/
//void Bootloader_ProcessUDSMessage(BootloaderContext *ctx) {
//    uds_sid_t sid;
//
//while (1)
//{
//    // Read the next UDS service identifier from the UDS layer.
//    sid = uds_get_sid();
//    switch (sid) {
//        case UDS_SID_REQUEST_DOWNLOAD: {
//            if (ctx->state == BL_WAIT_REQUEST_DOWNLOAD) {
//
//                //  Directly access the UDS download request structure variables.
//                ctx->flash_address = uds_download_req_data.mem_start_address;
//                ctx->expected_size   = uds_download_req_data.total_size;
//                ctx->max_block_length = uds_download_req_data.MaxNumberBlockLength;
//
//                    // // Validate the address
//                    // if ( /* perform address validity check */ true ) {
//                        ctx->state = BL_WAIT_ERASE_MEMORY;
//                        uds_send_response(true, UDS_SID_REQUEST_DOWNLOAD);
//                    // } else {
//                    //     ctx->state = STATE_ERROR;
//                    // }
//                }
//                break;
//            }
//
//        case UDS_SID_ERASE_MEMORY: {
//                if (ctx->state == BL_WAIT_ERASE_MEMORY) {
//                    // Erase the memory block starting at the requested flash address.
//
//                   status_t ret = BootloaderFlash_Erase(ctx->flash_address, ctx->expected_size);
//                    if (STATUS_SUCCESS == ret)
//                    {
//                        ctx->state = BL_RECEIVE_DATA;
//                        uds_send_response(true, UDS_SID_ERASE_MEMORY);
//                    }
//                    } else {
//                        ctx->state = BL_ERROR;
//                    }
//                }
//                break;
//            }
//
//            //BootloaderFlash_Program(uint32_t a_dest, uint32_t a_size, uint32_t a_source)
//
//            case UDS_SID_TRANSFER_DATA: {
//                if (ctx->state == BL_RECEIVE_DATA ) {
//                    // Directly access the UDS transfer data structure variables.
//                    Transferred_Data* tdata = &uds_transfer_data;
//
//                    // Verify the sequence number.
//                    if (tdata->seq_number != ctx->expected_seq_num) {
//                        ctx->state = STATE_ERROR;
//                        break;
//                    }
//                    // Program the data into flash.
//                    if (bootloaderflash_program(ctx->flash_address + ctx->programmed_size , ctx->data_size, tdata->data)) {
//                        ctx->programmed_size += ctx->data_size;
//                        ctx->expected_seq_num++; // Expect next sequence number
//                        uds_send_response(true, UDS_SID_TRANSFER_DATA);
//                    } else {
//                        ctx->state = BL_ERROR;
//                    }
//                }
//                break;
//            }
//
//        case UDS_SID_TRANSFER_EXIT: {
//                if (ctx->state == BL_RECEIVE_DATA) {
//                    // End of transfer. Verify that the total programmed size matches what was requested.
//                    if (ctx->programmed_size == ctx->expected_size) {
//                        uds_send_response(true, UDS_SID_TRANSFER_EXIT);
//                        ctx->state = BL_COMPLETE;  // Bootloading completed successfully.
//                    } else {
//                        ctx->state = BL_ERROR;
//                    }
//                }
//                break;
//            }
//
//        case BL_COMPLETE:
//            // Bootloader update finished successfully.
//            // The bootloader can now trigger a jump to the new application,
//            // or simply report success.
//            break;
//
//        case BL_ERROR:
//            // In an error state, additional messages may be ignored,
//            // or the system may wait for a reset.
//            break;
//
//        default:
//            break;
//    }
//}
//}
