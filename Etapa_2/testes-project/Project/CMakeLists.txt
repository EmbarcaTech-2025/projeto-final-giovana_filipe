cmake_minimum_required(VERSION 3.13)

# Configurar o projeto
project(BioSmartCooler C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Configurar o compilador ARM
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# Configurar o compilador
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

# Flags do compilador
set(CMAKE_C_FLAGS "-mcpu=cortex-m0plus -mthumb -O2 -g")
set(CMAKE_CXX_FLAGS "-mcpu=cortex-m0plus -mthumb -O2 -g")
set(CMAKE_ASM_FLAGS "-mcpu=cortex-m0plus -mthumb")

# Add executable
add_executable(BioSmartCooler
    src/main.c
    src/temperatura.c
    src/luminosidade.c
    src/acelerometro.c
    inc/ssd1306_i2c.c
    inc/bmp280.c
    inc/bmp280_low_level.c
    inc/mpu6050_i2c.c
)

# Add include directories
target_include_directories(BioSmartCooler PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

# Add compile definitions
target_compile_definitions(BioSmartCooler PRIVATE
    PICO_TARGET_NAME=BioSmartCooler
    PICO_PROGRAM_NAME="BioSmartCooler"
    PICO_PROGRAM_VERSION_STRING="1.0"
)

