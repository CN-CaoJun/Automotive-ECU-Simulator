# VS Code Boot/App 调试

在仓库根目录 `E:\Automotive-ECU-Simulator` 打开 VS Code。根目录
`.vscode/launch.json` 和 `.vscode/tasks.json` 已提供 Boot/App
两套独立调试入口。

## 调试 Boot

选择：

```text
Debug Boot (build + flash + OpenOCD)
```

启动任务会自动：

1. 使用 Debug 配置和 `-O0 -g3` 编译 `Boot`；
2. 烧录 `RTThread-Boot.bin` 到 `0x08000000`；
3. 启动 OpenOCD，复位后停在 Boot reset；
4. 加载 Boot ELF 并运行到 `main()`。

## 调试 App

选择：

```text
Debug App (build + flash + OpenOCD)
```

启动任务会自动：

1. 编译 QSPI App；
2. 烧录 App 到 `0x90000000`（要求 Boot 已烧录）；
3. 初始化 QSPI 映射并复位；
4. 加载 App ELF，Boot 跳转后在 App `main()` 命中。

App 运行在 QSPI XIP 区域，GDB 会自动使用硬件断点。VS Code
中断点、调用栈、变量查看和外设寄存器均可使用。

## 手工任务

任务面板中也可以单独执行：

- `Build Boot (Debug)`
- `Build App (Debug)`
- `Flash Boot`
- `Flash App`
- `Prepare Boot Debug`
- `Prepare App Debug`

## 已实测

2026-11-22 使用 CMSIS-DAP + OpenOCD 验证：

- Boot Debug 在 `Boot/applications/main.c` 的 `main()` 命中；
- App Debug 从 Boot 跳转后，在 `App/applications/main.c` 的
  `main()` 命中，入口位于 QSPI `0x900...`；
- GDB 对 QSPI 只读地址自动使用硬件断点。
