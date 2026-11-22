CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR cortex-m7)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

SET(TOOL_DIR "" CACHE PATH "GNU Arm Embedded toolchain root directory")

IF(WIN32)
	IF(NOT TOOL_DIR)
		IF(DEFINED ENV{ARM_GCC_ROOT} AND EXISTS "$ENV{ARM_GCC_ROOT}/bin/arm-none-eabi-gcc.exe")
			SET(TOOL_DIR "$ENV{ARM_GCC_ROOT}")
		ELSE()
			FIND_PROGRAM(ARM_NONE_EABI_GCC arm-none-eabi-gcc.exe)
			IF(ARM_NONE_EABI_GCC)
				GET_FILENAME_COMPONENT(ARM_NONE_EABI_BIN_DIR "${ARM_NONE_EABI_GCC}" DIRECTORY)
				GET_FILENAME_COMPONENT(TOOL_DIR "${ARM_NONE_EABI_BIN_DIR}" DIRECTORY)
			ELSEIF(EXISTS "E:/tools/xpack-arm-none-eabi-gcc-10.3.1-2.1/bin/arm-none-eabi-gcc.exe")
				SET(TOOL_DIR "E:/tools/xpack-arm-none-eabi-gcc-10.3.1-2.1")
			ENDIF()
		ENDIF()
	ENDIF()
ELSEIF(UNIX AND NOT TOOL_DIR)
	FIND_PROGRAM(ARM_NONE_EABI_GCC arm-none-eabi-gcc)
	IF(NOT ARM_NONE_EABI_GCC AND EXISTS "/opt/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-gcc")
		SET(ARM_NONE_EABI_GCC "/opt/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-gcc")
	ENDIF()
	IF(ARM_NONE_EABI_GCC)
		GET_FILENAME_COMPONENT(ARM_NONE_EABI_BIN_DIR "${ARM_NONE_EABI_GCC}" DIRECTORY)
		GET_FILENAME_COMPONENT(TOOL_DIR "${ARM_NONE_EABI_BIN_DIR}" DIRECTORY)
	ELSE()
		SET(TOOL_DIR "${CMAKE_CURRENT_LIST_DIR}/../../_tools/gcc-arm-none-eabi-10.3-2021.10")
	ENDIF()
ENDIF()

IF(WIN32)
	SET(CMAKE_C_COMPILER "${TOOL_DIR}/bin/arm-none-eabi-gcc.exe")
	SET(CMAKE_ASM_COMPILER "${TOOL_DIR}/bin/arm-none-eabi-gcc.exe")
	SET(CMAKE_OBJCOPY "${TOOL_DIR}/bin/arm-none-eabi-objcopy.exe")
	SET(CMAKE_SIZE "${TOOL_DIR}/bin/arm-none-eabi-size.exe")
ELSEIF(UNIX)
	SET(CMAKE_C_COMPILER "${TOOL_DIR}/bin/arm-none-eabi-gcc")
	SET(CMAKE_ASM_COMPILER "${TOOL_DIR}/bin/arm-none-eabi-gcc")
	SET(CMAKE_OBJCOPY "${TOOL_DIR}/bin/arm-none-eabi-objcopy")
	SET(CMAKE_SIZE "${TOOL_DIR}/bin/arm-none-eabi-size")
ENDIF()

IF(NOT EXISTS "${CMAKE_C_COMPILER}")
	MESSAGE(FATAL_ERROR "ARM GCC not found. Set TOOL_DIR or ARM_GCC_ROOT to the toolchain root.")
ENDIF()

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
	SET(BOOT_OPTIMIZATION_FLAGS " -O0 -g3")
ELSE()
	SET(BOOT_OPTIMIZATION_FLAGS " -Os -gdwarf-2 -g")
ENDIF()

SET(CMAKE_C_FLAGS " -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections -Dgcc${BOOT_OPTIMIZATION_FLAGS}")
SET(CMAKE_ASM_FLAGS " -c -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections -x assembler-with-cpp -Wa,-mimplicit-it=thumb -gdwarf-2")
SET(CMAKE_C_COMPILER_WORKS TRUE)
SET(CMAKE_EXE_LINKER_FLAGS " -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections --specs=nosys.specs -Wl,--gc-sections,-Map=boot.map,-cref,-u,Reset_Handler -T ${CMAKE_CURRENT_LIST_DIR}/linkscripts/boot.lds")
SET(CMAKE_C_STANDARD 11)

PROJECT(RTThread-Boot C ASM)

SET(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../App")
SET(RT_ROOT "${PROJECT_ROOT}/rt-thread")
SET(CMSIS_ROOT "${PROJECT_ROOT}/libraries/CMSIS")
SET(DEVICE_ROOT "${CMSIS_ROOT}/Device/ST/STM32H7xx")

INCLUDE_DIRECTORIES(
	"${CMAKE_CURRENT_LIST_DIR}/include"
	"${CMAKE_CURRENT_LIST_DIR}/board"
	"${PROJECT_ROOT}"
	"${PROJECT_ROOT}/drivers/include"
	"${DEVICE_ROOT}/Include"
	"${CMSIS_ROOT}/Include"
	"${CMSIS_ROOT}/RTOS/Template"
	"${RT_ROOT}/include"
	"${RT_ROOT}/libcpu/arm/common"
	"${RT_ROOT}/libcpu/arm/cortex-m7"
)

ADD_DEFINITIONS(
	-DSTM32H750xx
	-D__RTTHREAD__
	-D__RT_KERNEL_SOURCE__
)

SET(BOOT_SOURCES
	"${CMAKE_CURRENT_LIST_DIR}/applications/main.c"
	"${CMAKE_CURRENT_LIST_DIR}/board/boot_board.c"
	"${CMAKE_CURRENT_LIST_DIR}/board/boot_qspi.c"
	"${RT_ROOT}/src/clock.c"
	"${RT_ROOT}/src/components.c"
	"${RT_ROOT}/src/cpu_up.c"
	"${RT_ROOT}/src/defunct.c"
	"${RT_ROOT}/src/idle.c"
	"${RT_ROOT}/src/ipc.c"
	"${RT_ROOT}/src/irq.c"
	"${RT_ROOT}/src/klibc/kerrno.c"
	"${RT_ROOT}/src/klibc/kstdio.c"
	"${RT_ROOT}/src/klibc/kstring.c"
	"${RT_ROOT}/src/klibc/rt_vsnprintf_tiny.c"
	"${RT_ROOT}/src/kservice.c"
	"${RT_ROOT}/src/object.c"
	"${RT_ROOT}/src/scheduler_comm.c"
	"${RT_ROOT}/src/scheduler_up.c"
	"${RT_ROOT}/src/thread.c"
	"${RT_ROOT}/src/timer.c"
	"${RT_ROOT}/libcpu/arm/common/div0.c"
	"${RT_ROOT}/libcpu/arm/cortex-m7/context_gcc.S"
	"${RT_ROOT}/libcpu/arm/cortex-m7/cpuport.c"
	"${DEVICE_ROOT}/Source/Templates/gcc/startup_stm32h750xx.S"
	"${DEVICE_ROOT}/Source/Templates/system_stm32h7xx.c"
)

ADD_EXECUTABLE(RTThread-Boot.elf ${BOOT_SOURCES})

LINK_LIBRARIES(c m)

ADD_CUSTOM_COMMAND(TARGET RTThread-Boot.elf POST_BUILD
	COMMAND ${CMAKE_OBJCOPY} -O binary RTThread-Boot.elf RTThread-Boot.bin
	COMMAND ${CMAKE_OBJCOPY} -O ihex RTThread-Boot.elf RTThread-Boot.hex
	COMMAND ${CMAKE_SIZE} RTThread-Boot.elf
	COMMAND ${CMAKE_COMMAND}
		-DIMAGE=${CMAKE_CURRENT_BINARY_DIR}/RTThread-Boot.bin
		-DMAX_SIZE=131072
		-P "${CMAKE_CURRENT_LIST_DIR}/check_size.cmake"
)
