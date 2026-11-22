# STM32H750 RT-Thread Boot + QSPI App

## 实板验证结果

2026-11-22 在 STM32H750VBT6 + W25Q128 实板上完成验证：

| 项目 | 结果 |
| --- | --- |
| QSPI JEDEC ID | `EF 40 18`（W25Q128） |
| Boot 烧录 | `0x08000000`，OpenOCD 写入并校验通过 |
| Boot 向量 | `SP=0x24000C68`，`Reset=0x08002221` |
| Boot 大小 | `14,960` 字节，片内 128 KiB 限制内 |
| App 烧录 | `0x90000000`，OpenOCD 写入并校验通过 |
| App 大小 | `224,028` 字节 |
| App 向量 | `SP=0x24001280`，`Reset=0x90027D2D` |
| App 入口断点 | `PC=0x90027D2C` |
| 运行时 VTOR | `0x90000000` |
| 异常状态 | `CFSR=0x00000000`，`HFSR=0x00000000` |
| 连续运行 | 受控复位后稳定运行 40 秒，未再次进入 Boot |

串口可同时看到 Boot 和 App 日志。App 未插 SD 卡时可能出现
`mmcsd_dev: wait cmd completed timeout`，这是板级 SD 卡检测日志，与
Boot/QSPI 启动无关。

## 分区和引脚

| 镜像 | 存储器 | 地址 | 大小限制 |
| --- | --- | --- | --- |
| RT-Thread Boot | STM32H750 片内 Flash | `0x08000000` | 128 KiB |
| Simulated-GW-ECU | W25Q128 QSPI Flash | `0x90000000` | 16 MiB |

本板 QSPI 引脚：

| 信号 | 引脚 | 复用 |
| --- | --- | --- |
| `QSPI_CLK` | `PB2` | AF9 |
| `QSPI_BK1_NCS` | `PB6` | AF10 |
| `QSPI_BK1_IO0` | `PD11` | AF9 |
| `QSPI_BK1_IO1` | `PD12` | AF9 |
| `QSPI_BK1_IO2` | `PE2` | AF9 |
| `QSPI_BK1_IO3` | `PD13` | AF9 |

Boot 启动后初始化 QSPI、读取 JEDEC ID、置位 W25Q128 的 QE 位，校验 App
向量表，然后保留 Thumb 位设置 `VTOR=0x90000000` 并跳转。App 使用
`0x6B` 四线 XIP，失败时 Boot 回退到单线映射。

## 启动问题修复记录

早期 Boot 在 `boot_app_jump()` 中调用 `SCB_CleanDCache()` 清理整个
D-Cache。实板断点抓到异常发生在 `0x08000940` 的缓存脏行回写循环内，
此时 `CFSR=0x00000400`（`IMPRECISERR`），`HFSR=0x40000000`
（`FORCED`）。该回写可能访问 QSPI 映射区或其他不可写区域，导致跳转前
触发 HardFault。

现在 Boot 在跳转前关闭 I-Cache/D-Cache，只执行
`SCB_InvalidateDCache()`，不再把 Boot 的脏缓存写回存储器，然后设置
`VTOR` 并用保留 Thumb 位的入口地址跳转。App 启动后会重新打开缓存。

## 构建

```bash
cd H750VBT6_Simulated_AutoNode

cmake -S Boot -B Boot/build -DCMAKE_BUILD_TYPE=Release
cmake --build Boot/build -j"$(nproc)"

cmake -S App -B App/build -DAPP_IN_QSPI=ON
cmake --build App/build -j"$(nproc)"
```

生成：

- `Boot/build/RTThread-Boot.bin`
- `App/build/Simulated-GW-ECU.bin`

Boot 构建会检查镜像不超过 128 KiB。App 链接脚本为
`App/linkscripts/STM32H750VBTx/app_qspi.lds`，App 向量表位于
`0x90000000`。

Windows 下的脚本构建和烧录步骤见 `Docs/WINDOWS_BUILD_FLASH.md`。

## 连接检查

```bash
lsusb | grep -i 0d28
ls -l /dev/ttyACM0

cd H750VBT6_Simulated_AutoNode
./Scripts/openocd-scripts/check-qspi.sh
```

预期 QSPI ID：

```text
QSPI JEDEC ID: EF 40 18
```

`check-qspi.sh` 只通过 QSPI 寄存器执行间接读，不会在控制器未初始化时访问
`0x90000000`。`qspi_init.tcl` 中 `READ_ID` 的 CCR 必须是 `0x0500019F`
（间接读）；写成 `0x0100019F`（间接写）会一直等不到 `TCF`。

## 烧录

Boot：

```bash
cd H750VBT6_Simulated_AutoNode
./Scripts/openocd-scripts/flash-boot.sh
```

App：

```bash
cd H750VBT6_Simulated_AutoNode
./Scripts/openocd-scripts/flash-app.sh
```

两个脚本都通过 `program ... verify ...` 写入并回读校验。OpenOCD 的
`program` 辅助命令内部会执行一次 `reset init`，因此
`flash_app.cfg` 把 QSPI 初始化挂到目标的 `reset-init` 事件上；否则复位会
清掉 QSPI 控制器配置，导致擦除阶段报
`Cannot write enable flash1. Status=0x00`。

## 启动检查

串口为 DAPLink 第二路 CDC，参数 `115200 8N1`。显式配置串口终端可避免
本地 `stty` 遗留设置造成乱码：

```bash
stty -F /dev/ttyACM0 115200 cs8 -parenb -cstopb -ixon -ixoff -crtscts raw -echo
cat /dev/ttyACM0
```

Boot 实板输出：

```text
[Boot] RT-Thread QSPI bootloader
[Boot] QSPI JEDEC ID: EF4018
[Boot] QSPI mapped in 4-bit mode.
[Boot] Application vector: SP=0x24001280 entry=0x90027D2D
[Boot] Jumping to application at 0x90000000
```

随后 App 输出 RT-Thread 启动信息和应用日志，包括：

```text
initialize rti_board_start:0 done
initialize rt_hw_can_init:0 done
do components initialization.
isotp main: find fdcan1 success!
isotp main: create cantp_rx thread success!
```

App 入口和运行时状态可用下面的断点检查复现：

```bash
cd H750VBT6_Simulated_AutoNode/Scripts/openocd-scripts

openocd -f interface/cmsis-dap.cfg -f target/stm32h7x_extern.cfg -c \
  'init; reset halt; bp 0x90027d2c 2 hw; resume; wait_halt 5000;
   echo "--- app entry ---";
   echo [format "VTOR=%08x" [mrw 0xE000ED08]];
   echo [format "CFSR=%08x" [mrw 0xE000ED28]];
   resume; shutdown'
```

预期看到：

```text
[stm32h7x.cpu0] halted due to breakpoint, current mode: Thread
xPSR: 0x61000000 pc: 0x90027d2c psp: ...
--- app entry ---
VTOR=90000000
CFSR=00000000
```
