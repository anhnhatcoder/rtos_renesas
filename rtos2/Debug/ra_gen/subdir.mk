################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ra_gen/Sensor_Thread.c \
../ra_gen/Uart_Thread.c \
../ra_gen/common_data.c \
../ra_gen/hal_data.c \
../ra_gen/main.c \
../ra_gen/pin_data.c \
../ra_gen/vector_data.c 

C_DEPS += \
./ra_gen/Sensor_Thread.d \
./ra_gen/Uart_Thread.d \
./ra_gen/common_data.d \
./ra_gen/hal_data.d \
./ra_gen/main.d \
./ra_gen/pin_data.d \
./ra_gen/vector_data.d 

CREF += \
rtos2.cref 

OBJS += \
./ra_gen/Sensor_Thread.o \
./ra_gen/Uart_Thread.o \
./ra_gen/common_data.o \
./ra_gen/hal_data.o \
./ra_gen/main.o \
./ra_gen/pin_data.o \
./ra_gen/vector_data.o 

MAP += \
rtos2.map 


# Each subdirectory must supply rules for building sources it contributes
ra_gen/%.o: ../ra_gen/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Os -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0 -funsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Waggregate-return -Wno-parentheses-equality -Wfloat-equal -g3 -std=c99 -fshort-enums -fno-unroll-loops -I"C:\\Users\\admin\\e2_studio\\workspace\\rtos2\\ra_gen" -I"." -I"C:\\Users\\admin\\e2_studio\\workspace\\rtos2\\ra_cfg\\fsp_cfg\\bsp" -I"C:\\Users\\admin\\e2_studio\\workspace\\rtos2\\ra_cfg\\fsp_cfg" -I"C:\\Users\\admin\\e2_studio\\workspace\\rtos2\\ra_cfg\\aws" -I"C:\\Users\\admin\\e2_studio\\workspace\\rtos2\\src" -I"C:\\Users\\admin\\e2_studio\\workspace\\rtos2\\ra\\fsp\\inc" -I"C:\\Users\\admin\\e2_studio\\workspace\\rtos2\\ra\\fsp\\inc\\api" -I"C:\\Users\\admin\\e2_studio\\workspace\\rtos2\\ra\\fsp\\inc\\instances" -I"C:\\Users\\admin\\e2_studio\\workspace\\rtos2\\ra\\fsp\\src\\rm_freertos_port" -I"C:\\Users\\admin\\e2_studio\\workspace\\rtos2\\ra\\aws\\FreeRTOS\\FreeRTOS\\Source\\include" -I"C:\\Users\\admin\\e2_studio\\workspace\\rtos2\\ra\\arm\\CMSIS_6\\CMSIS\\Core\\Include" -D_RENESAS_RA_ -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -x c "$<" -c -o "$@")
	@clang --target=arm-none-eabi @"$@.in"

