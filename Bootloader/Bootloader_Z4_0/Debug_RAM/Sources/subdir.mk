################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/Bootloader.c \
../Sources/CanTP.c \
../Sources/ResetWrapper.c \
../Sources/UDS.c \
../Sources/main.c 

OBJS += \
./Sources/Bootloader.o \
./Sources/CanTP.o \
./Sources/ResetWrapper.o \
./Sources/UDS.o \
./Sources/main.o 

C_DEPS += \
./Sources/Bootloader.d \
./Sources/CanTP.d \
./Sources/ResetWrapper.d \
./Sources/UDS.d \
./Sources/main.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/%.o: ../Sources/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	powerpc-eabivle-gcc "@Sources/Bootloader.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


