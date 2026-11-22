# DoIP 网关

## 当前实现

App 在 lwIP 上启动两个服务：

| 协议 | 端口 | 功能 |
| --- | --- | --- |
| UDP | 13400 | 车辆识别请求/响应 |
| TCP | 13400 | 路由激活、Alive Check、诊断报文路由 |

DoIP 逻辑地址：

| 项目 | 值 |
| --- | --- |
| 网关实体地址 | `0x0100` |
| 默认测试仪地址 | `0x0E00` |
| VIN | `H750SIM0000000001` |
| CAN 诊断请求 ID | `0x731` |
| CAN 诊断响应 ID | `0x739` |

诊断路由结构：

```text
DoIP TCP 0x8001
  -> ISO-TP 客户端 0x731
  -> 外部 ECU 响应 0x739
  -> ISO-TP 客户端重组
  -> DoIP TCP 0x8001
```

TCP 路由激活成功码为 `0x10`。诊断报文接收后先返回 `0x8002`，CAN/ISO-TP
发送失败时返回 `0x8003`。当前只支持单个 TCP 测试仪连接和 2048 字节
DoIP payload。

## 网络地址

板卡固定地址：

```text
IP   192.168.100.30
MASK 255.255.255.0
GW   192.168.100.1
```

Windows 端可用同网段地址 `192.168.100.167/24`。把板卡 ETH 口接到
同一交换机，或直接接到 PC 网口。先确认：

```powershell
ping 192.168.100.30
```

板端 PHY link 为 down 时，DoIP 线程和 socket 仍会启动，但网络无法通信。

## 冒烟测试

脚本只使用 Python 标准库：

```powershell
python .\Scripts\doip-smoke-test.py --host 192.168.100.30 --uds 3E00
```

预期先看到车辆识别响应和路由激活成功，然后至少看到：

```text
UDP discovery OK: VIN=H750SIM0000000001, entity=0x0100
Routing activation: tester=0x0E00, entity=0x0100, code=0x10
Diagnostic ACK: 0E 00 01 00 00
```

若总线上有响应 `0x739` 的 CAN/ISO-TP 节点，随后还会打印：

```text
Diagnostic response: source=0x0100, target=0x0E00, UDS=...
```

只有网关本身而没有第二个 CAN 节点时，诊断请求会发出，但不会产生
`0x739` 响应；这属于总线拓扑问题，不是 DoIP socket 故障。

## 状态检查

串口 MSH 可执行：

```text
doip_info
```

可看到 UDP/TCP ready 状态、路由激活状态和收发计数。
