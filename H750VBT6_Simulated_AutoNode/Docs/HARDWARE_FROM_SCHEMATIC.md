# STM32H750 板级原理图对齐

依据：`Docs/原理图.pdf`（Schematic Prints，2024-04-21，单页 A4）。

## 电源与时钟

| 项目 | 原理图 |
| --- | --- |
| 主供电 | 5V USB 输入，AMS1117-3.3 产生 3.3V |
| HSE | 25 MHz |
| LSE | 32.768 kHz，接 PC14/PC15 |
| ETH 参考时钟 | 50 MHz 有源振荡器，接 PHY X1 和 MCU PA1 |

## 已对齐外设

| 功能 | 原理图连接 | 当前配置 | 状态 |
| --- | --- | --- | --- |
| QSPI App Flash | W25Q128：PB2 CLK、PB6 NCS、PD11/PD12/PD13/PE2 IO0-IO3 | Boot 直接初始化 QSPI，App XIP 于 `0x90000000` | 已验证 |
| 调试/控制台 | USART1：PA9 TX、PA10 RX；DAPLink | `uart1`，115200 8N1 | 已验证 |
| CAN | TJA1050：PB8 RX、PB9 TX，120R 终端 | FDCAN1，设备名 `fdcan1`，PB8/PB9 AF9 | 已配置，待接总线验证 |
| 以太网 | DP83848C RMII：PB11 TX_EN、PB12 TXD0、PB13 TXD1、PC1 MDC、PC4 RXD0、PC5 RXD1、PA2 MDIO、PA7 CRS_DV、PA1 REF_CLK、PA5 RESET | RMII + lwIP，DP83848C | PHY `0x01` 已识别，MAC 已初始化 |
| RS-485 | SP3485：PD5 TX、PD6 RX | USART2 | 已注册；方向脚不由 MCU 控制 |
| TF 卡 | SDMMC1：PC8-PC12、PD2 | RT-Thread 5.4 SDIO + 块设备，设备名 `sd` | 已识别 SDHC 卡 |
| USB OTG FS | PA11 DM、PA12 DP；USB-C 与 USB-A 共线 | 未启用 | 硬件只支持设备/主机二选一 |
| SPI Flash | W25Q64：PB3 SCK、PB4 MISO、PB5 MOSI、PD3 CS | SPI1，设备 `w25q64` | 已识别 JEDEC `EF 40 17` |
| EEPROM | 24C02：PB10 SCL、PB11 SDA | 未启用 | 与 ETH PB11 冲突 |
| LED | PB0-PWM、PE13-LED1，低电平点亮 | GPIO 推挽输出，默认熄灭 | 已配置；MSH 命令 `led on/off` |
| RTC | LSE 32.768 kHz：PC14/PC15 | 未启用 | 当前业务不依赖 RTC，可按需启用 |
| IO 扩展排针 | 引出 FDCAN/SDMMC/SPI/USART 等备用引脚 | 未分配固定功能 | 作为扩展接口保留 |
| DoIP | ETH 上的 UDP/TCP 13400 | lwIP socket + 路由激活 + CAN/ISO-TP 路由 | 服务已启动，待物理链路验证 |

## ETH 验证

当前固定地址：

```text
IP  192.168.100.30
GW  192.168.100.1
MASK 255.255.255.0
```

2026-11-23 实板启动日志：

```text
initialize rt_hw_stm32_eth_init
<6>drv.emac main: Found PHY at address 0x01
<6>drv.emac main: Ethernet MAC initialized
```

MSH 命令：

```text
eth_info
Ethernet PHY: address 0x01
Link: down, 100 Mbps, full-duplex
MAC: 00:80:E1:11:30:2D
IP: 192.168.100.30
```

当前 Windows `以太网 2` 连接的是办公网（1 Gbps，地址
`192.168.100.167`），板卡 ETH 口没有接入该网络，因此板端 PHY
仍为 down。把板卡 ETH 口接到同一交换机，或用网线直连
`以太网 2` 后，预期出现 `link up` 并可执行
`ping 192.168.100.30`。

## 原理图冲突与待办

1. **PB11 冲突**：EEPROM 的 I2C2 SDA 和 DP83848 RMII_TX_EN 都使用
   PB11。当前以以太网为优先，EEPROM 不启用；若要使用 EEPROM，
   必须修改硬件或分时复用，不能在 ETH 工作时同时使用。
2. **CAN 编号**：原理图网络名写作 `CAN2`，但 PB8/PB9 实际是
   STM32H750 的 FDCAN1 引脚。软件使用 `fdcan1`，没有启用不存在的
   FDCAN2 收发器。
3. **RS-485 方向**：原理图中 SP3485 的 RE/DE 通过电阻固定偏置，
   没有 MCU 方向控制 GPIO。当前只能作为固定方向半双工接口，
   若需双向收发需改板增加方向脚。
4. **TF 卡**：SDMMC1 引脚与原理图一致。RT-Thread 5.4 把 SDIO
   核心拆分为 `dev_mmcsd_core/dev_sd/dev_mmc/dev_sdio/dev_block`，
   已全部加入构建；实板插入 TF 卡后识别为 SDHC（`0x1D1E800 KiB`），
   未再出现 UsageFault。原理图没有独立 CD 引脚，热插拔检测不能
   依赖 GPIO 卡检测脚。
5. **USB**：USB-C 和 USB-A 共用 PA11/PA12，不能同时作为设备和
   主机使用；当前未启用 USB 协议栈。

## DoIP 验证

ETH 已具备 DP83848 + RMII + lwIP 基础，DoIP 服务代码已经加入：

- DoIP UDP 13400 车辆发现/声明；
- TCP 13400 路由激活；
- DoIP 诊断报文与现有 CAN/ISO-TP 节点之间的路由。

逻辑地址和测试方法见 `Docs/DOIP.md`。接线确认后先执行：

```text
eth_info
```

Windows 侧：

```powershell
ping 192.168.100.30
python .\Scripts\doip-smoke-test.py --host 192.168.100.30
```

看到 `Link: up`、正确 IP 和 DoIP 路由激活成功后，再做诊断报文的
端到端总线验证。当前缺少板卡 ETH 物理连接，因此还没有把
`ping` 或 DoIP socket 通信标记为通过。
