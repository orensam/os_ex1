################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../tests/sol/measure.c 

OBJS += \
./tests/sol/measure.o 

C_DEPS += \
./tests/sol/measure.d 


# Each subdirectory must supply rules for building sources it contributes
tests/sol/%.o: ../tests/sol/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


