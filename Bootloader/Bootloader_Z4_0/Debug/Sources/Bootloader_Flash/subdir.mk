################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/Bootloader_Flash/BootloaderFlash.c \
../Sources/Bootloader_Flash/BootloaderFlashCRCConfig.c 

OBJS += \
./Sources/Bootloader_Flash/BootloaderFlash.o \
./Sources/Bootloader_Flash/BootloaderFlashCRCConfig.o 

C_DEPS += \
./Sources/Bootloader_Flash/BootloaderFlash.d \
./Sources/Bootloader_Flash/BootloaderFlashCRCConfig.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/Bootloader_Flash/%.o: ../Sources/Bootloader_Flash/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@Sources/Bootloader_Flash/BootloaderFlash.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


