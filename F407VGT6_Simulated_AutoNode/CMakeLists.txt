CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR cortex-m4)
# SET(CMAKE_VERBOSE_MAKEFILE ON)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(BAREMETAL_ARM_TOOLCHAIN_PATH   "D:/Software_Tools/env-windows-1.5.0/tools/bin/../../tools/gnu_gcc/arm_gcc/mingw/bin")
SET(CMAKE_C_COMPILER "${BAREMETAL_ARM_TOOLCHAIN_PATH}/arm-none-eabi-gcc.exe")
SET(CMAKE_ASM_COMPILER "${BAREMETAL_ARM_TOOLCHAIN_PATH}/arm-none-eabi-gcc.exe")
SET(CMAKE_C_FLAGS " -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections -Dgcc -O0 -gdwarf-2 -g")
SET(CMAKE_ASM_FLAGS " -c -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections -x assembler-with-cpp -Wa,-mimplicit-it=thumb  -gdwarf-2")
SET(CMAKE_C_COMPILER_WORKS TRUE)

SET(CMAKE_CXX_COMPILER "${BAREMETAL_ARM_TOOLCHAIN_PATH}/arm-none-eabi-g++.exe")
SET(CMAKE_CXX_FLAGS " -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections -Dgcc -O0 -gdwarf-2 -g")
SET(CMAKE_CXX_COMPILER_WORKS TRUE)

SET(CMAKE_OBJCOPY "${BAREMETAL_ARM_TOOLCHAIN_PATH}/arm-none-eabi-objcopy.exe")
SET(CMAKE_SIZE "${BAREMETAL_ARM_TOOLCHAIN_PATH}/arm-none-eabi-size.exe")

SET(CMAKE_EXE_LINKER_FLAGS " -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections -Wl,--gc-sections,-Map=rtthread.map,-cref,-u,Reset_Handler -T ${CMAKE_SOURCE_DIR}/linkscripts/STM32F407VE/link.lds")

SET(PYOCD_PATH "${CMAKE_SOURCE_DIR}/_tools/PyOCD/0.1.3")
set(CMAKE_C_STANDARD 11)
SET(CMAKE_CXX_STANDARD 11)
PROJECT(rtthread C CXX ASM)

macro(timestamp _result)
    if(NOT DEFINED _timestamp_${_result})
        file(TIMESTAMP ${CMAKE_CURRENT_SOURCE_DIR} _timestamp_${_result})
    endif()
    set(${_result} "${_timestamp_${_result}}" PARENT_SCOPE)
endmacro()

# For Application management
aux_source_directory(applications APP_SRCS)
aux_source_directory(applications/network APP_SRCS)
aux_source_directory(applications/doip APP_SRCS)
INCLUDE_DIRECTORIES(
    applications
    applications/network
	applications/doip
)

#FAL
SET(RT_FAL_SOURCES
	rt-thread/components/fal/src/fal_flash.c
	rt-thread/components/fal/src/fal_partition.c
	rt-thread/components/fal/src/fal_rtt.c
	rt-thread/components/fal/samples/porting/fal_flash_sfud_port.c
	rt-thread/components/fal/src/fal.c
)
INCLUDE_DIRECTORIES(
	rt-thread/components/fal/inc
	rt-thread/components/fal/samples/porting
)


INCLUDE_DIRECTORIES(
	.
	applications
	cubemx
	cubemx/Inc
	drivers
	drivers/include
	drivers/include/config
	libraries/CMSIS/Device/ST/STM32F4xx/Include
	libraries/CMSIS/Include
	libraries/CMSIS/RTOS/Template
	libraries/STM32F4xx_HAL_Driver/Inc
	libraries/STM32F4xx_HAL_Driver/Inc/Legacy
	packages
	rt-thread/components/dfs/filesystems/devfs
	rt-thread/components/dfs/filesystems/elmfs
	rt-thread/components/dfs/include
	rt-thread/components/drivers/include
	rt-thread/components/drivers/spi
	rt-thread/components/drivers/spi/sfud/inc
	rt-thread/components/finsh
	rt-thread/components/libc/compilers/common/include
	rt-thread/components/libc/compilers/newlib
	# rt-thread/components/libc/cplusplus
	# rt-thread/components/libc/cpp11/gcc
	rt-thread/components/libc/posix/delay
	rt-thread/components/libc/posix/io/poll
	rt-thread/components/libc/posix/io/stdio
	rt-thread/components/libc/posix/ipc
	rt-thread/components/libc/posix/pthreads
	rt-thread/components/net/lwip/lwip-2.1.2/src/include
	rt-thread/components/net/lwip/lwip-2.1.2/src/include/netif
	rt-thread/components/net/lwip/lwip-2.1.2/src/include/compat/posix/net
	rt-thread/components/net/lwip/port
	rt-thread/components/net/netdev/include
	rt-thread/components/net/sal/impl
	rt-thread/components/net/sal/include
	rt-thread/components/net/sal/include/dfs_net
	rt-thread/components/net/sal/include/socket
	rt-thread/components/net/sal/include/socket/sys_socket
	rt-thread/components/utilities/ulog
	rt-thread/include
	rt-thread/libcpu/aem/common
	rt-thread/libcpu/aem/cortex-m4

)

ADD_DEFINITIONS(
	-DRT_USING_LIBC
	-DRT_USING_NEWLIBC
	-DSTM32F407xx
	-DUSE_HAL_DRIVER
	-D_POSIX_C_SOURCE=1
	-D__RTTHREAD__
	-DSOC_SERIES_STM32F4
)

SET(PROJECT_SOURCES
	# applications/main.c
	# applications/mnt.c
	# applications/readspeed.c
	# applications/spi_device.c
	rt-thread/libcpu/arm/cortex-m4/context_gcc.S
	rt-thread/libcpu/arm/cortex-m4/cpuport.c
	rt-thread/libcpu/arm/common/atomic_arm.c
	rt-thread/libcpu/arm/common/div0.c
	rt-thread/libcpu/arm/common/showmem.c
	rt-thread/components/net/sal/src/sal_socket.c
	rt-thread/components/net/sal/socket/net_netdb.c
	rt-thread/components/net/sal/socket/net_sockets.c
	rt-thread/components/net/sal/impl/af_inet_lwip.c
	rt-thread/components/net/sal/dfs_net/dfs_net.c
	rt-thread/components/net/netdev/src/netdev.c
	rt-thread/components/net/netdev/src/netdev_ipaddr.c
	rt-thread/components/net/lwip/port/ethernetif.c
	rt-thread/components/net/lwip/port/sys_arch.c
	rt-thread/components/net/lwip/lwip-2.1.2/src/netif/ethernet.c
rt-thread/components/net/lwip/lwip-2.1.2/src/netif/lowpan6.c
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
rt-thread/components/net/lwip/lwip-2.1.2/src/apps/ping/ping.c
rt-thread/components/net/lwip/lwip-2.1.2/src/api/api_lib.c
rt-thread/components/net/lwip/lwip-2.1.2/src/api/api_msg.c
rt-thread/components/net/lwip/lwip-2.1.2/src/api/err.c
rt-thread/components/net/lwip/lwip-2.1.2/src/api/if_api.c
rt-thread/components/net/lwip/lwip-2.1.2/src/api/netbuf.c
rt-thread/components/net/lwip/lwip-2.1.2/src/api/netdb.c
rt-thread/components/net/lwip/lwip-2.1.2/src/api/netifapi.c
rt-thread/components/net/lwip/lwip-2.1.2/src/api/sockets.c
rt-thread/components/net/lwip/lwip-2.1.2/src/api/tcpip.c
	rt-thread/components/libc/posix/pthreads/pthread.c
	rt-thread/components/libc/posix/pthreads/pthread_attr.c
	rt-thread/components/libc/posix/pthreads/pthread_barrier.c
	rt-thread/components/libc/posix/pthreads/pthread_cond.c
	rt-thread/components/libc/posix/pthreads/pthread_mutex.c
	rt-thread/components/libc/posix/pthreads/pthread_rwlock.c
	rt-thread/components/libc/posix/pthreads/pthread_spin.c
	rt-thread/components/libc/posix/pthreads/pthread_tls.c
	rt-thread/components/libc/posix/pthreads/sched.c
	rt-thread/components/libc/posix/io/stdio/libc.c
	rt-thread/components/libc/posix/delay/delay.c
	rt-thread/components/libc/posix/io/poll/select.c
	rt-thread/components/libc/posix/io/poll/poll.c
	rt-thread/components/libc/compilers/newlib/syscalls.c
	rt-thread/components/libc/compilers/common/cctype.c
	rt-thread/components/libc/compilers/common/cstdio.c
	rt-thread/components/libc/compilers/common/cstdlib.c
	rt-thread/components/libc/compilers/common/cstring.c
	rt-thread/components/libc/compilers/common/ctime.c
	rt-thread/components/libc/compilers/common/cwchar.c
	rt-thread/components/finsh/cmd.c
	rt-thread/components/finsh/msh.c
	rt-thread/components/finsh/msh_file.c
	rt-thread/components/finsh/msh_parse.c
	rt-thread/components/finsh/shell.c
	rt-thread/components/drivers/spi/sfud/src/sfud.c
	rt-thread/components/drivers/spi/sfud/src/sfud_sfdp.c
	rt-thread/components/drivers/spi/spi_core.c
	rt-thread/components/drivers/spi/spi_dev.c
	rt-thread/components/drivers/spi/spi_flash_sfud.c
	rt-thread/components/drivers/serial/serial.c
	rt-thread/components/drivers/sdio/block_dev.c
	rt-thread/components/drivers/sdio/gpt.c
	rt-thread/components/drivers/sdio/mmc.c
	rt-thread/components/drivers/sdio/mmcsd_core.c
	rt-thread/components/drivers/sdio/sd.c
	rt-thread/components/drivers/sdio/sdio.c
	rt-thread/components/drivers/rtc/rtc.c
	rt-thread/components/drivers/rtc/soft_rtc.c
	rt-thread/components/drivers/phy/phy.c
	rt-thread/components/drivers/misc/pin.c
	rt-thread/components/drivers/ipc/completion.c
	rt-thread/components/drivers/ipc/dataqueue.c
	rt-thread/components/drivers/ipc/pipe.c
	rt-thread/components/drivers/ipc/ringblk_buf.c
	rt-thread/components/drivers/ipc/ringbuffer.c
	rt-thread/components/drivers/ipc/waitqueue.c
	rt-thread/components/drivers/ipc/workqueue.c
	rt-thread/components/drivers/can/can.c
	rt-thread/components/drivers/mtd/mtd_nor.c
	rt-thread/components/dfs/src/dfs.c
	rt-thread/components/dfs/src/dfs_file.c
	rt-thread/components/dfs/src/dfs_fs.c
	rt-thread/components/dfs/src/dfs_posix.c
	rt-thread/components/dfs/filesystems/elmfat/dfs_elm.c
	rt-thread/components/dfs/filesystems/elmfat/ff.c
	rt-thread/components/dfs/filesystems/elmfat/ffunicode.c
	rt-thread/components/dfs/filesystems/devfs/devfs.c
	libraries/STM32F4xx_HAL_Driver/Src/Legacy/stm32f4xx_hal_can.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc_ex.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_can.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cec.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_crc.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cryp.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cryp_ex.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dac.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dac_ex.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dcmi.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dcmi_ex.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dfsdm.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma2d.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dsi.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_eth.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_fmpi2c.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_fmpi2c_ex.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_fmpsmbus.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_fmpsmbus_ex.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_hash.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_hash_ex.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_hcd.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c_ex.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2s.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2s_ex.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_irda.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_iwdg.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_lptim.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_ltdc.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_ltdc_ex.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_mmc.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_nand.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_nor.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pccard.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd_ex.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_qspi.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rng.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rtc.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rtc_ex.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sai.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sai_ex.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sd.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sdram.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_smartcard.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_smbus.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spdifrx.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sram.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_usart.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_wwdg.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_adc.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_crc.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_dac.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_dma.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_dma2d.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_exti.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_fmc.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_fmpi2c.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_fsmc.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_gpio.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_i2c.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_lptim.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_pwr.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rcc.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rng.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rtc.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_sdmmc.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_spi.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_tim.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usart.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
	libraries/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_utils.c
	libraries/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f407xx.S
	libraries/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c
	drivers/board.c
	drivers/drv_adc.c
	drivers/drv_can.c
	drivers/drv_clk.c
	drivers/drv_common.c
	drivers/drv_crypto.c
	drivers/drv_dac.c
	drivers/drv_eth.c
	drivers/drv_flash_f4.c
	drivers/drv_gpio.c
	drivers/drv_hwtimer.c
	drivers/drv_lcd.c
	drivers/drv_lcd_mipi.c
	drivers/drv_lptim.c
	drivers/drv_pm.c
	drivers/drv_pwm.c
	drivers/drv_qspi.c
	drivers/drv_rtc.c
	drivers/drv_sdio.c
	drivers/drv_sdram.c
	drivers/drv_soft_i2c.c
	drivers/drv_spi.c
	drivers/drv_usart.c
	drivers/drv_usart_v2.c
	drivers/drv_usbd.c
	drivers/drv_usbh.c
	drivers/drv_wdt.c
	cubemx/Src/stm32f4xx_hal_msp.c
	rt-thread/components/utilities/ulog/backend/console_be.c
	rt-thread/components/utilities/ulog/ulog.c
	rt-thread/src/clock.c
	rt-thread/src/components.c
	rt-thread/src/device.c
	rt-thread/src/idle.c
	rt-thread/src/ipc.c
	rt-thread/src/irq.c
	rt-thread/src/kservice.c
	rt-thread/src/mem.c
	rt-thread/src/mempool.c
	rt-thread/src/object.c
	rt-thread/src/scheduler_up.c
	rt-thread/src/thread.c
	rt-thread/src/timer.c
)


add_subdirectory(packages)


LINK_DIRECTORIES(
)

LINK_LIBRARIES(
	c
	m
)

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME}.elf 
	${APP_SRCS} 
	${PROJECT_SOURCES} 
	${RT_FAL_SOURCES}
)


target_link_libraries(${CMAKE_PROJECT_NAME}.elf 
	CmBackTraceLib
)

# start_timer()
ADD_CUSTOM_COMMAND(TARGET ${CMAKE_PROJECT_NAME}.elf POST_BUILD
	COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.bin
	COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.hex
	COMMAND ${CMAKE_SIZE} --format=berkeley ${CMAKE_PROJECT_NAME}.elf
	COMMAND ${CMAKE_COMMAND} -E md5sum ${CMAKE_PROJECT_NAME}.bin
	COMMAND ${CMAKE_COMMAND} -E md5sum ${CMAKE_PROJECT_NAME}.hex
	# COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/fill_product.py
)

add_custom_target(flash
	DEPENDS ${CMAKE_PROJECT_NAME}.elf
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin  ${CMAKE_SOURCE_DIR}/_tools/PyOCD/0.1.3/
	COMMAND ${CMAKE_COMMAND} -E echo "Executing flash script..."
	COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}/_tools/PyOCD/0.1.3  ${CMAKE_SOURCE_DIR}/_tools/PyOCD/0.1.3/flash.bat
	COMMENT "Running batch script after build"
)

add_custom_target(openocd
	DEPENDS ${CMAKE_PROJECT_NAME}.elf
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.elf  ${CMAKE_SOURCE_DIR}/
	COMMAND ${CMAKE_COMMAND} -E echo "Executing openocd flash script..."
	COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}/   ${CMAKE_SOURCE_DIR}/openocd_flash.bat
	COMMENT "Running batch script after build"
)

add_custom_target(debug
	DEPENDS ${CMAKE_PROJECT_NAME}.elf
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin  ${CMAKE_SOURCE_DIR}/_tools/PyOCD/0.1.3/
	COMMAND ${CMAKE_COMMAND} -E echo "Executing flash&debug script..."
	COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}/_tools/PyOCD/0.1.3  ${CMAKE_SOURCE_DIR}/_tools/PyOCD/0.1.3/flash_debug.bat
	COMMENT "Running batch script after build"
)

add_custom_target(debug_only
	DEPENDS ${CMAKE_PROJECT_NAME}.elf
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin  ${CMAKE_SOURCE_DIR}/_tools/PyOCD/0.1.3/
	COMMAND ${CMAKE_COMMAND} -E echo "Executing flash&debug script..."
	COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}/_tools/PyOCD/0.1.3  ${CMAKE_SOURCE_DIR}/_tools/PyOCD/0.1.3/flash_debug_only.bat
	COMMENT "Running batch script after build"
)