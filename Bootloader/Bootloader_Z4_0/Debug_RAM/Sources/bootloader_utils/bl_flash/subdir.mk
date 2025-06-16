################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/bootloader_utils/bl_flash/BootloaderFlash.c \
../Sources/bootloader_utils/bl_flash/BootloaderFlashCRCConfig.c 

OBJS += \
./Sources/bootloader_utils/bl_flash/BootloaderFlash.o \
./Sources/bootloader_utils/bl_flash/BootloaderFlashCRCConfig.o 

C_DEPS += \
./Sources/bootloader_utils/bl_flash/BootloaderFlash.d \
./Sources/bootloader_utils/bl_flash/BootloaderFlashCRCConfig.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/bootloader_utils/bl_flash/%.o: ../Sources/bootloader_utils/bl_flash/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@Sources/bootloader_utils/bl_flash/BootloaderFlash.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


