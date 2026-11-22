CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR cortex-m7)
#SET(CMAKE_VERBOSE_MAKEFILE ON)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

SET(TOOL_DIR "" CACHE PATH "GNU Arm Embedded toolchain root directory")
OPTION(APP_IN_QSPI "Build the RT-Thread application for XIP from QSPI flash" ON)

IF(APP_IN_QSPI)
	SET(APP_LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/linkscripts/STM32H750VBTx/app_qspi.lds")
ELSE()
	SET(APP_LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/linkscripts/STM32H750VBTx/link.lds")
ENDIF()

if(WIN32)
	if(NOT TOOL_DIR)
		if(DEFINED ENV{ARM_GCC_ROOT} AND EXISTS "$ENV{ARM_GCC_ROOT}/bin/arm-none-eabi-gcc.exe")
			SET(TOOL_DIR "$ENV{ARM_GCC_ROOT}")
		else()
			find_program(ARM_NONE_EABI_GCC arm-none-eabi-gcc.exe)
			if(ARM_NONE_EABI_GCC)
				get_filename_component(ARM_NONE_EABI_BIN_DIR "${ARM_NONE_EABI_GCC}" DIRECTORY)
				get_filename_component(TOOL_DIR "${ARM_NONE_EABI_BIN_DIR}" DIRECTORY)
			elseif(EXISTS "E:/tools/xpack-arm-none-eabi-gcc-10.3.1-2.1/bin/arm-none-eabi-gcc.exe")
				SET(TOOL_DIR "E:/tools/xpack-arm-none-eabi-gcc-10.3.1-2.1")
			endif()
		endif()
	endif()
elseif(UNIX AND NOT TOOL_DIR)
	find_program(ARM_NONE_EABI_GCC arm-none-eabi-gcc)
	if(NOT ARM_NONE_EABI_GCC AND EXISTS "/opt/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-gcc")
		SET(ARM_NONE_EABI_GCC "/opt/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-gcc")
	endif()
	if(ARM_NONE_EABI_GCC)
		get_filename_component(ARM_NONE_EABI_BIN_DIR "${ARM_NONE_EABI_GCC}" DIRECTORY)
		get_filename_component(TOOL_DIR "${ARM_NONE_EABI_BIN_DIR}" DIRECTORY)
	else()
		SET(TOOL_DIR "${CMAKE_CURRENT_LIST_DIR}/../_tools/gcc-arm-none-eabi-10.3-2021.10")
	endif()
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

if(NOT EXISTS "${CMAKE_C_COMPILER}")
	message(FATAL_ERROR "ARM GCC not found. Set TOOL_DIR or ARM_GCC_ROOT to the toolchain root.")
endif()

SET(CMAKE_C_FLAGS " -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections -Dgcc -O0 -gdwarf-2 -g")
SET(CMAKE_ASM_FLAGS " -c -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections -x assembler-with-cpp -Wa,-mimplicit-it=thumb  -gdwarf-2")
SET(CMAKE_C_COMPILER_WORKS TRUE)
SET(CMAKE_CXX_FLAGS " -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections -Dgcc -O0 -gdwarf-2 -g")
SET(CMAKE_CXX_COMPILER_WORKS TRUE)
SET(CMAKE_EXE_LINKER_FLAGS " -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections -Wl,--gc-sections,-Map=rtthread.map,-cref,-u,Reset_Handler -T ${APP_LINKER_SCRIPT}")
SET(CMAKE_C_STANDARD 11)
SET(CMAKE_CXX_STANDARD 14)
PROJECT(Simulated-GW-ECU C CXX ASM)

aux_source_directory(applications APP_SRCS)

# # Add Ulog 
# INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/rt-thread/components/utilities/ulog)
# INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/rt-thread/components/utilities/ulog/backend)
# INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/rt-thread/components/utilities/ulog/syslog)
# aux_source_directory(
# 	${PROJECT_SOURCE_DIR}/rt-thread/components/utilities/ulog/
# 	ULOG_SRCS
# )
# aux_source_directory(
# 	${PROJECT_SOURCE_DIR}/rt-thread/components/utilities/ulog/backend
# 	ULOG_SRCS
# )
# aux_source_directory(
# 	${PROJECT_SOURCE_DIR}/rt-thread/components/utilities/ulog/syslog
# 	ULOG_SRCS
# )

# message(STATUS "ULOG_SRCS: ${ULOG_SRCS}")

#Add ISOTP sources
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/applications/isotp/inc)
aux_source_directory(
	${PROJECT_SOURCE_DIR}/applications/isotp/src
	ISOTP_SRCS
)

INCLUDE_DIRECTORIES(
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
	rt-thread/components/dfs/dfs_v1/include
	rt-thread/components/dfs/dfs_v1/filesystems/devfs
	rt-thread/components/dfs/dfs_v1/filesystems/elmfat
	rt-thread/components/finsh
	.
	rt-thread/include
	rt-thread/libcpu/arm/common
	rt-thread/libcpu/arm/cortex-m7
	rt-thread/components/libc/posix/io/epoll
	rt-thread/components/libc/posix/io/eventfd
	rt-thread/components/libc/posix/io/poll
	rt-thread/components/libc/posix/ipc
	rt-thread/components/net/lwip/lwip-2.1.2/src/include
	rt-thread/components/net/lwip/port
	rt-thread/components/utilities/ulog
	rt-thread/components/utilities/ulog/syslog
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
	-D__RT_KERNEL_SOURCE__
)

IF(APP_IN_QSPI)
	ADD_DEFINITIONS(-DVECT_TAB_BASE_ADDRESS=0x90000000UL)
ENDIF()

SET(PROJECT_SOURCES
	applications/cantest.c
	applications/main.c
	applications/isotp_node.c
	rt-thread/components/libc/compilers/common/cctype.c
	rt-thread/components/libc/compilers/common/cstdlib.c
	rt-thread/components/libc/compilers/common/cstring.c
	rt-thread/components/libc/compilers/common/ctime.c
	rt-thread/components/libc/compilers/common/cunistd.c
	rt-thread/components/libc/compilers/common/cwchar.c
	rt-thread/components/libc/compilers/newlib/syscalls.c
	rt-thread/components/drivers/can/dev_can.c
	rt-thread/components/drivers/block/blk.c
	rt-thread/components/drivers/block/blk_dev.c
	rt-thread/components/drivers/block/blk_dfs.c
	rt-thread/components/drivers/block/blk_partition.c
	rt-thread/components/drivers/core/device.c
	rt-thread/components/drivers/ipc/completion_comm.c
	rt-thread/components/drivers/ipc/completion_up.c
	rt-thread/components/drivers/ipc/condvar.c
	rt-thread/components/drivers/ipc/dataqueue.c
	rt-thread/components/drivers/ipc/pipe.c
	rt-thread/components/drivers/ipc/ringblk_buf.c
	rt-thread/components/drivers/ipc/ringbuffer.c
	rt-thread/components/drivers/ipc/waitqueue.c
	rt-thread/components/drivers/ipc/workqueue.c
	rt-thread/components/drivers/pin/dev_pin.c
	rt-thread/components/drivers/serial/dev_serial.c
	rt-thread/components/drivers/sdio/dev_block.c
	rt-thread/components/drivers/sdio/dev_mmc.c
	rt-thread/components/drivers/sdio/dev_mmcsd_core.c
	rt-thread/components/drivers/sdio/dev_sd.c
	rt-thread/components/drivers/sdio/dev_sdio.c
	rt-thread/components/drivers/spi/spi_core.c
	rt-thread/components/drivers/spi/spi_dev.c
	rt-thread/components/dfs/dfs_v1/filesystems/devfs/devfs.c
	rt-thread/components/dfs/dfs_v1/filesystems/elmfat/dfs_elm.c
	rt-thread/components/dfs/dfs_v1/filesystems/elmfat/ff.c
	rt-thread/components/dfs/dfs_v1/filesystems/elmfat/ffunicode.c
	rt-thread/components/dfs/dfs_v1/src/dfs.c
	rt-thread/components/dfs/dfs_v1/src/dfs_file.c
	rt-thread/components/dfs/dfs_v1/src/dfs_fs.c
	rt-thread/components/dfs/dfs_v1/src/dfs_posix.c
	rt-thread/components/finsh/msh_file.c
	rt-thread/components/finsh/cmd.c
	rt-thread/components/finsh/msh.c
	rt-thread/components/finsh/msh_parse.c
	rt-thread/components/finsh/shell.c
	rt-thread/src/clock.c
	rt-thread/src/components.c
	rt-thread/src/cpu_up.c
	rt-thread/src/defunct.c
	rt-thread/src/idle.c
	rt-thread/src/ipc.c
	rt-thread/src/irq.c
	rt-thread/src/klibc/kerrno.c
	rt-thread/src/klibc/kstdio.c
	rt-thread/src/klibc/kstring.c
	rt-thread/src/klibc/rt_vsnprintf_tiny.c
	rt-thread/src/klibc/rt_vsscanf.c
	rt-thread/src/kservice.c
	rt-thread/src/mem.c
	rt-thread/src/mempool.c
	rt-thread/src/object.c
	rt-thread/src/scheduler_comm.c
	rt-thread/src/scheduler_up.c
	rt-thread/src/signal.c
	rt-thread/src/thread.c
	rt-thread/src/timer.c
	rt-thread/libcpu/arm/common/div0.c
	rt-thread/libcpu/arm/common/showmem.c
	rt-thread/libcpu/arm/cortex-m7/context_gcc.S
	rt-thread/libcpu/arm/cortex-m7/cpu_cache.c
	rt-thread/libcpu/arm/cortex-m7/cpuport.c
	rt-thread/components/utilities/ulog/backend/console_be.c
	rt-thread/components/utilities/ulog/syslog/syslog.c
	rt-thread/components/utilities/ulog/ulog.c
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
	cubemx/Src/stm32h7xx_hal_msp.c
	rt-thread/components/net/lwip/port/ethernetif.c
	rt-thread/components/net/lwip/port/sys_arch.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/netif/ethernet.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/ipv4/autoip.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/ipv4/dhcp.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/ipv4/etharp.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/ipv4/icmp.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/ipv4/igmp.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/ipv4/ip4.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/ipv4/ip4_addr.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/ipv4/ip4_frag.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/altcp.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/altcp_alloc.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/altcp_tcp.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/def.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/dns.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/inet_chksum.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/init.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/ip.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/memp.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/netif.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/pbuf.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/raw.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/stats.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/sys.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/tcp.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/tcp_in.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/tcp_out.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/timeouts.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/core/udp.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/api/api_lib.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/api/api_msg.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/api/err.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/api/if_api.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/api/netbuf.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/api/netdb.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/api/netifapi.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/api/sockets.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/api/tcpip.c
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

SET(SCRIPTS_DIR "${CMAKE_CURRENT_LIST_DIR}/../Scripts")

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME}.elf 
				${ISOTP_SRCS}
				${APP_SRCS} 
				${PROJECT_SOURCES}
)

ADD_CUSTOM_COMMAND(TARGET ${CMAKE_PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.bin 
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.hex 
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.axf
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.s19
    COMMAND ${CMAKE_SIZE} ${CMAKE_PROJECT_NAME}.elf
    # COMMAND ${CMAKE_READ} -l -h  ${CMAKE_PROJECT_NAME}.elf
)

add_custom_target(flash
		DEPENDS ${CMAKE_PROJECT_NAME}.elf
		COMMAND ${CMAKE_COMMAND} -E echo "Executing flash script..."
		COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File "${SCRIPTS_DIR}/flash-windows.ps1" -AppOnly
		COMMENT "Running batch script after build"
	)

add_custom_target(reset
		DEPENDS ${CMAKE_PROJECT_NAME}.elf
		COMMAND ${CMAKE_COMMAND} -E echo "Executing reset script..."
		COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}  ${SCRIPTS_DIR}/pyocd_reset.bat
	)

add_custom_target(debug
		DEPENDS ${CMAKE_PROJECT_NAME}.elf
		# COMMAND ${CMAKE_COMMAND} -E echo "Executing flash debug script..."
		# COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}  ${SCRIPTS_DIR}/pyocd_flash.bat
		COMMAND ${CMAKE_COMMAND} -E echo "Executing flash debug script..."
		COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}  ${SCRIPTS_DIR}/pyocd_debug.bat
	)


add_custom_target(oflash
		DEPENDS ${CMAKE_PROJECT_NAME}.elf
		COMMAND ${CMAKE_COMMAND} -E echo "Executing flash openocd_flash"
		COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File "${SCRIPTS_DIR}/flash-windows.ps1" -AppOnly
	)

add_custom_target(odebug
		DEPENDS ${CMAKE_PROJECT_NAME}.elf
		COMMAND ${CMAKE_COMMAND} -E echo "Executing flash openocd_debug"
		COMMAND ${CMAKE_COMMAND} -E chdir ${SCRIPTS_DIR}/openocd-scripts  ${SCRIPTS_DIR}/openocd-scripts/debug.bat
	)

# if(WIN32)
# 	add_custom_target(oflash
# 		DEPENDS ${CMAKE_PROJECT_NAME}.elf
# 		COMMAND ${CMAKE_COMMAND} -E echo "Executing flash openocd_flash"
# 		COMMAND ${CMAKE_COMMAND} -E chdir ${SCRIPTS_DIR}/openocd-scripts  ${SCRIPTS_DIR}/openocd-scripts/flash.bat
# 	)
# else()
#     add_custom_target(oflash
#         DEPENDS ${CMAKE_PROJECT_NAME}.elf
#         COMMAND ${CMAKE_COMMAND} -E echo "Executing flash openocd_flash"
#         COMMAND ${CMAKE_COMMAND} -E chdir ${SCRIPTS_DIR}/openocd-scripts bash ${SCRIPTS_DIR}/openocd-scripts/flash.sh
#     )
# endif()
