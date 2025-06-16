################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/bootloader_utils/bl_signature/bl_hash/sha256.c 

OBJS += \
./Sources/bootloader_utils/bl_signature/bl_hash/sha256.o 

C_DEPS += \
./Sources/bootloader_utils/bl_signature/bl_hash/sha256.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/bootloader_utils/bl_signature/bl_hash/%.o: ../Sources/bootloader_utils/bl_signature/bl_hash/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@Sources/bootloader_utils/bl_signature/bl_hash/sha256.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


