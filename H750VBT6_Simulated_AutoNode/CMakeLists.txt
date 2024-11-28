CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR cortex-m7)
#SET(CMAKE_VERBOSE_MAKEFILE ON)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    SET(TOOL_DIR "D:/Software_Tools/env-windows-1.5.0/tools/bin/../../tools/gnu_gcc/arm_gcc/mingw")
	set(PYTHON_EXECUTABLE "C:/Users/dj461v/AppData/Local/Programs/Python/Python38/python.exe")
elseif(UNIX)
    SET(TOOL_DIR "/home/dj461v/wkspace/gcc-arm-none-eabi-10.3-2021.10/")
endif()

message(STATUS "TOOL_DIR: ${TOOL_DIR}")

if(WIN32)
	SET(CMAKE_C_COMPILER "${TOOL_DIR}/bin/arm-none-eabi-gcc.exe")
	SET(CMAKE_ASM_COMPILER "${TOOL_DIR}/bin/arm-none-eabi-gcc.exe")
	SET(CMAKE_CXX_COMPILER "${TOOL_DIR}/bin/arm-none-eabi-g++.exe")
	SET(CMAKE_OBJCOPY "${TOOL_DIR}/bin/arm-none-eabi-objcopy.exe")
	SET(CMAKE_SIZE "${TOOL_DIR}/bin/arm-none-eabi-size.exe")
	SET(CMAKE_READ "${TOOL_DIR}/bin/arm-none-eabi-readelf.exe")
elseif(UNIX)
	SET(CMAKE_C_COMPILER "${TOOL_DIR}/bin/arm-none-eabi-gcc")
	SET(CMAKE_ASM_COMPILER "${TOOL_DIR}/bin/arm-none-eabi-gcc")
	SET(CMAKE_CXX_COMPILER "${TOOL_DIR}/bin/arm-none-eabi-g++")
	SET(CMAKE_OBJCOPY "${TOOL_DIR}/bin/arm-none-eabi-objcopy")
	SET(CMAKE_SIZE "${TOOL_DIR}/bin/arm-none-eabi-size")
endif()

SET(CMAKE_C_FLAGS " -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections -Dgcc -O0 -gdwarf-2 -g -std=c99")
SET(CMAKE_ASM_FLAGS " -c -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections -x assembler-with-cpp -Wa,-mimplicit-it=thumb  -gdwarf-2")
SET(CMAKE_C_COMPILER_WORKS TRUE)
SET(CMAKE_CXX_FLAGS " -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections -Dgcc -O0 -gdwarf-2 -g")
SET(CMAKE_CXX_COMPILER_WORKS TRUE)
SET(CMAKE_EXE_LINKER_FLAGS " -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections -Wl,--gc-sections,-Map=rtthread.map,-cref,-u,Reset_Handler -T ${CMAKE_SOURCE_DIR}/linkscripts/STM32H750VBTx/link.lds")

SET(CMAKE_CXX_STANDARD 14)
PROJECT(Simulated-GW-ECU C CXX ASM)

aux_source_directory(applications APP_SRCS)

INCLUDE_DIRECTORIES(
	.
	applications
	drivers
	drivers/include
	drivers/include/config
	libraries/CMSIS/Device/ST/STM32H7xx/Include
	libraries/CMSIS/Include
	libraries/CMSIS/RTOS/Template
	libraries/STM32H7xx_HAL_Driver/Inc
	libraries/STM32H7xx_HAL_Driver/Inc/Legacy
	rt-thread/components/libc/compilers/common/include
	rt-thread/components/libc/compilers/newlib
	rt-thread/components/drivers/include
	rt-thread/components/finsh
	rt-thread/include
	rt-thread/libcpu/arm/common
	rt-thread/libcpu/arm/cortex-m7
	rt-thread/components/libc/posix/io/epoll
	rt-thread/components/libc/posix/io/eventfd
	rt-thread/components/libc/posix/io/poll
	rt-thread/components/libc/posix/ipc
	cubemx
	cubemx/Inc
)

ADD_DEFINITIONS(
	-DRT_USING_LIBC
	-DRT_USING_NEWLIBC
	-DSTM32H750xx
	-DSOC_SERIES_STM32H7
	-DUSE_HAL_DRIVER
	-D_POSIX_C_SOURCE=1
	-D__RTTHREAD__
)

SET(PROJECT_SOURCES
	cubemx/Src/stm32h7xx_hal_msp.c
	rt-thread/components/libc/compilers/common/cctype.c
	rt-thread/components/libc/compilers/common/cstdlib.c
	rt-thread/components/libc/compilers/common/cstring.c
	rt-thread/components/libc/compilers/common/ctime.c
	rt-thread/components/libc/compilers/common/cunistd.c
	rt-thread/components/libc/compilers/common/cwchar.c
	rt-thread/components/libc/compilers/newlib/syscalls.c
	rt-thread/components/drivers/core/device.c
	rt-thread/components/drivers/ipc/completion.c
	rt-thread/components/drivers/ipc/condvar.c
	rt-thread/components/drivers/ipc/dataqueue.c
	rt-thread/components/drivers/ipc/pipe.c
	rt-thread/components/drivers/ipc/ringblk_buf.c
	rt-thread/components/drivers/ipc/ringbuffer.c
	rt-thread/components/drivers/ipc/waitqueue.c
	rt-thread/components/drivers/ipc/workqueue.c
	rt-thread/components/drivers/pin/pin.c
	rt-thread/components/drivers/serial/serial.c
	rt-thread/components/drivers/can/can.c
	rt-thread/components/finsh/shell.c
	rt-thread/components/finsh/msh.c
	rt-thread/components/finsh/msh_parse.c
	rt-thread/components/finsh/cmd.c
	rt-thread/src/clock.c
	rt-thread/src/components.c
	rt-thread/src/idle.c
	rt-thread/src/ipc.c
	rt-thread/src/irq.c
	rt-thread/src/klibc/kstdio.c
	rt-thread/src/klibc/kstring.c
	rt-thread/src/kservice.c
	rt-thread/src/mem.c
	rt-thread/src/mempool.c
	rt-thread/src/object.c
	rt-thread/src/scheduler_comm.c
	rt-thread/src/scheduler_up.c
	rt-thread/src/thread.c
	rt-thread/src/timer.c
	rt-thread/libcpu/arm/common/div0.c
	rt-thread/libcpu/arm/common/showmem.c
	rt-thread/libcpu/arm/cortex-m7/context_gcc.S
	rt-thread/libcpu/arm/cortex-m7/cpu_cache.c
	rt-thread/libcpu/arm/cortex-m7/cpuport.c
	drivers/board.c
	drivers/drv_adc.c
	drivers/drv_clk.c
	drivers/drv_common.c
	drivers/drv_eth.c
	drivers/drv_gpio.c
	drivers/drv_hwtimer.c
	drivers/drv_pwm.c
	drivers/drv_qspi.c
	drivers/drv_rtc.c
	drivers/drv_sdio.c
	drivers/drv_soft_i2c.c
	drivers/drv_spi.c
	drivers/drv_usart.c
	drivers/drv_usbd.c
	drivers/drv_usbh.c
	drivers/drv_wdt.c
	drivers/drv_fdcan.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cec.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_comp.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cryp.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cryp_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dcmi.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dfsdm.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dfsdm_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma2d.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dsi.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dts.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_eth.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_eth_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_fdcan.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gfxmmu.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hash.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hash_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hcd.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hrtim.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2s.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2s_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_irda.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_iwdg.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_jpeg.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_lptim.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdios.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mmc.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mmc_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_nand.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_nor.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_opamp.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_opamp_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ospi.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_otfdec.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pssi.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_qspi.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ramecc.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sai.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sai_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sd.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sd_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sdram.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smartcard.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smartcard_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smbus.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spdifrx.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sram.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_swpmi.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_usart.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_usart_ex.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_wwdg.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usb.c
	libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_fdcan.c
	libraries/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h750xx.S
	libraries/CMSIS/Device/ST/STM32H7xx/Source/Templates/system_stm32h7xx.c
)

LINK_DIRECTORIES(
)

LINK_LIBRARIES(
	c
	m
)

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME}.elf 
				${APP_SRCS} 
				${PROJECT_SOURCES}
)

ADD_CUSTOM_COMMAND(TARGET ${CMAKE_PROJECT_NAME}.elf POST_BUILD
COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.bin COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_PROJECT_NAME}.elf ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.hex COMMAND ${CMAKE_SIZE} ${CMAKE_PROJECT_NAME}.elf COMMAND ${CMAKE_READ} -S ${CMAKE_PROJECT_NAME}.elf)

ADD_CUSTOM_COMMAND(TARGET ${CMAKE_PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_PROJECT_NAME}.elf ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_PROJECT_NAME}.elf ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.bin
)

add_custom_target(flash
	DEPENDS ${CMAKE_PROJECT_NAME}.elf
	COMMAND ${CMAKE_COMMAND} -E echo "Executing flash script..."
	COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}  ${CMAKE_SOURCE_DIR}/pyocd_flash.bat
	COMMENT "Running batch script after build"
)

add_custom_target(debug
	DEPENDS ${CMAKE_PROJECT_NAME}.elf
	# COMMAND ${CMAKE_COMMAND} -E echo "Executing flash debug script..."
	# COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}  ${CMAKE_SOURCE_DIR}/pyocd_flash.bat
	COMMAND ${CMAKE_COMMAND} -E echo "Executing flash debug script..."
	COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}  ${CMAKE_SOURCE_DIR}/pyocd_debug.bat
)

add_custom_target(oflash
	DEPENDS ${CMAKE_PROJECT_NAME}.elf
	COMMAND ${CMAKE_COMMAND} -E echo "Executing flash openocd_flash"
	COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}/openocd-scripts  ${CMAKE_SOURCE_DIR}/openocd-scripts/flash.bat

)

add_custom_target(odebug
	DEPENDS ${CMAKE_PROJECT_NAME}.elf
	COMMAND ${CMAKE_COMMAND} -E echo "Executing flash openocd_debug"
	COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}/openocd-scripts  ${CMAKE_SOURCE_DIR}/openocd-scripts/debug.bat

)
