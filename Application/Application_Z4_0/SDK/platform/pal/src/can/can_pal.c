/*
 * Copyright 2017-2019 NXP
 * All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 * @file can_pal.c
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 1.3, Taking address of near auto variable.
 * The code is not dynamically linked. An absolute stack address is obtained
 * when taking the address of the near auto variable. A source of error in
 * writing dynamic code is that the stack segment may be different from the data
 * segment.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, External could be made static.
 * Function is defined for usage by application code.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.9, An object should be defined at block
 * scope if its identifier only appears in a single function.
 * An object with static storage duration declared at block scope cannot be
 * accessed directly from outside the block.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.3, Expression assigned to a narrower or
 * different essential type
 * This is needed for the conversion between generic CAN types to FlexCAN types.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 10.5, Impermissible cast; cannot cast
 * from 'essentially enum<i>' to 'essentially enum<i>'
 * This is needed for the conversion between generic CAN types to FlexCAN types.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.3, Cast performed between a pointer to
 * object type and a pointer to a different object type. This is needed for the
 * extension of the user configuration structure, for which the actual type
 * cannot be known.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.5, Conversion from pointer to void to
 * pointer to other type. This is needed for the extension of the user
 * configuration structure, for which the actual type cannot be known.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.8, attempt to cast away const/volatile
 * from a pointer or reference
 * This is needed for the conversion between generic CAN types to FlexCAN types.
 */

#include "can_pal.h"
#include "device_registers.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if (defined(CAN_OVER_FLEXCAN))

/* Internal FlexCAN Rx FIFO state information */
typedef struct
{
    bool rxFifoEn;
    flexcan_rx_fifo_id_filter_num_t numIdFilters;
} flexcan_rx_fifo_state_t;

#endif /* CAN_OVER_FLEXCAN */

#if (defined(CAN_OVER_MCAN))
/* Internal FlexCAN Rx FIFO state information */
typedef struct
{
    bool rxFifoEn;
    uint8_t numIdFilters;
} mcan_rx_fifo_state_t;
#endif    /* CAN_OVER_MCAN */

/*******************************************************************************
 * Variables
 ******************************************************************************/
#if (defined(CAN_OVER_FLEXCAN))

/*! @brief FlexCAN state structures */
static flexcan_state_t s_flexcanState[NO_OF_FLEXCAN_INSTS_FOR_CAN];
/*! @brief FlexCAN state-instance matching */
static uint32_t s_flexcanStateInstanceMapping[NO_OF_FLEXCAN_INSTS_FOR_CAN];
/*! @brief FlexCAN available resources table */
static bool s_flexcanStateIsAllocated[NO_OF_FLEXCAN_INSTS_FOR_CAN];
/*! @brief FlexCAN buffer configs */
static const can_buff_config_t *s_hwObjConfigs[NO_OF_FLEXCAN_INSTS_FOR_CAN][FEATURE_CAN_MAX_MB_NUM];
/*! @brief FlexCAN Rx FIFO state structures */
static flexcan_rx_fifo_state_t s_flexcanRxFifoState[NO_OF_FLEXCAN_INSTS_FOR_CAN];
/*! @brief Callback function provided by user for each instance*/
static can_callback_t userFlexCallbacks[NO_OF_FLEXCAN_INSTS_FOR_CAN];

#endif

#if (defined(CAN_OVER_MCAN))

/*! @brief MCAN state structures */
static mcan_state_t s_mcanState[NO_OF_MCAN_INSTS_FOR_CAN];
/*! @brief MCAN state-instance matching */
static uint32_t s_mcanStateInstanceMapping[NO_OF_MCAN_INSTS_FOR_CAN];
/*! @brief MCAN available resources table */
static bool s_mcanStateIsAllocated[NO_OF_MCAN_INSTS_FOR_CAN];
/*! @brief MCAN buffer configs */
static const can_buff_config_t *s_hwObjConfigs1[NO_OF_MCAN_INSTS_FOR_CAN][FEATURE_MCAN_RX_MB_NUM + FEATURE_MCAN_TX_MB_NUM];
/*! @brief MCAN Rx FIFO state structures */
static mcan_rx_fifo_state_t s_mcanRxFifoState[NO_OF_MCAN_INSTS_FOR_CAN];
/*! @brief Callback function provided by user for each instance*/
static can_callback_t userMcanCallbacks[NO_OF_MCAN_INSTS_FOR_CAN];
/*! @brief MCAN available Rx Mbs*/
static uint32_t s_rxBuffs;
/*! @brief MCAN available Tx Mbs*/
static uint32_t s_txBuffs;

#endif /* CAN_OVER_MCAN */

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_AllocateState
 * Description   : Allocates one of the available state structures.
 *
 *END**************************************************************************/
static uint8_t CAN_AllocateState(bool *isAllocated,
                                 uint32_t *instanceMapping,
                                 uint32_t instance,
                                 uint32_t instanceNo)
{
    uint8_t i;
    /* Allocate one of the CAN state structures for this instance */
    for (i = 0; i < instanceNo; i++)
    {
        if (isAllocated[i] == false)
        {
            instanceMapping[i] = instance;
            isAllocated[i] = true;
            break;
        }
    }

    if (i >= instanceNo)
    {
        /* Should Never Reach Here */
        DEV_ASSERT(false);
        i = (instanceNo-1U);
    }

    return i;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_FreeState
 * Description   : Deallocates one of the available state structures.
 *
 *END**************************************************************************/
static void CAN_FreeState(bool *isAllocated,
                          const uint32_t *instanceMapping,
                          uint32_t instance,
                          uint32_t instanceNo)
{
    uint8_t i;
    /* Free one of the CAN state structures for this instance */
    for (i = 0; i < instanceNo; i++)
    {
        if (instanceMapping[i] == instance)
        {
            isAllocated[i] = false;
            break;
        }
    }
}

#if (defined(CAN_OVER_FLEXCAN))

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_FindFlexCANState
 * Description   : Search the state structure of the FlexCAN instance
 *
 *END**************************************************************************/
static uint8_t CAN_FindFlexCANState(uint32_t instance)
{
    uint8_t i;

    for (i = 0; i < NO_OF_FLEXCAN_INSTS_FOR_CAN; i++)
    {
        if (s_flexcanStateInstanceMapping[i] == instance)
        {
            break;
        }
    }

    if (i >= NO_OF_FLEXCAN_INSTS_FOR_CAN)
    {
        /* Should Never Reach Here */
        DEV_ASSERT(false);
        i = (NO_OF_FLEXCAN_INSTS_FOR_CAN-1U);
    }

    return i;

}

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_GetVirtualBuffIdx
 * Description   : Determines the index of the last buffer occupied by Rx
 *                 FIFO filters
 *
 *END**************************************************************************/
static inline uint32_t CAN_GetVirtualBuffIdx(uint32_t x)
{
    return (5U + ((((x) + 1U) * 8U) / 4U));
}


/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_FlexInternalCallback
 * Description   : Internal callback used to translate buffer indices
 *                 and event types for FlexCAN interface
 *
 *END**************************************************************************/
static void CAN_FlexInternalCallback(uint8_t instance,
                                 flexcan_event_type_t eventType,
                                 uint32_t buffIdx,
                                 flexcan_state_t *state)
{
    uint8_t index = CAN_FindFlexCANState(instance);
    can_callback_t callback = userFlexCallbacks[index];

    /* If FlexCAN Rx FIFO is enabled, translate real buffer index to virtual index */
    if ((s_flexcanRxFifoState[index].rxFifoEn == true) && (buffIdx != 0U))
    {
        buffIdx -= CAN_GetVirtualBuffIdx((uint32_t) s_flexcanRxFifoState[index].numIdFilters);
    }

    /* Translate FlexCAN events to CAN PAL events and invoke the callback provided by user */
    switch (eventType)
    {
        case FLEXCAN_EVENT_TX_COMPLETE:
            callback(instance,
                     CAN_EVENT_TX_COMPLETE,
                     (uint8_t) buffIdx,
                     (flexcan_state_t *) state);
            break;
        case FLEXCAN_EVENT_RX_COMPLETE:
            callback(instance,
                     CAN_EVENT_RX_COMPLETE,
                     (uint8_t) buffIdx,
                     (flexcan_state_t *) state);
            break;
        case FLEXCAN_EVENT_RXFIFO_COMPLETE:
            callback(instance,
                     CAN_EVENT_RX_COMPLETE,
                     (uint8_t) buffIdx,
                     (flexcan_state_t *) state);
            break;
        default:
            /* Event types not implemented in PAL */
            break;
    }
}

#endif /* CAN_OVER_FLEXCAN */

#if (defined(CAN_OVER_MCAN))
/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_FindMCANState
 * Description   : Search the state structure of the MCAN instance
 *
 *END**************************************************************************/
static uint8_t CAN_FindMCANState(uint32_t instance)
{
    uint8_t i;

    for (i = 0; i < NO_OF_MCAN_INSTS_FOR_CAN; i++)
    {
        if (s_mcanStateInstanceMapping[i] == instance)
        {
            break;
        }
    }

    if (i >= NO_OF_MCAN_INSTS_FOR_CAN)
    {
        /* Should Never Reach Here */
        DEV_ASSERT(false);
        i = (NO_OF_MCAN_INSTS_FOR_CAN-1U);
    }

    return i;

}

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_McanInternalCallback
 * Description   : Internal callback used to translate buffer indices
 *                 and event types for MCAN interface
 *
 *END**************************************************************************/
static void CAN_McanInternalCallback(uint8_t instance,
                                     mcan_event_type_t eventType,
                                     uint32_t buffIdx,
                                     mcan_state_t *state)
{
    uint8_t index = CAN_FindMCANState(instance);
    can_callback_t callback = userMcanCallbacks[index];

    /* Translate FlexCAN events to CAN PAL events and invoke the callback provided by user */
    switch (eventType)
    {
        case MCAN_EVENT_TX_COMPLETE:
            callback(instance,
                     CAN_EVENT_TX_COMPLETE,
                     (uint32_t) (buffIdx+s_rxBuffs),
                     (mcan_state_t *) state);
            break;
        case MCAN_EVENT_RX_COMPLETE:
            /* If FlexCAN Rx FIFO is enabled, translate real buffer index to virtual index */
            if (s_mcanRxFifoState[index].rxFifoEn == true)
            {
                /* First buffer index is virtually allocated for FIFO0 */
                buffIdx++;
            }
            callback(instance,
                     CAN_EVENT_RX_COMPLETE,
                     (uint32_t) buffIdx,
                     (mcan_state_t *) state);
            break;
        case MCAN_EVENT_RX0FIFO_COMPLETE:
            callback(instance,
                     CAN_EVENT_RX_COMPLETE,
                     (uint32_t) buffIdx,
                     (mcan_state_t *) state);
            break;
        default:
            /* Event types not implemented in PAL */
            break;
    }
}

#endif /* CAN_OVER_MCAN */
/*******************************************************************************
 * Public Functions
 ******************************************************************************/

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_Init
 * Description   : Configures the CAN module
 *
 * Implements    : CAN_Init_Activity
 *END**************************************************************************/
status_t CAN_Init(const can_instance_t * const instance,
                  const can_user_config_t *config)
{
    DEV_ASSERT(instance != NULL);
    DEV_ASSERT(config != NULL);

    status_t status = STATUS_ERROR;
    uint8_t index = 0U;

    /* Define CAN PAL over FLEXCAN */
    #if (defined (CAN_OVER_FLEXCAN))
    if (instance->instType == CAN_INST_TYPE_FLEXCAN)
    {
        DEV_ASSERT(instance->instIdx < CAN_INSTANCE_COUNT);

        flexcan_user_config_t flexcanConfig;

        /* Allocate one of the FLEXCAN state structure for this instance */
        index = CAN_AllocateState(s_flexcanStateIsAllocated,
                                  s_flexcanStateInstanceMapping,
                                  instance->instIdx,
                                  NO_OF_FLEXCAN_INSTS_FOR_CAN);

        /* Clear Rx FIFO state */
        s_flexcanRxFifoState[index].rxFifoEn = false;

        /* Configure features implemented by PAL */
        flexcanConfig.max_num_mb = config->txBuffNum + config->rxBuffNum;
        flexcanConfig.flexcanMode = (flexcan_operation_modes_t) config->mode;
#if FEATURE_CAN_HAS_FD
        flexcanConfig.fd_enable = config->enableFD;
        flexcanConfig.payload = (flexcan_fd_payload_size_t) config->payloadSize;
#endif

        flexcanConfig.bitrate.phaseSeg1 = config->nominalBitrate.phaseSeg1;
        flexcanConfig.bitrate.phaseSeg2 = config->nominalBitrate.phaseSeg2;
        flexcanConfig.bitrate.preDivider = config->nominalBitrate.preDivider;
        flexcanConfig.bitrate.propSeg = config->nominalBitrate.propSeg;
        flexcanConfig.bitrate.rJumpwidth = config->nominalBitrate.rJumpwidth;

#if FEATURE_CAN_HAS_FD
        flexcanConfig.bitrate_cbt.phaseSeg1 = config->dataBitrate.phaseSeg1;
        flexcanConfig.bitrate_cbt.phaseSeg2 = config->dataBitrate.phaseSeg2;
        flexcanConfig.bitrate_cbt.preDivider = config->dataBitrate.preDivider;
        flexcanConfig.bitrate_cbt.propSeg = config->dataBitrate.propSeg;
        flexcanConfig.bitrate_cbt.rJumpwidth = config->dataBitrate.rJumpwidth;
#endif

#if FEATURE_CAN_HAS_PE_CLKSRC_SELECT
        flexcanConfig.pe_clock = (flexcan_clk_source_t) config->peClkSrc;
#endif

        /* If extension is used, configure Rx FIFO */
        if (config->extension != NULL)
        {
            flexcanConfig.is_rx_fifo_needed = true;
            flexcanConfig.num_id_filters = ((extension_flexcan_rx_fifo_t *)
                                           (config->extension))->numIdFilters;
#if FEATURE_CAN_HAS_DMA_ENABLE
            flexcanConfig.rxFifoDMAChannel = 0U;
#endif
            flexcanConfig.transfer_type = FLEXCAN_RXFIFO_USING_INTERRUPTS;

            /* Compute maximum number of virtual buffers */
            flexcanConfig.max_num_mb += CAN_GetVirtualBuffIdx(flexcanConfig.num_id_filters) + 1U;

            /* Update Rx FIFO state */
            s_flexcanRxFifoState[index].rxFifoEn = true;
            s_flexcanRxFifoState[index].numIdFilters = flexcanConfig.num_id_filters;
        }
        else
        {
            flexcanConfig.is_rx_fifo_needed = false;
            flexcanConfig.num_id_filters = FLEXCAN_RX_FIFO_ID_FILTERS_8;
#if FEATURE_CAN_HAS_DMA_ENABLE
            flexcanConfig.rxFifoDMAChannel = 0U;
#endif
            flexcanConfig.transfer_type = FLEXCAN_RXFIFO_USING_INTERRUPTS;
        }

        /* Initialize FLEXCAN instance */
        status = FLEXCAN_DRV_Init((uint8_t) instance->instIdx,
                                  &s_flexcanState[index],
                                  &flexcanConfig);

        /* Configure Rx FIFO if needed */
        if ((status == STATUS_SUCCESS) &&
            (s_flexcanRxFifoState[index].rxFifoEn == true))
        {
            FLEXCAN_DRV_ConfigRxFifo(
                    (uint8_t) instance->instIdx,
                    ((extension_flexcan_rx_fifo_t *) (config->extension))->idFormat,
                    ((extension_flexcan_rx_fifo_t *) (config->extension))->idFilterTable);
        }
    }
    #endif
    /* Define CAN PAL over MCAN */
    #if (defined (CAN_OVER_MCAN))
    if (instance->instType == CAN_INST_TYPE_MCAN)
    {
        DEV_ASSERT(instance->instIdx < M_CAN_INSTANCE_COUNT);

        mcan_user_config_t mcanConfig;

        /* Allocate one of the MCAN state structure for this instance */
        index = CAN_AllocateState(s_mcanStateIsAllocated,
                                  s_mcanStateInstanceMapping,
                                  instance->instIdx,
                                  NO_OF_MCAN_INSTS_FOR_CAN);

         /* Clear Rx FIFO state */
         s_mcanRxFifoState[index].rxFifoEn = false;
         /* Configure features implemented by PAL */
         mcanConfig.rx_num_mb = config->rxBuffNum;
         mcanConfig.tx_num_mb = config->txBuffNum;
         s_rxBuffs = config->rxBuffNum;
         s_txBuffs = config->txBuffNum;
         mcanConfig.mcanMode = (mcan_operation_modes_t) config->mode;

         mcanConfig.fd_enable = config->enableFD;
         switch (config->payloadSize)
         {
         case CAN_PAYLOAD_SIZE_8:
             mcanConfig.payload = MCAN_PAYLOAD_SIZE_8;
             break;
         case CAN_PAYLOAD_SIZE_16:
             mcanConfig.payload = MCAN_PAYLOAD_SIZE_16;
             break;
         case CAN_PAYLOAD_SIZE_32:
             mcanConfig.payload = MCAN_PAYLOAD_SIZE_32;
             break;
         case CAN_PAYLOAD_SIZE_64:
             mcanConfig.payload = MCAN_PAYLOAD_SIZE_64;
             break;
         default :
             /* All cases covered */
             break;
         }

        mcanConfig.bitrate.phaseSeg1 = config->nominalBitrate.phaseSeg1;
        mcanConfig.bitrate.phaseSeg2 = config->nominalBitrate.phaseSeg2;
        mcanConfig.bitrate.preDivider = config->nominalBitrate.preDivider;
        mcanConfig.bitrate.propSeg = config->nominalBitrate.propSeg;
        mcanConfig.bitrate.rJumpwidth = config->nominalBitrate.rJumpwidth;

        mcanConfig.bitrate_cbt.phaseSeg1 = config->dataBitrate.phaseSeg1;
        mcanConfig.bitrate_cbt.phaseSeg2 = config->dataBitrate.phaseSeg2;
        mcanConfig.bitrate_cbt.preDivider = config->dataBitrate.preDivider;
        mcanConfig.bitrate_cbt.propSeg = config->dataBitrate.propSeg;
        mcanConfig.bitrate_cbt.rJumpwidth = config->dataBitrate.rJumpwidth;

        if (config->extension != NULL)
        {

            mcanConfig.rx_fifo_needed = MCAN_RXFIFO_0_ENABLE;
            s_mcanRxFifoState[index].rxFifoEn = true;
            mcanConfig.fifoConfigs = (rxfifo_conf_t *) &((extension_mcan_rx_fifo_t *) (config->extension))->rxFifo0config;
            mcanConfig.num_id_filters = ((extension_mcan_rx_fifo_t *) config->extension)->numIdFilters;
            mcanConfig.filterConfigs = ((extension_mcan_rx_fifo_t *) config->extension)->idFilterTable;
            /* Add virtual Buff 0 for RxFIFO, individual RX buff index is from 1 to rxBuffNum and
             * individual TX buff index is from rxBuffNum to (rxBuffNum + txBuffNum) */
            s_rxBuffs++;
        }
        else
        {
            mcanConfig.rx_fifo_needed = MCAN_RXFIFO_DISABLED;
            mcanConfig.num_id_filters = 0U;
            mcanConfig.filterConfigs = NULL;
        }
        /* Initialize MCAN instance */
        status = MCAN_DRV_Init((uint8_t) instance->instIdx,
                               &s_mcanState[index],
                               &mcanConfig);
        /* Configure Rx FIFO if needed */
        if ((status == STATUS_SUCCESS) && (s_mcanRxFifoState[index].rxFifoEn == true))
        {
            status = MCAN_DRV_ConfigRxFifo((uint8_t) instance->instIdx,
                                          ((extension_mcan_rx_fifo_t *) (config->extension))->idFilterTable,
                                          ((extension_mcan_rx_fifo_t *) (config->extension))->numIdFilters);
        }
    }
    #endif /* Define CAN PAL over MCAN */
    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_Deinit
 * Description   : De-initializes the CAN module
 *
 * Implements    : CAN_Deinit_Activity
 *END**************************************************************************/
status_t CAN_Deinit(const can_instance_t * const instance)
{
    DEV_ASSERT(instance != NULL);

    status_t status = STATUS_ERROR;

    /* Define CAN PAL over FLEXCAN */
    #if defined(CAN_OVER_FLEXCAN)
    if (instance->instType == CAN_INST_TYPE_FLEXCAN)
    {
        /* De-initialize the FlexCAN module */
        status = FLEXCAN_DRV_Deinit((uint8_t) instance->instIdx);

        if (status == STATUS_SUCCESS)
        {
            /* Clear FlexCAN instance mapping */
            CAN_FreeState(s_flexcanStateIsAllocated,
                          s_flexcanStateInstanceMapping,
                          instance->instIdx,
                          NO_OF_FLEXCAN_INSTS_FOR_CAN);
        }
    }
    #endif /* CAN_OVER_FLEXCAN */

    #if defined(CAN_OVER_MCAN)
    if (instance->instType == CAN_INST_TYPE_MCAN)
    {
        /* De-initialize the MCAN module */
        status = MCAN_DRV_Deinit((uint8_t) instance->instIdx);
    
        if (status == STATUS_SUCCESS)
        {
            /* Clear MCAN instance mapping */
            CAN_FreeState(s_mcanStateIsAllocated,
                          s_mcanStateInstanceMapping,
                          instance->instIdx,
                          NO_OF_MCAN_INSTS_FOR_CAN);
        }
    }
    #endif /* CAN_OVER_MCAN*/

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_SetBitrate
 * Description   : Configures the CAN bit timing variables.
 *
 * Implements    : CAN_SetBitrate_Activity
 *END**************************************************************************/
status_t CAN_SetBitrate(const can_instance_t * const instance,
                        can_bitrate_phase_t phase,
                        const can_time_segment_t *bitTiming)
{
    DEV_ASSERT(instance != NULL);
    DEV_ASSERT(bitTiming != NULL);

    status_t status = STATUS_ERROR;
    /* Define CAN PAL over FLEXCAN */
    #if defined(CAN_OVER_FLEXCAN)
    if(instance->instType == CAN_INST_TYPE_FLEXCAN)
    {
        flexcan_time_segment_t flexcanBitTime;
        flexcanBitTime.phaseSeg1 = bitTiming->phaseSeg1;
        flexcanBitTime.phaseSeg2 = bitTiming->phaseSeg2;
        flexcanBitTime.preDivider = bitTiming->preDivider;
        flexcanBitTime.propSeg = bitTiming->propSeg;
        flexcanBitTime.rJumpwidth = bitTiming->rJumpwidth;

        status = STATUS_SUCCESS;

        if (phase == CAN_NOMINAL_BITRATE)
        {
            FLEXCAN_DRV_SetBitrate((uint8_t) instance->instIdx,
                                   &flexcanBitTime);
        }
#if FEATURE_CAN_HAS_FD
        else
        {
            FLEXCAN_DRV_SetBitrateCbt((uint8_t) instance->instIdx,
                                      &flexcanBitTime);
        }
#endif /* FEATURE_CAN_HAS_FD */
    }
    #endif /* CAN_OVER_FLEXCAN */

    #if defined(CAN_OVER_MCAN)
    if(instance->instType == CAN_INST_TYPE_MCAN)
    {
        mcan_time_segment_t mcanBitTime;
        mcanBitTime.phaseSeg1 = bitTiming->phaseSeg1;
        mcanBitTime.phaseSeg2 = bitTiming->phaseSeg2;
        mcanBitTime.preDivider = bitTiming->preDivider;
        mcanBitTime.propSeg = bitTiming->propSeg;
        mcanBitTime.rJumpwidth = bitTiming->rJumpwidth;

        if (phase == CAN_NOMINAL_BITRATE)
        {
            status = MCAN_DRV_SetNominalBitrate((uint8_t) instance->instIdx,
                                                &mcanBitTime);
            if (status == STATUS_SUCCESS)
            {
                status = MCAN_DRV_SetDataBitrate((uint8_t) instance->instIdx,
                                                 &mcanBitTime);
            }
        }
        else
        {
            status = MCAN_DRV_SetDataBitrate((uint8_t) instance->instIdx,
                                             &mcanBitTime);
        }
    }
    #endif /* CAN_OVER_MCAN */

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_GetBitrate
 * Description   : Returns the CAN configured bit timing variables.
 *
 * Implements    : CAN_GetBitrate_Activity
 *END**************************************************************************/
status_t CAN_GetBitrate(const can_instance_t * const instance,
                        can_bitrate_phase_t phase,
                        can_time_segment_t *bitTiming)
{
    DEV_ASSERT(instance != NULL);
    DEV_ASSERT(bitTiming != NULL);

    status_t status = STATUS_ERROR;

    /* Define CAN PAL over FLEXCAN */
    #if defined(CAN_OVER_FLEXCAN)
    if (instance->instType == CAN_INST_TYPE_FLEXCAN)
    {
        status = STATUS_SUCCESS;

        if (phase == CAN_NOMINAL_BITRATE)
        {
            FLEXCAN_DRV_GetBitrate((uint8_t) instance->instIdx,
                                   (flexcan_time_segment_t *) bitTiming);
        }
#if FEATURE_CAN_HAS_FD
        else
        {
            FLEXCAN_DRV_GetBitrateFD((uint8_t) instance->instIdx,
                                     (flexcan_time_segment_t *) bitTiming);
        }
#endif
    }
    #endif
    
    /* Define CAN PAL over MCAN */
    #if defined(CAN_OVER_MCAN)
    if (instance->instType == CAN_INST_TYPE_MCAN)
    {
        status = STATUS_SUCCESS;

        if (phase == CAN_NOMINAL_BITRATE)
        {
            MCAN_DRV_GetBitrate((uint8_t) instance->instIdx,
                                   (mcan_time_segment_t *) bitTiming);
        }
        else
        {
            MCAN_DRV_GetBitrateFD((uint8_t) instance->instIdx,
                                     (mcan_time_segment_t *) bitTiming);
        }
    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_ConfigTxBuff
 * Description   : Configures a buffer for transmission.
 *
 * Implements    : CAN_ConfigTxBuff_Activity
 *END**************************************************************************/
status_t CAN_ConfigTxBuff(const can_instance_t * const instance,
                          uint32_t buffIdx,
                          const can_buff_config_t *config)
{
    DEV_ASSERT(instance != NULL);
    DEV_ASSERT(config != NULL);

    status_t status = STATUS_ERROR;
    uint8_t index;

    /* Define CAN PAL over FLEXCAN */
    #if defined(CAN_OVER_FLEXCAN)
    if (instance->instType == CAN_INST_TYPE_FLEXCAN)
    {
        DEV_ASSERT(instance->instIdx < CAN_INSTANCE_COUNT);

        index = CAN_FindFlexCANState(instance->instIdx);

        /* If Rx FIFO is enabled, buffer 0 (zero) can only be used for reception */
        DEV_ASSERT((s_flexcanRxFifoState[index].rxFifoEn == false) || (buffIdx != 0U));
        /* Check buffer index to avoid overflow */
        DEV_ASSERT(buffIdx < FEATURE_CAN_MAX_MB_NUM);

        flexcan_data_info_t dataInfo = {
            .msg_id_type = (flexcan_msgbuff_id_type_t) (config->idType),
            /* Used only to initialize the MB, when send data will be updated */
            .data_length = (uint32_t) 8U,
#if FEATURE_CAN_HAS_FD
            .fd_enable = config->enableFD,
            .fd_padding = config->fdPadding,
            .enable_brs = config->enableBRS,
#endif
            .is_remote = false
        };

        /* Save buffer config  for later use */
        s_hwObjConfigs[index][buffIdx] = config;

        /* Compute virtual buffer index */
        if (s_flexcanRxFifoState[index].rxFifoEn)
        {
            buffIdx += CAN_GetVirtualBuffIdx(s_flexcanRxFifoState[index].numIdFilters);
        }

        /* Configure FlexCAN MB for transmission */
        status = FLEXCAN_DRV_ConfigTxMb((uint8_t) instance->instIdx,
                                        (uint8_t) buffIdx,
                                        &dataInfo,
                                        0U);
    }
    #endif
    /* Define CAN PAL over MCAN */
    #if defined(CAN_OVER_MCAN)
    if (instance->instType == CAN_INST_TYPE_MCAN)
    {
        DEV_ASSERT(instance->instIdx < M_CAN_INSTANCE_COUNT);
        index = CAN_FindMCANState(instance->instIdx);
        /* The first rxBuff no as buffers 0 -> (rxBuff-1) are allocated for Rx,
         * the rest from rxBuff to (rxBuff+txBuff-1) are for Tx*/
        if ((buffIdx >= s_rxBuffs) && (buffIdx < (s_rxBuffs + s_txBuffs)))
        {
            /* Check buffer index to avoid overflow */
            DEV_ASSERT((buffIdx - s_rxBuffs) < FEATURE_MCAN_TX_MB_NUM);
            mcan_data_info_t dataInfo = {
                .msg_id_type = (mcan_msgbuff_id_type_t) (config->idType),
                /* Used only to initialize the MB, when send data will be updated */
                .data_length = (uint32_t) 8U,
                .fd_enable = config->enableFD,
                .fd_padding = config->fdPadding,
                .enable_brs = config->enableBRS,
                .is_remote = false
            };

            /* Save buffer config  for later use */
            s_hwObjConfigs1[index][buffIdx] = config;
            /* Configure FlexCAN MB for transmission */
            status = MCAN_DRV_ConfigTxMb((uint8_t) instance->instIdx,
                                         (uint8_t) (buffIdx - s_rxBuffs),
                                         &dataInfo,
                                         0U);
        }
    }
    #endif /* CAN_OVER_MCAN */
    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_ConfigRemoteResponseBuff
 * Description   : Configures a transmit buffer for remote frame response. This
 * function will set up the buffer fields, configure the buffer code for Tx
 * buffer as RX_RANSWER, and enable the buffer interrupt.
 *
 * Implements    : CAN_ConfigRemoteResponseBuff_Activity
 *END**************************************************************************/
status_t CAN_ConfigRemoteResponseBuff(const can_instance_t * const instance,
                                      uint32_t buffIdx,
                                      const can_buff_config_t *config,
                                      const can_message_t *message)
{
    DEV_ASSERT(instance != NULL);
    DEV_ASSERT(config != NULL);

    status_t status = STATUS_ERROR;

    /* Define CAN PAL over FLEXCAN */
    #if defined(CAN_OVER_FLEXCAN)
    if (instance->instType == CAN_INST_TYPE_FLEXCAN)
    {
        uint8_t index;
        DEV_ASSERT(instance->instIdx < CAN_INSTANCE_COUNT);

        index = CAN_FindFlexCANState(instance->instIdx);

        /* If Rx FIFO is enabled, buffer 0 (zero) can only be used for reception */
        DEV_ASSERT((s_flexcanRxFifoState[index].rxFifoEn == false) || (buffIdx != 0U));
        /* Check buffer index to avoid overflow */
        DEV_ASSERT(buffIdx < FEATURE_CAN_MAX_MB_NUM);

        flexcan_data_info_t dataInfo = {
            .msg_id_type = (flexcan_msgbuff_id_type_t) (config->idType),
            .data_length = message->length,
#if FEATURE_CAN_HAS_FD
            .fd_enable = config->enableFD,
            .fd_padding = config->fdPadding,
            .enable_brs = config->enableBRS,
#endif
            .is_remote = config->isRemote
        };

        /* Save buffer config  for later use */
        s_hwObjConfigs[index][buffIdx] = config;

        /* Compute virtual buffer index */
        if (s_flexcanRxFifoState[index].rxFifoEn)
        {
            buffIdx += CAN_GetVirtualBuffIdx(s_flexcanRxFifoState[index].numIdFilters);
        }

        /* Configure FlexCAN MB for transmission */
        status = FLEXCAN_DRV_ConfigRemoteResponseMb((uint8_t) instance->instIdx,
                                                    (uint8_t) buffIdx,
                                                    &dataInfo,
                                                    message->id,
                                                    message->data);
    }
    #endif
    /* Define CAN PAL over MCAN */
    #if defined(CAN_OVER_MCAN)
    if (instance->instType == CAN_INST_TYPE_MCAN)
    {
        DEV_ASSERT(instance->instIdx < M_CAN_INSTANCE_COUNT);
        /* Do nothing, because this functionality is not support by MCAN peripheral. */
        status = STATUS_UNSUPPORTED;
    }
    #endif
    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_ConfigRxBuff
 * Description   : Configures a buffer for reception.
 *
 * Implements    : CAN_ConfigRxBuff_Activity
 *END**************************************************************************/
status_t CAN_ConfigRxBuff(const can_instance_t * const instance,
                          uint32_t buffIdx,
                          const can_buff_config_t *config,
                          uint32_t acceptedId)
{
    DEV_ASSERT(instance != NULL);
    DEV_ASSERT(config != NULL);

    status_t status = STATUS_ERROR;
    uint8_t index;

    /* Define CAN PAL over FLEXCAN */
    #if defined(CAN_OVER_FLEXCAN)
    if (instance->instType == CAN_INST_TYPE_FLEXCAN)
    {
        DEV_ASSERT(instance->instIdx < CAN_INSTANCE_COUNT);

        index = CAN_FindFlexCANState(instance->instIdx);

        /* If Rx FIFO is enabled, buffer 0 (zero) is configured at init time */
        DEV_ASSERT((s_flexcanRxFifoState[index].rxFifoEn == false) || (buffIdx != 0U));
        /* Check buffer index to avoid overflow */
        DEV_ASSERT(buffIdx < FEATURE_CAN_MAX_MB_NUM);

        flexcan_data_info_t dataInfo = {
            .msg_id_type = (flexcan_msgbuff_id_type_t) (config->idType),
            .data_length = (config->enableFD ? (uint32_t) 64U :  (uint32_t) 8U),
#if FEATURE_CAN_HAS_FD
            .fd_enable = config->enableFD,
            .fd_padding = config->fdPadding,
            .enable_brs = config->enableBRS,
#endif
            .is_remote = config->isRemote
        };

        /* Save buffer config for later use */
        s_hwObjConfigs[index][buffIdx] = config;
        /* Compute virtual buffer index */
        if (s_flexcanRxFifoState[index].rxFifoEn)
        {
            buffIdx += CAN_GetVirtualBuffIdx(s_flexcanRxFifoState[index].numIdFilters);
        }

        /* Configure FlexCAN MB for reception */
        status = FLEXCAN_DRV_ConfigRxMb((uint8_t) instance->instIdx,
                                        (uint8_t) buffIdx,
                                        &dataInfo,
                                        acceptedId);
    }
    #endif /* CAN_OVER_FLEXCAN */
    #if defined(CAN_OVER_MCAN)
    if (instance->instType == CAN_INST_TYPE_MCAN)
    {
        DEV_ASSERT(instance->instIdx < M_CAN_INSTANCE_COUNT);

        index = CAN_FindMCANState(instance->instIdx);

        if (buffIdx < s_rxBuffs)
        {
            /* If Rx FIFO is enabled, buffer 0 (zero) can only be used for reception */
            DEV_ASSERT((s_mcanRxFifoState[index].rxFifoEn == false) || (buffIdx != 0U));
            if (s_mcanRxFifoState[index].rxFifoEn == true)
            {
                /* In case of RxFIFO enabled the buff index is from 1 to Rx Buff No and
                 * the real Message Buffers are from 0 to (Rx Buff - 1) */
                buffIdx--;
            }
            mcan_data_info_t dataInfo = {
                .msg_id_type = (mcan_msgbuff_id_type_t) (config->idType),
                .data_length = (config->enableFD ? (uint32_t) 64U :  (uint32_t) 8U),
                .fd_enable = config->enableFD,
                .fd_padding = config->fdPadding,
                .enable_brs = config->enableBRS,
                .is_remote = false
            };
            /* Save buffer config  for later use */
            s_hwObjConfigs1[index][buffIdx] = config;
            /* Check buffer index to avoid overflow */
            DEV_ASSERT(buffIdx < FEATURE_MCAN_RX_MB_NUM);
            /* Configure MCAN MB for reception */
            status = MCAN_DRV_ConfigRxMb((uint8_t) instance->instIdx,
                                         (uint8_t) buffIdx,
                                         &dataInfo,
                                         acceptedId);
        }
    }
    #endif /* CAN_OVER_MCAN */
    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_Send
 * Description   : Sends a CAN frame using the specified buffer.
 *
 * Implements    : CAN_Send_Activity
 *END**************************************************************************/
status_t CAN_Send(const can_instance_t * const instance,
                  uint32_t buffIdx,
                  const can_message_t *message)
{
    DEV_ASSERT(instance != NULL);
    DEV_ASSERT(message != NULL);

    status_t status = STATUS_ERROR;
    uint8_t index;

    /* Define CAN PAL over FLEXCAN */
    #if defined(CAN_OVER_FLEXCAN)
    if (instance->instType == CAN_INST_TYPE_FLEXCAN)
    {
        DEV_ASSERT(instance->instIdx < CAN_INSTANCE_COUNT);

        index = CAN_FindFlexCANState(instance->instIdx);

        /* If Rx FIFO is enabled, buffer 0 (zero) can only be used for reception */
        DEV_ASSERT((s_flexcanRxFifoState[index].rxFifoEn == false) || (buffIdx != 0U));
        /* Check buffer index to avoid overflow */
        DEV_ASSERT(buffIdx < FEATURE_CAN_MAX_MB_NUM);

        flexcan_data_info_t dataInfo = {
            .msg_id_type = (flexcan_msgbuff_id_type_t) s_hwObjConfigs[index][buffIdx]->idType,
            .data_length = message->length,
#if FEATURE_CAN_HAS_FD
            .fd_enable = s_hwObjConfigs[index][buffIdx]->enableFD,
            .fd_padding = s_hwObjConfigs[index][buffIdx]->fdPadding,
            .enable_brs = s_hwObjConfigs[index][buffIdx]->enableBRS,
#endif
            .is_remote = s_hwObjConfigs[index][buffIdx]->isRemote
        };

        /* Compute virtual buffer index */
        if (s_flexcanRxFifoState[index].rxFifoEn)
        {
            buffIdx += CAN_GetVirtualBuffIdx(s_flexcanRxFifoState[index].numIdFilters);
        }

        status = FLEXCAN_DRV_Send((uint8_t) instance->instIdx,
                                  (uint8_t) buffIdx,
                                  &dataInfo,
                                  message->id,
                                  message->data);
    }
    #endif

    #if defined(CAN_OVER_MCAN)
    if (instance->instType == CAN_INST_TYPE_MCAN)
    {
        DEV_ASSERT(instance->instIdx < M_CAN_INSTANCE_COUNT);

        index = CAN_FindMCANState(instance->instIdx);

        if ((buffIdx >= s_rxBuffs) && (buffIdx < (s_rxBuffs+s_txBuffs)))
        {
            /* Check buffer index to avoid overflow */
            DEV_ASSERT((buffIdx - s_rxBuffs) < FEATURE_MCAN_TX_MB_NUM);

            mcan_data_info_t dataInfo = {
                .msg_id_type = (mcan_msgbuff_id_type_t) s_hwObjConfigs1[index][buffIdx]->idType,
                .data_length = message->length,
                .fd_enable = s_hwObjConfigs1[index][buffIdx]->enableFD,
                .fd_padding = s_hwObjConfigs1[index][buffIdx]->fdPadding,
                .enable_brs = s_hwObjConfigs1[index][buffIdx]->enableBRS,
                .is_remote = s_hwObjConfigs1[index][buffIdx]->isRemote
            };

            status = MCAN_DRV_Send((uint8_t) instance->instIdx,
                                   (uint8_t) (buffIdx-s_rxBuffs),
                                   &dataInfo,
                                   message->id,
                                   message->data);
        }
    }
    #endif
    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_SendBlocking
 * Description   : Sends a CAN frame using the specified buffer,
 *                 in a blocking manner.
 *
 * Implements    : CAN_SendBlocking_Activity
 *END**************************************************************************/
status_t CAN_SendBlocking(const can_instance_t * const instance,
                          uint32_t buffIdx,
                          const can_message_t *message,
                          uint32_t timeoutMs)
{
    DEV_ASSERT(instance != NULL);
    DEV_ASSERT(message != NULL);

    status_t status = STATUS_ERROR;
    uint8_t index;

    /* Define CAN PAL over FLEXCAN */
    #if defined(CAN_OVER_FLEXCAN)
    if (instance->instType == CAN_INST_TYPE_FLEXCAN)
    {
        DEV_ASSERT(instance->instIdx < CAN_INSTANCE_COUNT);

        index = CAN_FindFlexCANState(instance->instIdx);

        /* If Rx FIFO is enabled, buffer 0 (zero) can only be used for reception */
        DEV_ASSERT((s_flexcanRxFifoState[index].rxFifoEn == false) || (buffIdx != 0U));
        /* Check buffer index to avoid overflow */
        DEV_ASSERT(buffIdx < FEATURE_CAN_MAX_MB_NUM);

        flexcan_data_info_t dataInfo = {
            .msg_id_type = (flexcan_msgbuff_id_type_t) s_hwObjConfigs[index][buffIdx]->idType,
            .data_length = message->length,
#if FEATURE_CAN_HAS_FD
            .fd_enable = s_hwObjConfigs[index][buffIdx]->enableFD,
            .fd_padding = s_hwObjConfigs[index][buffIdx]->fdPadding,
            .enable_brs = s_hwObjConfigs[index][buffIdx]->enableBRS,
#endif
            .is_remote = s_hwObjConfigs[index][buffIdx]->isRemote
        };

        /* Compute virtual buffer index */
        if (s_flexcanRxFifoState[index].rxFifoEn)
        {
            buffIdx += CAN_GetVirtualBuffIdx(s_flexcanRxFifoState[index].numIdFilters);
        }

        status = FLEXCAN_DRV_SendBlocking((uint8_t) instance->instIdx,
                                          (uint8_t) buffIdx,
                                          &dataInfo,
                                          message->id,
                                          message->data,
                                          timeoutMs);
    }
    #endif /* defined(CAN_OVER_FLEXCAN) */
    /* Define CAN PAL over MCAN */
    #if defined(CAN_OVER_MCAN)
    if (instance->instType == CAN_INST_TYPE_MCAN)
    {
       DEV_ASSERT(instance->instIdx < M_CAN_INSTANCE_COUNT);

       index = CAN_FindMCANState(instance->instIdx);
       if ((buffIdx >= s_rxBuffs) && (buffIdx < (s_rxBuffs+s_txBuffs)))
       {
           /* Check buffer index to avoid overflow */
           DEV_ASSERT((buffIdx - s_rxBuffs) < FEATURE_MCAN_TX_MB_NUM);

           mcan_data_info_t dataInfo = {
               .msg_id_type = (mcan_msgbuff_id_type_t) s_hwObjConfigs1[index][buffIdx]->idType,
               .data_length = message->length,
               .fd_enable = s_hwObjConfigs1[index][buffIdx]->enableFD,
               .fd_padding = s_hwObjConfigs1[index][buffIdx]->fdPadding,
               .enable_brs = s_hwObjConfigs1[index][buffIdx]->enableBRS,
               .is_remote = s_hwObjConfigs1[index][buffIdx]->isRemote
           };

           status = MCAN_DRV_SendBlocking((uint8_t) instance->instIdx,
                                          (uint8_t) (buffIdx - s_rxBuffs),
                                          &dataInfo,
                                          message->id,
                                          message->data,
                                          timeoutMs);
       }
    }
       #endif /* defined(CAN_OVER_MCAN) */

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_Receive
 * Description   : Receives a CAN frame using the specified message buffer.
 *
 * Implements    : CAN_Receive_Activity
 *END**************************************************************************/
status_t CAN_Receive(const can_instance_t * const instance,
                     uint32_t buffIdx,
                     can_message_t *message)
{
    DEV_ASSERT(instance != NULL);
    DEV_ASSERT(message != NULL);

    status_t status = STATUS_ERROR;
    uint8_t index;

    /* Define CAN PAL over FLEXCAN */
    #if defined(CAN_OVER_FLEXCAN)
    if (instance->instType == CAN_INST_TYPE_FLEXCAN)
    {
        /* Check buffer index to avoid overflow */
        DEV_ASSERT(buffIdx < FEATURE_CAN_MAX_MB_NUM);
        DEV_ASSERT(instance->instIdx < CAN_INSTANCE_COUNT);

        index = CAN_FindFlexCANState(instance->instIdx);

        /* If Rx FIFO is enabled, buffer 0 (zero) is used to read frames received in FIFO0  */
        if ((s_flexcanRxFifoState[index].rxFifoEn == true) && (buffIdx == 0U))
        {
            status = FLEXCAN_DRV_RxFifo((uint8_t) instance->instIdx,
                                        (flexcan_msgbuff_t *) message);
        }
        else
        {
            /* Compute virtual buffer index */
            if (s_flexcanRxFifoState[index].rxFifoEn)
            {
                buffIdx += CAN_GetVirtualBuffIdx(s_flexcanRxFifoState[index].numIdFilters);
            }

            status = FLEXCAN_DRV_Receive((uint8_t) instance->instIdx,
                                         (uint8_t) buffIdx,
                                         (flexcan_msgbuff_t *) message);
        }
    }
    #endif /* defined(CAN_OVER_FLEXCAN) */

    /* Define CAN PAL over MCAN */
    #if defined(CAN_OVER_MCAN)
    if (instance->instType == CAN_INST_TYPE_MCAN)
    {
        DEV_ASSERT(instance->instIdx < M_CAN_INSTANCE_COUNT);

        index = CAN_FindMCANState(instance->instIdx);

        if (buffIdx < s_rxBuffs)
        {
            /* If Rx FIFO is enabled, buffer 0 (zero) is used to read frames received in FIFO0  */
            if ((s_mcanRxFifoState[index].rxFifoEn == true) && (buffIdx == 0U))
            {
                status = MCAN_DRV_RxFifo((uint8_t) instance->instIdx,
                                         0U,
                                         (mcan_msgbuff_t *) message);
            }
            else
            {
                if (s_mcanRxFifoState[index].rxFifoEn == true)
                {
                    /* In case of RxFIFO enabled the individual buff index is from 1 to Rx Buff No and
                     * the real Message Buffers are from 0 to (Rx Buff - 1) */
                    buffIdx--;
                }
                /* Check buffer index to avoid overflow */
                DEV_ASSERT(buffIdx < FEATURE_MCAN_RX_MB_NUM);
                status = MCAN_DRV_Receive((uint8_t) instance->instIdx,
                                          (uint8_t) buffIdx,
                                          (mcan_msgbuff_t *) message);
            }
        }
    }
    #endif /* defined(CAN_OVER_MCAN) */

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_ReceiveBlocking
 * Description   : Receives a CAN frame using the specified buffer,
 *                 in a blocking manner.
 *
 * Implements    : CAN_ReceiveBlocking_Activity
 *END**************************************************************************/
status_t CAN_ReceiveBlocking(const can_instance_t * const instance,
                             uint32_t buffIdx,
                             can_message_t *message,
                             uint32_t timeoutMs)
{
    DEV_ASSERT(instance != NULL);
    DEV_ASSERT(message != NULL);

    status_t status = STATUS_ERROR;
    uint8_t index;

    /* Define CAN PAL over FLEXCAN */
    #if defined(CAN_OVER_FLEXCAN)
    if (instance->instType == CAN_INST_TYPE_FLEXCAN)
    {
        /* Check buffer index to avoid overflow */
        DEV_ASSERT(buffIdx < FEATURE_CAN_MAX_MB_NUM);
        DEV_ASSERT(instance->instIdx < CAN_INSTANCE_COUNT);

        index = CAN_FindFlexCANState(instance->instIdx);

        /* If Rx FIFO is enabled, buffer 0 (zero) is used to read frames received in FIFO */
        if ((s_flexcanRxFifoState[index].rxFifoEn == true) && (buffIdx == 0U))
        {
            status = FLEXCAN_DRV_RxFifoBlocking((uint8_t) instance->instIdx,
                                                (flexcan_msgbuff_t *) message,
                                                timeoutMs);
        }
        else
        {
            /* Compute virtual buffer index */
            if (s_flexcanRxFifoState[index].rxFifoEn)
            {
                buffIdx += CAN_GetVirtualBuffIdx(s_flexcanRxFifoState[index].numIdFilters);
            }

            status = FLEXCAN_DRV_ReceiveBlocking((uint8_t) instance->instIdx,
                                                 (uint8_t) buffIdx,
                                                 (flexcan_msgbuff_t *) message,
                                                 timeoutMs);
        }
    }
    #endif

    /* Define CAN PAL over FLEXCAN */
    #if defined(CAN_OVER_MCAN)
    if (instance->instType == CAN_INST_TYPE_MCAN)
    {
        /* Check buffer index to avoid overflow */
        DEV_ASSERT(buffIdx < FEATURE_MCAN_RX_MB_NUM);
        DEV_ASSERT(instance->instIdx < M_CAN_INSTANCE_COUNT);

        index = CAN_FindMCANState(instance->instIdx);

        /* If Rx FIFO is enabled, buffer 0 (zero) is used to read frames received in FIFO */
        if ((s_mcanRxFifoState[index].rxFifoEn == true) && (buffIdx == 0U))
        {
            status = MCAN_DRV_RxFifoBlocking((uint8_t) instance->instIdx,
                                             0U,
                                             (mcan_msgbuff_t *) message,
                                             timeoutMs);
        }
        else if (buffIdx < s_rxBuffs)
        {
            /* If Rx FIFO is enabled, buffer 0 (zero) can only be used for reception */
            DEV_ASSERT((s_mcanRxFifoState[index].rxFifoEn == false) || (buffIdx != 0U));
            if (s_mcanRxFifoState[index].rxFifoEn == true)
            {
                /* In case of RxFIFO enabled the buff index is from 1 to Rx Buff No and
                 * the real Message Buffers are from 0 to (Rx Buff - 1) */
                buffIdx--;
            }

            status = MCAN_DRV_ReceiveBlocking((uint8_t) instance->instIdx,
                                              (uint8_t) buffIdx,
                                              (mcan_msgbuff_t *) message,
                                              timeoutMs);
        }
        else
        {
            /* Misra Require Rule 15.7 */
        }
    }
    #endif
    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_AbortTransfer
 * Description   : This function ends a non-blocking CAN transfer early.
 *
 * Implements    : CAN_AbortTransfer_Activity
 *END**************************************************************************/
status_t CAN_AbortTransfer(const can_instance_t * const instance,
                           uint32_t buffIdx)
{
    DEV_ASSERT(instance != NULL);

    status_t status = STATUS_ERROR;
    uint8_t index;

    /* Define CAN PAL over FLEXCAN */
    #if defined(CAN_OVER_FLEXCAN)
    if (instance->instType == CAN_INST_TYPE_FLEXCAN)
    {
        /* Check buffer index to avoid overflow */
        DEV_ASSERT(buffIdx < FEATURE_CAN_MAX_MB_NUM);
        DEV_ASSERT(instance->instIdx < CAN_INSTANCE_COUNT);

       index = CAN_FindFlexCANState(instance->instIdx);

       /* Compute virtual buffer index */
       if (s_flexcanRxFifoState[index].rxFifoEn)
       {
           buffIdx += CAN_GetVirtualBuffIdx(s_flexcanRxFifoState[index].numIdFilters);
       }

        status = FLEXCAN_DRV_AbortTransfer((uint8_t) instance->instIdx,
                                           (uint8_t)buffIdx);
    }
    #endif /* CAN_OVER_FLEXCAN */

    #if defined(CAN_OVER_MCAN)
    if (instance->instType == CAN_INST_TYPE_MCAN)
    {
        /* Check buffer index to avoid overflow */
        DEV_ASSERT(buffIdx < (s_rxBuffs + s_txBuffs));
        DEV_ASSERT(instance->instIdx < M_CAN_INSTANCE_COUNT);

        index = CAN_FindMCANState(instance->instIdx);
        if (buffIdx < s_rxBuffs)
        {
            if ((s_mcanRxFifoState[index].rxFifoEn == true) && (buffIdx == 0U))
            {
                status = MCAN_DRV_AbortTransfer((uint8_t) instance->instIdx,
                                                MCAN_RX_FIFO0,
                                                (uint8_t) buffIdx);
            }
            else
            {
                if (s_mcanRxFifoState[index].rxFifoEn == true)
                {
                    /* In case of RxFIFO enabled the buff index is from 1 to Rx Buff No and
                     * the real Message Buffers are from 0 to (Rx Buff - 1) */
                    buffIdx--;
                }

                status = MCAN_DRV_AbortTransfer((uint8_t) instance->instIdx,
                                                MCAN_RX_BUFF,
                                                (uint8_t) buffIdx);
            }
        }
        else
        {
            if (buffIdx < (s_rxBuffs+s_txBuffs))
            {
                status = MCAN_DRV_AbortTransfer((uint8_t) instance->instIdx,
                                                MCAN_TX_BUFF,
                                                (uint8_t) (buffIdx-s_rxBuffs));
            }
        }

    }
    #endif /* CAN_OVER_MCAN */
    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_SetRxFilter
 * Description   : Configures an ID filter mask for a specific reception buffer if FlexCAN interface is used.
 *                    Configures an ID filter mask for a specific RxFifo filter if MCAN interface is used.
 *
 * Implements    : CAN_SetRxFilter_Activity
 *END**************************************************************************/
status_t CAN_SetRxFilter(const can_instance_t * const instance,
                         can_msg_id_type_t idType,
                         uint32_t buffIdx,
                         uint32_t mask)
{
    DEV_ASSERT(instance != NULL);

    status_t status = STATUS_ERROR;
    uint8_t index;

    /* Define CAN PAL over FLEXCAN */
    #if defined(CAN_OVER_FLEXCAN)
    if (instance->instType == CAN_INST_TYPE_FLEXCAN)
    {
        DEV_ASSERT(instance->instIdx < CAN_INSTANCE_COUNT);

        index = CAN_FindFlexCANState(instance->instIdx);

        /* If Rx FIFO is enabled, buffer 0 (zero) filters are configured at init time */
        DEV_ASSERT((s_flexcanRxFifoState[index].rxFifoEn == false) || (buffIdx != 0U));
        (void) index;
        /* Check buffer index to avoid overflow */
        DEV_ASSERT(buffIdx < FEATURE_CAN_MAX_MB_NUM);

        FLEXCAN_DRV_SetRxMaskType((uint8_t) instance->instIdx,
                                  FLEXCAN_RX_MASK_INDIVIDUAL);
        status = FLEXCAN_DRV_SetRxIndividualMask((uint8_t) instance->instIdx,
                                                 (flexcan_msgbuff_id_type_t) idType,
                                                 (uint8_t) buffIdx,
                                                 mask);
    }
    #endif

    /* Define CAN PAL over MCAN */
    #if defined(CAN_OVER_MCAN)
    if (instance->instType == CAN_INST_TYPE_MCAN)
    {
        DEV_ASSERT(instance->instIdx < M_CAN_INSTANCE_COUNT);

        index = CAN_FindMCANState(instance->instIdx);

        /* If Rx FIFO must be enable  */
        DEV_ASSERT(s_mcanRxFifoState[index].rxFifoEn == true);
        (void) index;

        status = MCAN_DRV_SetRxFifoFilterMask((uint8_t) instance->instIdx,
                                              (mcan_msgbuff_id_type_t) idType,
                                              (uint8_t) buffIdx,
                                              mask);
    }
    #endif
    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_GetTransferStatus
 * Description   : Returns the state of the previous CAN transfer.
 *
 * Implements    : CAN_GetTransferStatus_Activity
 *END**************************************************************************/
status_t CAN_GetTransferStatus(const can_instance_t * const instance,
                               uint32_t buffIdx)
{
    DEV_ASSERT(instance != NULL);

    status_t status = STATUS_ERROR;
    uint8_t index;

    /* Define CAN PAL over FLEXCAN */
    #if defined(CAN_OVER_FLEXCAN)
    if (instance->instType == CAN_INST_TYPE_FLEXCAN)
    {
        /* Check buffer index to avoid overflow */
        DEV_ASSERT(buffIdx < FEATURE_CAN_MAX_MB_NUM);
        DEV_ASSERT(instance->instIdx < CAN_INSTANCE_COUNT);

        index = CAN_FindFlexCANState(instance->instIdx);

        /* Compute virtual buffer index */
        if ((s_flexcanRxFifoState[index].rxFifoEn) && (buffIdx != 0U))
        {
            buffIdx += CAN_GetVirtualBuffIdx(s_flexcanRxFifoState[index].numIdFilters);
        }

        status = FLEXCAN_DRV_GetTransferStatus((uint8_t) instance->instIdx,
                                               (uint8_t) buffIdx);
    }
    #endif

    /* Define CAN PAL over MCAN */
    #if defined(CAN_OVER_MCAN)
    if (instance->instType == CAN_INST_TYPE_MCAN)
    {
        /* Check buffer index to avoid overflow */
        DEV_ASSERT(buffIdx <  (s_rxBuffs + s_txBuffs));
        DEV_ASSERT(instance->instIdx < M_CAN_INSTANCE_COUNT);

        index = CAN_FindMCANState(instance->instIdx);

        if (buffIdx < s_rxBuffs)
        {
            if ((s_mcanRxFifoState[index].rxFifoEn == true) && (buffIdx == 0U))
            {
                status = MCAN_DRV_GetTransferStatus((uint8_t) instance->instIdx,
                                                                MCAN_RX_FIFO0,
                                                               (uint8_t) buffIdx);
            }
            else
            {
                if (s_mcanRxFifoState[index].rxFifoEn == true)
                {
                    /* In case of RxFIFO enabled the buff index is from 1 to Rx Buff No and
                     * the real Message Buffers are from 0 to (Rx Buff - 1) */
                    buffIdx--;
                }

                status = MCAN_DRV_GetTransferStatus((uint8_t) instance->instIdx,
                                                    MCAN_RX_BUFF,
                                                    (uint8_t) buffIdx);
            }
        }
        else
        {
            if (buffIdx < (s_rxBuffs + s_txBuffs))
            {
                status = MCAN_DRV_GetTransferStatus((uint8_t) instance->instIdx,
                                                    MCAN_TX_BUFF,
                                                    (uint8_t) (buffIdx-s_rxBuffs));
            }
        }
    }
    #endif
    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_InstallEventCallback
 * Description   : Installs a callback function for the IRQ handler.
 *
 * Implements    : CAN_InstallEventCallback_Activity
 *END**************************************************************************/
status_t CAN_InstallEventCallback(const can_instance_t * const instance,
                                  can_callback_t callback,
                                  void *callbackParam)
{
    DEV_ASSERT(instance != NULL);

    status_t status = STATUS_ERROR;
    uint8_t index;

    /* Define CAN PAL over FLEXCAN */
    #if defined(CAN_OVER_FLEXCAN)
    if (instance->instType == CAN_INST_TYPE_FLEXCAN)
    {
        if (callback != NULL)
        {
            DEV_ASSERT(instance->instIdx < CAN_INSTANCE_COUNT);

            index = CAN_FindFlexCANState(instance->instIdx);

            /* Save user callback */
            userFlexCallbacks[index] = callback;
            /* Install internal FlexCAN callback */
            FLEXCAN_DRV_InstallEventCallback((uint8_t) instance->instIdx,
                                             CAN_FlexInternalCallback,
                                             callbackParam);
        }
        else
        {
            /* Uninstall internal FlexCAN callback */
            FLEXCAN_DRV_InstallEventCallback((uint8_t) instance->instIdx,
                                             NULL,
                                             callbackParam);
        }

        status = STATUS_SUCCESS;
    }
    #endif

    /* Define CAN PAL over MCAN */
    #if defined(CAN_OVER_MCAN)
    if (instance->instType == CAN_INST_TYPE_MCAN)
    {
        if (callback != NULL)
        {
            DEV_ASSERT(instance->instIdx < M_CAN_INSTANCE_COUNT);

            index = CAN_FindMCANState(instance->instIdx);

            /* Save user callback */
            userMcanCallbacks[index] = callback;
            /* Install internal FlexCAN callback */
            MCAN_DRV_InstallEventCallback((uint8_t) instance->instIdx,
                                          CAN_McanInternalCallback,
                                          callbackParam);
        }
        else
        {
            /* Uninstall internal FlexCAN callback */
            MCAN_DRV_InstallEventCallback((uint8_t) instance->instIdx,
                                          NULL,
                                          callbackParam);
        }

        status = STATUS_SUCCESS;
    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : CAN_GetDefaultConfig
 * Description   : Return Default configuration for CAN Pal component
 *
 * Implements    : CAN_GetDefaultConfig_Activity
 *END**************************************************************************/
 void CAN_GetDefaultConfig(can_instance_t * instance,
                           can_user_config_t * config)
{
     
    /* Define CAN PAL over FLEXCAN */
    #if defined(CAN_OVER_FLEXCAN)
	if (instance->instType == CAN_INST_TYPE_FLEXCAN)
	{
        flexcan_user_config_t flexcanConfig;
        /* Get Default configuration and calculate cbt segments based on clock value */
        FLEXCAN_DRV_GetDefaultConfig(&flexcanConfig);

        instance->instType = CAN_INST_TYPE_FLEXCAN;
        instance->instIdx = 0UL;
        /* Split no of MBs */
        config->txBuffNum = flexcanConfig.max_num_mb>>1U;
        config->rxBuffNum = flexcanConfig.max_num_mb>>1U;
        config->mode = CAN_NORMAL_MODE;
        config->peClkSrc = CAN_CLK_SOURCE_OSC;
        config->enableFD = false;
        config->payloadSize = CAN_PAYLOAD_SIZE_8;

        config->nominalBitrate.phaseSeg1   = flexcanConfig.bitrate.phaseSeg1;
        config->nominalBitrate.phaseSeg2   = flexcanConfig.bitrate.phaseSeg2;
        config->nominalBitrate.preDivider  = flexcanConfig.bitrate.preDivider;
        config->nominalBitrate.propSeg     = flexcanConfig.bitrate.propSeg;
        config->nominalBitrate.rJumpwidth  = flexcanConfig.bitrate.rJumpwidth;

        config->dataBitrate.phaseSeg1  = flexcanConfig.bitrate.phaseSeg1;
        config->dataBitrate.phaseSeg2  = flexcanConfig.bitrate.phaseSeg2;
        config->dataBitrate.preDivider = flexcanConfig.bitrate.preDivider;
        config->dataBitrate.propSeg    = flexcanConfig.bitrate.propSeg;
        config->dataBitrate.rJumpwidth = flexcanConfig.bitrate.rJumpwidth;
	}
    #endif

	/* Define CAN PAL over MCAN */
	#if defined(CAN_OVER_MCAN)
	if (instance->instType == CAN_INST_TYPE_MCAN)
	{

	    mcan_user_config_t mcanConfig;
		/* Get Default configuration and calculate cbt segments based on clock value */
		MCAN_DRV_GetDefaultConfig(&mcanConfig);

		instance->instType = CAN_INST_TYPE_MCAN;
		instance->instIdx = 0UL;
		config->txBuffNum = mcanConfig.tx_num_mb;
		config->rxBuffNum = mcanConfig.rx_num_mb;
		config->mode = CAN_NORMAL_MODE;
		config->peClkSrc = CAN_CLK_SOURCE_OSC;
		config->enableFD = false;
		config->payloadSize = CAN_PAYLOAD_SIZE_8;

		config->nominalBitrate.phaseSeg1   = mcanConfig.bitrate.phaseSeg1;
		config->nominalBitrate.phaseSeg2   = mcanConfig.bitrate.phaseSeg2;
		config->nominalBitrate.preDivider  = mcanConfig.bitrate.preDivider;
		config->nominalBitrate.propSeg     = mcanConfig.bitrate.propSeg;
		config->nominalBitrate.rJumpwidth  = mcanConfig.bitrate.rJumpwidth;

		config->dataBitrate.phaseSeg1  = mcanConfig.bitrate.phaseSeg1;
		config->dataBitrate.phaseSeg2  = mcanConfig.bitrate.phaseSeg2;
		config->dataBitrate.preDivider = mcanConfig.bitrate.preDivider;
		config->dataBitrate.propSeg    = mcanConfig.bitrate.propSeg;
		config->dataBitrate.rJumpwidth = mcanConfig.bitrate.rJumpwidth;
	}
	#endif
    config->extension = NULL;
}
 
/*******************************************************************************
 * EOF
 ******************************************************************************/
