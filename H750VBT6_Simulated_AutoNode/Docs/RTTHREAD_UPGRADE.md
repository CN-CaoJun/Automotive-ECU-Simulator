# RT-Thread 主线升级记录

## 升级基线

| 项目 | 值 |
| --- | --- |
| 原 RT-Thread | `v5.1.0`（`4f7940167d172858e37373bc330c7093e8765d5b`） |
| 新 RT-Thread | `master` 主线 RT-Thread 5.4.0 |
| 主线提交 | `6fdfe70062ec394e5dd7099c51cbcff127cacab1`（2026-11-18） |
| 应用配置版本 | `RT_VER_NUM=0x50400` |

App 与 Boot 共用 `App/rt-thread` 内核快照。升级同步了内核
`src`、`include`、Cortex-M7 移植层，以及本项目启用的设备驱动、
FINSH、DFS v1、newlib、ULOG 和工具脚本。

## 迁移内容

- 内核源文件切换到新调度器架构：增加 `cpu_up.c`、`defunct.c`、
  `klibc/kerrno.c` 和新版 `rt_vsnprintf`/`rt_vsscanf`。
- CAN 驱动框架切换到 `dev_can.c`，FDCAN 适配
  `RT_CAN_MODE_LISTEN`、新的收发返回值和 CAN-FD 位掩码解析。
- 串口、PIN、SDIO、块设备和 IPC 切换到主线新实现；启用
  `RT_USING_BLK`，补齐 RT-Thread 5.4 拆分后的 SDIO 核心源文件，
  并适配新版块设备框架。
- App 的 `RT_NAME_MAX` 调整为 16，避免主线新增对象名被截断。
- 保留 QSPI 启动布局、Boot 跳转前 Cache 处理、`VTOR=0x90000000`
  和 W25Q128 四线映射等 H750 板级改动。

## 构建验证

构建命令：

```powershell
cd E:\Automotive-ECU-Simulator\H750VBT6_Simulated_AutoNode
.\Scripts\build-windows.ps1 -Fresh
```

使用 Windows CMake + Unix Makefiles、ARM GCC 10.3.1 完成全新构建。

```text
RTThread-Boot.elf  text 14876  data 1124  bss 5416
Boot image size: 16000 / 131072 bytes

Simulated-GW-ECU.elf  text 336344  data 4384  bss 93628
Simulated-GW-ECU.bin  340728 bytes
```

## lwIP 集成方式

本项目不是“另外下载一个 lwIP 包”，而是直接使用 RT-Thread 主线自带的
`components/net/lwip`：

- 版本选择：`RT_USING_LWIP212`，即 lwIP 2.1.2；
- RT-Thread 适配层：`components/net/lwip/port/ethernetif.c` 和
  `components/net/lwip/port/sys_arch.c`；
- lwIP 配置：`components/net/lwip/port/lwipopts.h`，由 `rtconfig.h`
  中的 `RT_LWIP_*` 选项驱动；
- 网络接口：`drv_eth.c` 初始化 DP83848C，并通过 `ethernetif.c` 注册
  lwIP netif；
- 当前固定地址：`192.168.100.30/24`，TCP/UDP socket 已用于 DoIP
  13400 服务。

RT-Thread 的官方工具链通常通过 `menuconfig`/SConscript 选择这些源文件。
本工程要求使用 Windows CMake + Unix Makefiles，因此不能直接执行
`scons`；我已把官方 Kconfig 选项保持为 `RT_USING_LWIP` +
`RT_USING_LWIP212`，并将官方 lwIP/port 源文件显式加入
`App/CMakeLists.txt`。这不是替代实现，编译对象仍是
`rt-thread/components/net/lwip/lwip-2.1.2` 和官方 port。

如果希望以后改成 RT-Thread 原生 SCons 流程，可以另建 SCons 构建入口；
当前交付的 CMake 构建不需要外部 lwIP 包管理下载。

## 烧录与运行验证

2026-11-23 在 STM32H750VBT6 + W25Q128 实板完成：

| 项目 | 结果 |
| --- | --- |
| Boot 烧录 | `0x08000000`，写入并校验通过 |
| App 烧录 | `0x90000000`，写入并校验通过 |
| QSPI JEDEC ID | `EF4018` |
| Boot 日志 | QSPI 四线映射、App 向量检查、跳转成功 |
| App 内核版本 | RT-Thread 5.4.0 |
| 运行时 VTOR | `0x90000000` |
| CFSR / HFSR | `0x00000000` / `0x00000000` |
| RT-Thread tick | 复位后持续递增 |
| CAN 初始化 | `find fdcan1 success` |
| CAN 接收线程 | `create cantp_rx thread success` |
| CAN 引脚复用 | `GPIOB_AFRH=0x00bbb099`，PB8/PB9 为 AF9 |
| TF 卡 | SDMMC1 识别 SDHC 卡，容量 `0x1D1E800 KiB`（约 29.1 GiB） |
| SPI Flash | SPI1 识别 W25Q64，JEDEC ID `EF 40 17` |
| LED | PB0/PE13 已配置为推挽输出，默认熄灭 |
| DoIP | UDP/TCP 13400 socket 已启动并处于可绑定状态 |
| ETH | RMII + DP83848C 初始化成功，PHY 地址 `0x01`，MAC `00:80:E1:11:30:2D` |

串口关键日志：

```text
[Boot] RT-Thread QSPI bootloader
[Boot] QSPI JEDEC ID: EF4018
[Boot] QSPI mapped in 4-bit mode.
[Boot] Application vector: SP=0x24001520 entry=0x90041F59
[Boot] Jumping to application at 0x90000000

 \ | /
- RT -     Thread Operating System
 / | \     5.4.0 build Nov 22 2026 23:06:05
 2006 - 2026 Copyright by RT-Thread team

initialize rt_hw_can_init:0 done
<7>isotp main: find fdcan1 success!
<7>isotp main: create cantp_rx thread success!
```

SDMMC1 已接入 RT-Thread 5.4 的 `dev_mmcsd_core`、`dev_sd`、
`dev_mmc`、`dev_sdio` 和 `dev_block`。实板上插入 TF 卡后，
`mmcsd_detect` 正常完成识别，连续运行 20 秒未再出现 UsageFault。

ETH 侧尚未接板卡网线：Windows 的 `以太网 2` 当前连接到办公网，
板端 PHY link 保持 down，因此 ping 尚不能通过。该状态不影响
QSPI 启动、CAN 和 SDIO。
