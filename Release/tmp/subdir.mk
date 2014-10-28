################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../tmp/measure.o \
../tmp/osm.o 

C_SRCS += \
../tmp/measure.c \
../tmp/osm.c 

OBJS += \
./tmp/measure.o \
./tmp/osm.o 

C_DEPS += \
./tmp/measure.d \
./tmp/osm.d 


# Each subdirectory must supply rules for building sources it contributes
tmp/%.o: ../tmp/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


