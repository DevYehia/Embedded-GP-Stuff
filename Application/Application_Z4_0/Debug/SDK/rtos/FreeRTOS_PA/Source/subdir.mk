################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../SDK/rtos/FreeRTOS_PA/Source/croutine.c \
../SDK/rtos/FreeRTOS_PA/Source/event_groups.c \
../SDK/rtos/FreeRTOS_PA/Source/list.c \
../SDK/rtos/FreeRTOS_PA/Source/queue.c \
../SDK/rtos/FreeRTOS_PA/Source/stream_buffer.c \
../SDK/rtos/FreeRTOS_PA/Source/tasks.c \
../SDK/rtos/FreeRTOS_PA/Source/timers.c 

OBJS += \
./SDK/rtos/FreeRTOS_PA/Source/croutine.o \
./SDK/rtos/FreeRTOS_PA/Source/event_groups.o \
./SDK/rtos/FreeRTOS_PA/Source/list.o \
./SDK/rtos/FreeRTOS_PA/Source/queue.o \
./SDK/rtos/FreeRTOS_PA/Source/stream_buffer.o \
./SDK/rtos/FreeRTOS_PA/Source/tasks.o \
./SDK/rtos/FreeRTOS_PA/Source/timers.o 

C_DEPS += \
./SDK/rtos/FreeRTOS_PA/Source/croutine.d \
./SDK/rtos/FreeRTOS_PA/Source/event_groups.d \
./SDK/rtos/FreeRTOS_PA/Source/list.d \
./SDK/rtos/FreeRTOS_PA/Source/queue.d \
./SDK/rtos/FreeRTOS_PA/Source/stream_buffer.d \
./SDK/rtos/FreeRTOS_PA/Source/tasks.d \
./SDK/rtos/FreeRTOS_PA/Source/timers.d 


# Each subdirectory must supply rules for building sources it contributes
SDK/rtos/FreeRTOS_PA/Source/%.o: ../SDK/rtos/FreeRTOS_PA/Source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@SDK/rtos/FreeRTOS_PA/Source/croutine.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


