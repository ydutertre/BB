################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lv_examples/src/lv_demo_stress/lv_demo_stress.c 

OBJS += \
./lv_examples/src/lv_demo_stress/lv_demo_stress.o 

C_DEPS += \
./lv_examples/src/lv_demo_stress/lv_demo_stress.d 


# Each subdirectory must supply rules for building sources it contributes
lv_examples/src/lv_demo_stress/%.o: ../lv_examples/src/lv_demo_stress/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/horinek/eclipse-workspace" -O0 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

