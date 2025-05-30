/* ###################################################################
**     This component module is generated by Processor Expert. Do not modify it.
**     Filename    : FreeRTOS.h
**     Project     : Bootloader_Z4_0
**     Processor   : MPC5748G_324
**     Component   : FreeRTOS
**     Version     : Component SDK_S32_PA_11, Driver 01.00, CPU db: 3.00.000
**     Repository  : SDK_S32_PA_11
**     Compiler    : GNU C Compiler
**     Date/Time   : 2025-02-19, 17:04, # CodeGen: 4
**     Contents    :
**         xCoRoutineCreate               - BaseType_t xCoRoutineCreate(crCOROUTINE_CODE pxCoRoutineCode,UBaseType_t...
**         crDELAY                        - void crDELAY(CoRoutineHandle_t xHandle ,TickType_t xTicksToDelay);
**         crQUEUE_SEND                   - void crQUEUE_SEND(CoRoutineHandle_t xHandle, QueueHandle_t xQueue, void...
**         crQUEUE_RECEIVE                - void crQUEUE_RECEIVE(CoRoutineHandle_t xHandle, QueueHandle_t xQueue, void...
**         crQUEUE_SEND_FROM_ISR          - BaseType_t crQUEUE_SEND_FROM_ISR(QueueHandle_t xQueue, void * pvItemToQueue,...
**         crQUEUE_RECEIVE_FROM_ISR       - BaseType_t crQUEUE_RECEIVE_FROM_ISR(QueueHandle_t xQueue, void * pvBuffer,...
**         vCoRoutineSchedule             - void vCoRoutineSchedule(void);
**         xEventGroupCreate              - EventGroupHandle_t xEventGroupCreate(void);
**         xEventGroupClearBits           - EventBits_t xEventGroupClearBits(EventGroupHandle_t xEventGroup,const...
**         xEventGroupClearBitsFromISR    - EventBits_t xEventGroupClearBitsFromISR(EventGroupHandle_t xEventGroup,const...
**         vEventGroupDelete              - void vEventGroupDelete(EventGroupHandle_t xEventGroup);
**         xEventGroupGetBits             - EventBits_t xEventGroupGetBits(EventGroupHandle_t xEventGroup);
**         xEventGroupGetBitsFromISR      - EventBits_t xEventGroupGetBitsFromISR(EventGroupHandle_t xEventGroup);
**         xEventGroupSetBits             - EventBits_t xEventGroupSetBits(EventGroupHandle_t xEventGroup, const...
**         xEventGroupSetBitsFromISR      - BaseType_t xEventGroupSetBitsFromISR(EventGroupHandle_t xEventGroup, const...
**         xEventGroupSync                - EventBits_t xEventGroupSync(EventGroupHandle_t xEventGroup, const EventBits_t...
**         xEventGroupWaitBits            - EventBits_t xEventGroupWaitBits(EventGroupHandle_t xEventGroup,const...
**         vQueueAddToRegistry            - void vQueueAddToRegistry(QueueHandle_t xQueue,const char * pcName);
**         xQueueAddToSet                 - BaseType_t xQueueAddToSet(QueueSetMemberHandle_t...
**         xQueueCreate                   - QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize);
**         xQueueCreateSet                - QueueSetHandle_t xQueueCreateSet(const UBaseType_t uxEventQueueLength);
**         vQueueDelete                   - void vQueueDelete(QueueHandle_t xQueue);
**         xQueueIsQueueEmptyFromISR      - BaseType_t xQueueIsQueueEmptyFromISR(const QueueHandle_t xQueue);
**         xQueueIsQueueFullFromISR       - BaseType_t xQueueIsQueueFullFromISR(const QueueHandle_t xQueue);
**         uxQueueMessagesWaiting         - UBaseType_t uxQueueMessagesWaiting(const QueueHandle_t xQueue);
**         uxQueueMessagesWaitingFromISR  - BaseType_t uxQueueMessagesWaitingFromISR(QueueHandle_t xQueue, const void *...
**         xQueueOverwriteFromISR         - BaseType_t xQueueOverwriteFromISR(QueueHandle_t xQueue, const void *...
**         xQueuePeek                     - BaseType_t xQueuePeek(QueueHandle_t xQueue,void *pvBuffer, TickType_t...
**         xQueuePeekFromISR              - BaseType_t xQueuePeekFromISR(QueueHandle_t xQueue,void *pvBuffer);
**         xQueueReceive                  - BaseType_t xQueueReceive(QueueHandle_t xQueue,void *pvBuffer,TickType_t...
**         xQueueReceiveFromISR           - BaseType_t xQueueReceiveFromISR(QueueHandle_t xQueue,void *const...
**         xQueueRemoveFromSet            - BaseType_t xQueueRemoveFromSet(QueueSetMemberHandle_t...
**         xQueueReset                    - BaseType_t xQueueReset(QueueHandle_t xQueue);
**         xQueueSelectFromSet            - QueueSetMemberHandle_t xQueueSelectFromSet(QueueSetHandle_t xQueueSet,const...
**         xQueueSelectFromSetFromISR     - QueueSetMemberHandle_t xQueueSelectFromSetFromISR(QueueSetHandle_t xQueueSet);
**         xQueueSend                     - BaseType_t xQueueSend(QueueHandle_t xQueue, const void * pvItemToQueue,...
**         xQueueSendToFront              - BaseType_t xQueueSendToFront(QueueHandle_t xQueue, const void *...
**         xQueueSendToBack               - BaseType_t xQueueSendToBack(QueueHandle_t xQueue, const void * pvItemToQueue,...
**         xQueueSendFromISR              - BaseType_t xQueueSendFromISR(QueueHandle_t xQueue, const void *...
**         xQueueSendToBackFromISR        - BaseType_t xQueueSendToBackFromISR(QueueHandle_t xQueue, const void *...
**         xQueueSendToFrontFromISR       - BaseType_t xQueueSendToFrontFromISR(QueueHandle_t xQueue, const void *...
**         uxQueueSpacesAvailable         - UBaseType_t uxQueueSpacesAvailable(const QueueHandle_t xQueue);
**         vSemaphoreCreateBinary         - void vSemaphoreCreateBinary(SemaphoreHandle_t xSemaphore);
**         xSemaphoreCreateBinary         - SemaphoreHandle_t xSemaphoreCreateBinary(void);
**         xSemaphoreCreateCounting       - SemaphoreHandle_t xSemaphoreCreateCounting(UBaseType_t uxMaxCount,...
**         xSemaphoreCreateMutex          - SemaphoreHandle_t xSemaphoreCreateMutex(void);
**         xSemaphoreCreateRecursiveMutex - SemaphoreHandle_t xSemaphoreCreateRecursiveMutex(void);
**         vSemaphoreDelete               - void vSemaphoreDelete(SemaphoreHandle_t xSemaphore);
**         xSemaphoreGetMutexHolder       - TaskHandle_t xSemaphoreGetMutexHolder(SemaphoreHandle_t xMutex);
**         xSemaphoreGive                 - BaseType_t xSemaphoreGive(SemaphoreHandle_t xSemaphore);
**         xSemaphoreGiveFromISR          - BaseType_t xSemaphoreGiveFromISR(SemaphoreHandle_t xSemaphore, signed...
**         xSemaphoreGiveRecursive        - BaseType_t xSemaphoreGiveRecursive(SemaphoreHandle_t xMutex);
**         xSemaphoreTake                 - BaseType_t xSemaphoreTake(SemaphoreHandle_t xSemaphore, TickType_t...
**         xSemaphoreTakeFromISR          - BaseType_t xSemaphoreTakeFromISR(SemaphoreHandle_t xSemaphore, signed...
**         xSemaphoreTakeRecursive        - BaseType_t xSemaphoreTakeRecursive(SemaphoreHandle_t xMutex, TickType_t...
**         xTimerPendFunctionCall         - BaseType_t xTimerPendFunctionCall(PendedFunction_t xFunctionToPend,void *...
**         xTimerPendFunctionCallFromISR  - BaseType_t xTimerPendFunctionCallFromISR(PendedFunction_t...
**         portSWITCH_TO_USER_MODE        - void portSWITCH_TO_USER_MODE(void);
**         vTaskAllocateMPURegions        - void vTaskAllocateMPURegions(TaskHandle_t xTaskToModify, const MemoryRegion_t...
**         xTaskCreate                    - BaseType_t xTaskCreate(TaskFunction_t pvTaskCode, const char * const pcName,...
**         xTaskCreateRestricted          - BaseType_t xTaskCreateRestricted(TaskParameters_t *pxTaskDefinition,...
**         vTaskDelay                     - void vTaskDelay(const TickType_t xTicksToDelay);
**         vTaskDelayUntil                - void vTaskDelayUntil(TickType_t *const pxPreviousWakeTime,const TickType_t...
**         vTaskDelete                    - void vTaskDelete(TaskHandle_t xTaskToDelete);
**         taskDISABLE_INTERRUPTS         - void taskDISABLE_INTERRUPTS(void);
**         taskENABLE_INTERRUPTS          - void taskENABLE_INTERRUPTS(void);
**         taskENTER_CRITICAL             - void taskENTER_CRITICAL(void);
**         taskEXIT_CRITICAL              - void taskEXIT_CRITICAL(void);
**         xTaskGetApplicationTaskTag     - TaskHookFunction_t xTaskGetApplicationTaskTag(TaskHandle_t xTask);
**         xTaskGetCurrentTaskHandle      - TaskHandle_t xTaskGetCurrentTaskHandle();
**         uxTaskGetNumberOfTasks         - UBaseType_t uxTaskGetNumberOfTasks();
**         vTaskGetRunTimeStats           - void vTaskGetRunTimeStats(char * pcWriteBuffer,size_t bufSize);
**         xTaskGetSchedulerState         - BaseType_t xTaskGetSchedulerState();
**         uxTaskGetStackHighWaterMark    - UBaseType_t uxTaskGetStackHighWaterMark(TaskHandle_t xTask);
**         uxTaskGetSystemState           - UBaseType_t uxTaskGetSystemState(TaskStatus_t *const pxTaskStatusArray,const...
**         xTaskGetTickCount              - TickType_t xTaskGetTickCount();
**         xTaskGetTickCountFromISR       - TickType_t xTaskGetTickCountFromISR();
**         vTaskList                      - void vTaskList(char * pcWriteBuffer,size_t bufSize);
**         uxTaskPriorityGet              - UBaseType_t uxTaskPriorityGet(TaskHandle_t xTask);
**         vTaskPrioritySet               - void vTaskPrioritySet(TaskHandle_t xTask,UBaseType_t uxNewPriority);
**         vTaskResume                    - void vTaskResume(TaskHandle_t xTaskToResume);
**         xTaskResumeAll                 - BaseType_t xTaskResumeAll();
**         xTaskResumeFromISR             - BaseType_t xTaskResumeFromISR(TaskHandle_t xTaskToResume);
**         vTaskStepTick                  - void vTaskStepTick(const TickType_t xTicksToJump);
**         vTaskSuspend                   - void vTaskSuspend(TaskHandle_t xTaskToSuspend);
**         vTaskSuspendAll                - void vTaskSuspendAll();
**         taskYIELD                      - void taskYIELD();
**         vTaskStartScheduler            - void vTaskStartScheduler();
**         vTaskEndScheduler              - void vTaskEndScheduler();
**         xTaskCallApplicationTaskHook   - BaseType_t xTaskCallApplicationTaskHook(TaskHandle_t xTask,void * pvParameter);
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
** @file FreeRTOS.h
** @version 01.00
*/         
/*!
**  @addtogroup FreeRTOS_module FreeRTOS module documentation
**  @{
*/         
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/**
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Directive 4.9, Function-like macro defined.
 * The macro is used for development validation.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 3.1, C comment contains C++ comment.
 * Detections are URL links from FreeRTOS header text.
 *
 */

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION                     1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION  1
#define configCPU_CLOCK_HZ                       ( 40000000UL )
#define configTICK_RATE_HZ                       ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES                     ( 8 )
#define configMINIMAL_STACK_SIZE                 ( ( unsigned short ) 200 )
#define configMAX_TASK_NAME_LEN                  ( 12 )
#define configUSE_16_BIT_TICKS                   0
#define configIDLE_SHOULD_YIELD                  1
#define configUSE_TASK_NOTIFICATIONS             1
#define configUSE_MUTEXES                        1
#define configUSE_RECURSIVE_MUTEXES              1
#define configUSE_COUNTING_SEMAPHORES            1
#define configQUEUE_REGISTRY_SIZE                0
#define configUSE_QUEUE_SETS                     0
#define configUSE_TIME_SLICING                   1
#define configUSE_NEWLIB_REENTRANT               0
#define configENABLE_BACKWARD_COMPATIBILITY      1
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS  0
#define configUSE_APPLICATION_TASK_TAG           0

/* Memory allocation related definitions. */
#define configSUPPORT_STATIC_ALLOCATION          0
#define configSUPPORT_DYNAMIC_ALLOCATION         1
#define configTOTAL_HEAP_SIZE                    ( ( size_t ) 30192 )
#define configAPPLICATION_ALLOCATED_HEAP         0

/* Hook function related definitions. */
#define configUSE_IDLE_HOOK                      0
#define configUSE_TICK_HOOK                      0
#define configCHECK_FOR_STACK_OVERFLOW           0
#define configUSE_MALLOC_FAILED_HOOK             0
#define configUSE_DAEMON_TASK_STARTUP_HOOK       0

/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS            0 
#define configUSE_TRACE_FACILITY                 0
#define configUSE_STATS_FORMATTING_FUNCTIONS     0

/* Co-routine related definitions. */
#define configUSE_CO_ROUTINES                    0
#define configMAX_CO_ROUTINE_PRIORITIES          ( 2 )

/* Software timer related definitions. */
#define configUSE_TIMERS                         0

/* Optional functions - most linkers will remove unused functions anyway. */
#define INCLUDE_vTaskPrioritySet                 1
#define INCLUDE_uxTaskPriorityGet                1
#define INCLUDE_vTaskDelete                      1
#define INCLUDE_vTaskSuspend                     1
#define INCLUDE_vTaskDelayUntil                  1
#define INCLUDE_vTaskDelay                       1
#define INCLUDE_xTaskGetSchedulerState           1
#define INCLUDE_xTaskGetCurrentTaskHandle        1
#define INCLUDE_uxTaskGetStackHighWaterMark      1
#define INCLUDE_xTaskGetIdleTaskHandle           0
#define INCLUDE_eTaskGetState                    0
#define INCLUDE_xEventGroupSetBitFromISR         1
#define INCLUDE_xTimerPendFunctionCall           0
#define INCLUDE_xTaskAbortDelay                  1
#define INCLUDE_xTaskGetHandle                   1
#define INCLUDE_xTaskResumeFromISR               1
#define INCLUDE_xQueueGetMutexHolder             1

/* Definition assert() function. */
#define configASSERT(x)                          if((x)==0) { taskDISABLE_INTERRUPTS(); for( ;; ); }   

/* Tickless Idle Mode */
#define configUSE_TICKLESS_IDLE                  0

/* Additional settings can be defined in the property Settings > User settings > Definitions of the FreeRTOS component */

/* Run time stats gathering definitions. */
#if defined(__GNUC__) && !defined(__ASSEMBLER__)
	/* The #ifdef just prevents this C specific syntax from being included in
	assembly files. */
	void vMainConfigureTimerForRunTimeStats( void );
	unsigned long ulMainGetRunTimeCounterValue( void );
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY  (0x00)

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY	 (0x0F)

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#ifndef configKERNEL_INTERRUPT_PRIORITY
  #define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8-configPRIO_BITS))  
#endif
  
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#ifndef configMAX_SYSCALL_INTERRUPT_PRIORITY
  #define configMAX_SYSCALL_INTERRUPT_PRIORITY    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8-configPRIO_BITS))
#endif

/* PowerPC specific: pit channel to use 0-15 */
#define configUSE_PIT_CHANNEL                    (3)

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configMAX_API_CALL_INTERRUPT_PRIORITY    (15)


#endif /* FREERTOS_CONFIG_H */

