################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/bootloader_utils/bl_decompress/bl_decompress.c \
../Sources/bootloader_utils/bl_decompress/bl_decompress_pbcfg.c \
../Sources/bootloader_utils/bl_decompress/lz4.c 

OBJS += \
./Sources/bootloader_utils/bl_decompress/bl_decompress.o \
./Sources/bootloader_utils/bl_decompress/bl_decompress_pbcfg.o \
./Sources/bootloader_utils/bl_decompress/lz4.o 

C_DEPS += \
./Sources/bootloader_utils/bl_decompress/bl_decompress.d \
./Sources/bootloader_utils/bl_decompress/bl_decompress_pbcfg.d \
./Sources/bootloader_utils/bl_decompress/lz4.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/bootloader_utils/bl_decompress/%.o: ../Sources/bootloader_utils/bl_decompress/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@Sources/bootloader_utils/bl_decompress/bl_decompress.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


