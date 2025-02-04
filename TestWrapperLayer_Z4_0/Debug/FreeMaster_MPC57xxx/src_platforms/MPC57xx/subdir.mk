################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/NXP/S32DS_Power_v2.1/S32DS/software/FreeMASTER_Serial_Communication_Driver_V2_0/src_platforms/MPC57xx/freemaster_MPC57xx.c 

OBJS += \
./FreeMaster_MPC57xxx/src_platforms/MPC57xx/freemaster_MPC57xx.o 

C_DEPS += \
./FreeMaster_MPC57xxx/src_platforms/MPC57xx/freemaster_MPC57xx.d 


# Each subdirectory must supply rules for building sources it contributes
FreeMaster_MPC57xxx/src_platforms/MPC57xx/freemaster_MPC57xx.o: C:/NXP/S32DS_Power_v2.1/S32DS/software/FreeMASTER_Serial_Communication_Driver_V2_0/src_platforms/MPC57xx/freemaster_MPC57xx.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@FreeMaster_MPC57xxx/src_platforms/MPC57xx/freemaster_MPC57xx.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


