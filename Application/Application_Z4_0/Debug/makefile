################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Sources/subdir.mk
-include SDK/rtos/osif/subdir.mk
-include SDK/rtos/FreeRTOS_PA/Source/portable/MemMang/subdir.mk
-include SDK/rtos/FreeRTOS_PA/Source/portable/GCC/PowerPC/subdir.mk
-include SDK/rtos/FreeRTOS_PA/Source/subdir.mk
-include SDK/platform/pal/src/can/subdir.mk
-include SDK/platform/drivers/src/pit/subdir.mk
-include SDK/platform/drivers/src/pins/siul2/subdir.mk
-include SDK/platform/drivers/src/interrupt/subdir.mk
-include SDK/platform/drivers/src/flexcan/subdir.mk
-include SDK/platform/drivers/src/edma/subdir.mk
-include SDK/platform/drivers/src/clock/MPC57xx/subdir.mk
-include SDK/platform/devices/MPC5748G/startup/subdir.mk
-include SDK/platform/devices/subdir.mk
-include Project_Settings/Startup_Code/subdir.mk
-include Project_Settings/Linker_Files/subdir.mk
-include Generated_Code/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
Application_Z4_0.srec \

SECONDARY_SIZE += \
Application_Z4_0.siz \


# All Target
all: Application_Z4_0.elf secondary-outputs

# Tool invocations
Application_Z4_0.elf: $(OBJS) C:/Users/user/Desktop/nxp_board_code/Application/Application_Z4_0/Project_Settings/Linker_Files/linker_flash.ld $(OSPT) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Standard S32DS C Linker'
	powerpc-eabivle-gcc -o "Application_Z4_0.elf" "@Application_Z4_0.args"  $(USER_OBJS) $(LIBS) $(OSPT)
	@echo 'Finished building target: $@'
	@echo ' '

Application_Z4_0.srec: Application_Z4_0.elf
	@echo 'Invoking: Standard S32DS Create Flash Image'
	powerpc-eabivle-objcopy -O srec Application_Z4_0.elf  "Application_Z4_0.srec"
	@echo 'Finished building: $@'
	@echo ' '

Application_Z4_0.siz: Application_Z4_0.elf
	@echo 'Invoking: Standard S32DS Print Size'
	powerpc-eabivle-size --format=sysv Application_Z4_0.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) ./*
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
