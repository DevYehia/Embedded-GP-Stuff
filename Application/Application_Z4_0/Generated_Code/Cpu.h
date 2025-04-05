/* ###################################################################
**     This component module is generated by Processor Expert. Do not modify it.
**     Filename    : Cpu.h
**     Project     : Application_Z4_0
**     Processor   : MPC5748G_324
**     Component   : MPC5748G_324
**     Version     : Component 01.197, Driver 01.00, CPU db: 3.00.000
**     Datasheet   : MPC5748G RM Rev. 6, 10/2017
**     Compiler    : GNU C Compiler
**     Date/Time   : 2025-04-05, 20:48, # CodeGen: 25
**     Abstract    :
**
**     Settings    :
**
**     Contents    :
**         SystemInit            - void SystemInit(void);
**         SystemCoreClockUpdate - void SystemCoreClockUpdate(void);
**         SystemSoftwareReset   - void SystemSoftwareReset(void);
**
**     (c) Freescale Semiconductor, Inc.
**     2004 All Rights Reserved
**
**     Copyright 1997 - 2015 Freescale Semiconductor, Inc. 
**     Copyright 2016-2017 NXP 
**     All Rights Reserved.
**     
**     THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
**     IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
**     OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
**     IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
**     INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
**     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
**     SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
**     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
**     STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
**     IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
**     THE POSSIBILITY OF SUCH DAMAGE.
** ###################################################################*/
/*!
** @file Cpu.h
** @version 01.00
** @brief
**
*/         
/*!
**  @addtogroup Cpu_module Cpu module documentation
**  @{
*/         

#ifndef Cpu_H
#define Cpu_H

/* MODULE Cpu. */

/*Include shared modules, which are used for whole project*/
#include "device_registers.h"
#include "interrupt_manager.h"
#include "clock.h"
#include "edma_driver.h"
#include "osif.h"
#include "can_pal.h"
#include "pit_driver.h"
#include "system_MPC5748G.h"

/* Including needed modules to compile this module/procedure */
#include "pin_mux.h"
#include "clockMan1.h"
#include "dmaController1.h"
#include "FreeRTOS.h"
#include "can_pal1.h"
#include "pit1.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Cpu configuration will be declared here. */

#ifdef __cplusplus
}
#endif

/* END Cpu. */

#endif
/* Cpu_H */

/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.1 [05.21]
**     for the NXP C55 series of microcontrollers.
**
** ###################################################################
*/
