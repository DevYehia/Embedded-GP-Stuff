################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../SDK/rtos/FreeRTOS_PA/Source/portable/GCC/PowerPC/portasm.s 

C_SRCS += \
../SDK/rtos/FreeRTOS_PA/Source/portable/GCC/PowerPC/port.c \
../SDK/rtos/FreeRTOS_PA/Source/portable/GCC/PowerPC/porttimer.c 

OBJS += \
./SDK/rtos/FreeRTOS_PA/Source/portable/GCC/PowerPC/port.o \
./SDK/rtos/FreeRTOS_PA/Source/portable/GCC/PowerPC/portasm.o \
./SDK/rtos/FreeRTOS_PA/Source/portable/GCC/PowerPC/porttimer.o 

C_DEPS += \
./SDK/rtos/FreeRTOS_PA/Source/portable/GCC/PowerPC/port.d \
./SDK/rtos/FreeRTOS_PA/Source/portable/GCC/PowerPC/porttimer.d 


# Each subdirectory must supply rules for building sources it contributes
SDK/rtos/FreeRTOS_PA/Source/portable/GCC/PowerPC/%.o: ../SDK/rtos/FreeRTOS_PA/Source/portable/GCC/PowerPC/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@SDK/rtos/FreeRTOS_PA/Source/portable/GCC/PowerPC/port.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

SDK/rtos/FreeRTOS_PA/Source/portable/GCC/PowerPC/%.o: ../SDK/rtos/FreeRTOS_PA/Source/portable/GCC/PowerPC/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS Assembler'
	powerpc-eabivle-gcc "@SDK/rtos/FreeRTOS_PA/Source/portable/GCC/PowerPC/portasm.args" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


