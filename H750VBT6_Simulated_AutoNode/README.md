# STM32H750 Simulated Auto Node

工程按职责分为四个目录：

| 目录 | 内容 |
| --- | --- |
| `App` | RT-Thread 应用、驱动、STM32 HAL、CubeMX、链接脚本和 CMake 工程 |
| `Boot` | 从片内 Flash 启动并跳转 QSPI App 的 Bootloader |
| `Docs` | Windows 构建烧录指南、QSPI 启动记录和 SVD |
| `Scripts` | Windows/Linux 构建、OpenOCD、PyOCD 和调试脚本 |

## 快速开始

```powershell
cd E:\Automotive-ECU-Simulator\H750VBT6_Simulated_AutoNode
.\Scripts\build-windows.ps1
.\Scripts\flash-windows.ps1
```

只编译或烧录 Boot：

```powershell
cmake -S Boot -B Boot/build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build Boot/build --parallel
.\Scripts\flash-windows.ps1 -BootOnly
```

只编译或烧录 App：

```powershell
cmake -S App -B App/build -G "Unix Makefiles" -DAPP_IN_QSPI=ON
cmake --build App/build --parallel
.\Scripts\flash-windows.ps1 -AppOnly
```

详细说明见 `Docs/WINDOWS_BUILD_FLASH.md` 和 `Docs/boot_app_qspi.md`。
