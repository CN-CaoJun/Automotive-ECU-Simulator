# Windows 编译与烧录指南

本文记录 STM32H750VBT6 + RT-Thread + W25Q128 QSPI App 工程在 Windows 下的
编译、烧录和启动验证方法。

工程目录结构：

```text
H750VBT6_Simulated_AutoNode/
├── App/       RT-Thread 应用、驱动、HAL、CubeMX 和 CMake 工程
├── Boot/      QSPI Bootloader
├── Docs/      工程文档和 SVD
└── Scripts/   构建、OpenOCD、PyOCD 和调试脚本
```

验证环境：

- Windows 10/11
- STM32H750VBT6 + W25Q128
- DAPLink（CMSIS-DAP）
- RT-Thread 5.4.0（主线 `6fdfe700`）
- 2026-11-23 实板验证通过

## 1. 工具链

工具链统一放在 `E:\tools`：

| 工具 | 版本 | 路径 |
| --- | --- | --- |
| ARM GCC | 10.3.1 | `E:\tools\xpack-arm-none-eabi-gcc-10.3.1-2.1` |
| CMake | 4.4.4 | `E:\tools\cmake-4.4.4-windows-x86_64` |
| GNU Make | 4.4.1 | `E:\tools\mingw64\bin\make.exe` |
| OpenOCD | 0.12.0 | `E:\tools\OpenOCD-20260302-0.12.0` |

构建使用 `CMake + Unix Makefiles`，不需要 Ninja。

确认工具链版本：

```powershell
arm-none-eabi-gcc --version
cmake --version
make --version
openocd --version
```

如果新开的 PowerShell 找不到命令，可临时设置当前终端环境：

```powershell
$env:ARM_GCC_ROOT = 'E:\tools\xpack-arm-none-eabi-gcc-10.3.1-2.1'
$env:PATH = 'E:\tools\cmake-4.4.4-windows-x86_64\bin;E:\tools\xpack-arm-none-eabi-gcc-10.3.1-2.1\bin;E:\tools\mingw64\bin;E:\tools\OpenOCD-20260302-0.12.0\bin;' + $env:PATH
```

也可以将以上目录永久加入当前用户的 PATH。设置后需要重新打开终端。

## 2. 编译

进入工程目录：

```powershell
cd E:\Automotive-ECU-Simulator\H750VBT6_Simulated_AutoNode
```

一键编译 Boot 和 QSPI App：

```powershell
.\Scripts\build-windows.ps1
```

脚本会分别执行：

1. 配置并编译 `Boot`，输出到 `Boot\build`。
2. 配置并编译 `App`，输出到 `App\build`。
3. App 使用 QSPI XIP 链接脚本，镜像必须从 `0x90000000` 运行。

生成文件：

| 镜像 | 文件 | 烧录地址 |
| --- | --- | --- |
| RT-Thread Boot | `Boot\build\RTThread-Boot.bin` | `0x08000000` |
| Simulated-GW-ECU | `App\build\Simulated-GW-ECU.bin` | `0x90000000` |

本次验证结果（RT-Thread 5.4.0）：

```text
RTThread-Boot.elf  text 14876  data 1124  bss 5416
Boot image size: 16000 / 131072 bytes

Simulated-GW-ECU.elf  text 336344  data 4384  bss 93628
Simulated-GW-ECU.bin  340728 bytes
```

## 3. 手工编译命令

如果不用脚本，可按以下命令编译。

编译 Boot：

```powershell
cmake -S Boot -B Boot/build `
  -G "Unix Makefiles" `
  -DCMAKE_MAKE_PROGRAM="E:/tools/mingw64/bin/make.exe" `
  -DCMAKE_BUILD_TYPE=Release

cmake --build Boot/build --parallel
```

编译 QSPI App：

```powershell
cmake -S App -B App/build `
  -G "Unix Makefiles" `
  -DCMAKE_MAKE_PROGRAM="E:/tools/mingw64/bin/make.exe" `
  -DAPP_IN_QSPI=ON

cmake --build App/build --parallel
```

CMake 会通过 `ARM_GCC_ROOT` 或 PATH 查找 ARM GCC。也可以显式指定：

```powershell
cmake -S Boot -B Boot/build `
  -G "Unix Makefiles" `
  -DCMAKE_MAKE_PROGRAM="E:/tools/mingw64/bin/make.exe" `
  -DTOOL_DIR="E:/tools/xpack-arm-none-eabi-gcc-10.3.1-2.1"
```

## 4. 烧录

连接 DAPLink 和板子电源，确认设备管理器中能看到 CMSIS-DAP。

烧录 Boot 和 App，并执行回读校验：

```powershell
cd E:\Automotive-ECU-Simulator\H750VBT6_Simulated_AutoNode
.\Scripts\flash-windows.ps1
```

只烧 Boot：

```powershell
.\Scripts\flash-windows.ps1 -BootOnly
```

只烧 App：

```powershell
.\Scripts\flash-windows.ps1 -AppOnly
```

成功时 OpenOCD 输出应包含：

```text
** Programming Started **
** Programming Finished **
** Verify Started **
** Verified OK **
```

App 烧录时还应看到 QSPI 型号识别结果：

```text
QSPI JEDEC ID: EF 40 18
flash1 'win w25q128fv/jv' id = 0x1840ef size = 16384 KiB
```

## 5. 手工烧录命令

打开 OpenOCD 脚本目录：

```powershell
cd E:\Automotive-ECU-Simulator\H750VBT6_Simulated_AutoNode\Scripts\openocd-scripts
```

烧录 Boot：

```powershell
openocd -f interface/cmsis-dap.cfg `
  -f target/stm32h7x_extern.cfg `
  -f flash_boot.cfg
```

烧录 App：

```powershell
openocd -f interface/cmsis-dap.cfg `
  -f target/stm32h7x_extern.cfg `
  -f flash_app.cfg
```

如果 `openocd` 不在 PATH 中，使用完整路径：

```powershell
& 'E:\tools\OpenOCD-20260302-0.12.0\bin\openocd.exe' `
  -f interface/cmsis-dap.cfg `
  -f target/stm32h7x_extern.cfg `
  -f flash_boot.cfg
```

`flash_boot.cfg` 和 `flash_app.cfg` 使用相对路径查找构建产物，因此必须在
`Scripts\openocd-scripts` 目录中执行。

## 6. 启动验证

串口参数：

```text
COM3  115200  8N1  无流控
```

正常启动日志：

```text
 \ | /
- RT -     Thread Operating System
 / | \     5.4.0 build Nov 22 2026 23:06:00
 2006 - 2026 Copyright by RT-Thread team

[Boot] RT-Thread QSPI bootloader
[Boot] QSPI JEDEC ID: EF4018
[Boot] QSPI mapped in 4-bit mode.
[Boot] Application vector: SP=0x24001520 entry=0x90041F59
[Boot] Jumping to application at 0x90000000
initialize rti_board_start:0 done
initialize rt_hw_can_init:0 done
isotp main: find fdcan1 success!
isotp main: create cantp_rx thread success!
```

2026-11-23 的 Release 复验中，App 静态占用为：

```text
text 336344  data 4384  bss 93628
bin 340728 bytes
```

TF 卡已接入 SDMMC1 并能在运行日志中完成 `mmcsd_detect`；CAN 引脚
保持在 AF9（`GPIOB_AFRH=0x00bbb099`）。SPI1 已识别 W25Q64
（`EF 40 17`），PB0/PE13 LED 默认熄灭。ETH 驱动、DP83848C PHY
和 lwIP 初始化正常，但只有板卡 ETH 网口物理连接到主机或交换机后
才会出现 link up。DoIP 的 UDP/TCP 13400 服务已随 App 启动，
端到端测试脚本见 `Docs/DOIP.md`。

升级与迁移细节见 `Docs/RTTHREAD_UPGRADE.md`。

也可以通过 SWD 检查运行时状态：

```powershell
cd E:\Automotive-ECU-Simulator\H750VBT6_Simulated_AutoNode\Scripts\openocd-scripts

openocd -f interface/cmsis-dap.cfg `
  -f target/stm32h7x_extern.cfg `
  -c "init; reset run; sleep 4000; halt; reg pc; echo [format {VTOR=%08x} [mrw 0xE000ED08]]; echo [format {CFSR=%08x} [mrw 0xE000ED28]]; echo [format {HFSR=%08x} [mrw 0xE000ED2C]]; resume; shutdown"
```

预期结果：

```text
pc: 0x9000xxxx
VTOR=90000000
CFSR=00000000
HFSR=00000000
```

## 7. 常见问题

### 7.1 CMake 找不到 ARM GCC

检查 `ARM_GCC_ROOT` 和 PATH：

```powershell
$env:ARM_GCC_ROOT
arm-none-eabi-gcc --version
```

也可以在配置时传入：

```powershell
-DTOOL_DIR="E:/tools/xpack-arm-none-eabi-gcc-10.3.1-2.1"
```

### 7.2 Boot 链接时报 `_exit`、`_kill`、`_getpid` 未定义

Boot 是无 RTOS 文件系统的裸机镜像，已经在
`Boot\CMakeLists.txt` 中加入：

```text
--specs=nosys.specs
```

如果该参数被删除，链接会再次失败。

### 7.3 `Access to the path 'COM3' is denied`

串口被其他程序占用。关闭 MobaXterm、PuTTY、串口助手或其他终端中的 COM3
连接后重试。烧录本身使用 SWD，不占用串口。

### 7.4 QSPI ID 为 `000000` 或 `FFFFFF`

检查：

- W25Q128 供电是否正常。
- `PB2`、`PB6`、`PD11`、`PD12`、`PD13`、`PE2` 的 QSPI 连线。
- 是否使用 `stm32h7x_extern.cfg`，其中已创建 `0x90000000` 的外部 Flash bank。

QSPI 控制器未初始化时不要直接读取 `0x90000000`。

### 7.5 App 烧录时报 `Cannot write enable flash1`

`flash_app.cfg` 已把 QSPI 初始化挂到 OpenOCD 的 `reset-init` 事件上。
不要删除 `qspi_init.tcl` 或其中的 `reset-init` 绑定，否则擦除阶段可能找不到
外部 Flash。

## 8. 文件说明

| 文件 | 用途 |
| --- | --- |
| `Scripts\build-windows.ps1` | 一键配置并编译 Boot 和 App |
| `Scripts\flash-windows.ps1` | 一键烧录并校验 Boot 和 App |
| `Scripts\openocd-scripts\flash_boot.cfg` | OpenOCD Boot 烧录配置 |
| `Scripts\openocd-scripts\flash_app.cfg` | OpenOCD QSPI App 烧录配置 |
| `Scripts\openocd-scripts\qspi_init.tcl` | 烧录前初始化 QSPI 控制器 |
| `Boot\build\RTThread-Boot.bin` | Boot 烧录镜像 |
| `App\build\Simulated-GW-ECU.bin` | App 烧录镜像 |
