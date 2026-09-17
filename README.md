# ESP32 桌面番茄钟

用经典 ESP32 从零做一个**脱离电脑也能独立运行**的桌面番茄钟。面包板原型，全程免焊接。

面向有编程基础、刚开始接触硬件的学习者。完整方案见 [`docs/ESP32番茄钟制作指南.md`](docs/ESP32番茄钟制作指南.md)。

## 功能

- 默认专注 25 分钟，短休息 5 分钟，完成 4 次专注后长休息 15 分钟
- A 键：开始 / 暂停 / 继续；B 键：重置当前阶段
- 时间到：屏幕显示 `DONE`，LED 闪烁；按 A 确认后进入下一阶段
- 屏幕显示阶段、剩余时间、运行状态、已完成专注次数

第一版不发声、不联网、不保存断电前进度。

## 分阶段进度

| 阶段 | 内容 | 状态 |
| --- | --- | --- |
| 1 | 裸板串口自检（不接任何器件） | ✅ 已完成 |
| 2 | 只接 OLED，扫描 I²C 地址 | ⬜ 待做 |
| 3 | 只接按钮 + LED，独立测试 | ⬜ 待做 |
| 4 | 接全套，烧录完整程序（`TEST_MODE = true`） | ⬜ 待做 |
| 5 | 关闭 `TEST_MODE`，脱机运行并与手机对表 | ⬜ 待做 |

分阶段是为了**每次只引入一个变量**：链接不通、屏幕不亮、按钮失灵、逻辑跑飞，各自有明确的验证手段。

## 硬件清单

| 器材 | 规格 | 连接 |
| --- | --- | --- |
| ESP32 开发板 | 经典 ESP32-WROOM-32 DevKit（板载 CP2102） | USB 供电 + 烧录 |
| OLED | 0.96" 128×64 SSD1306，I²C 四针 | GND / 3V3 / SDA→GPIO21 / SCL→GPIO22 |
| 轻触按钮 ×2 | 6×6mm 四脚直插，常开 | A: GPIO18↔GND　B: GPIO19↔GND（内部上拉） |
| LED ×1 | 3mm/5mm 红色直插 | GPIO23 → 1kΩ → LED 正极，负极→GND |
| 面包板 ×2 + 杜邦线 | | |

ESP32 GPIO 是 3.3V 电平，**不要直接接 5V**。改变接线前先拔 USB。

## 开发环境

| 项 | 版本 / 值 |
| --- | --- |
| 构建系统 | PlatformIO Core 6.2.0（VSCode 的 PlatformIO IDE 扩展 3.3.4） |
| 平台 | `espressif32` 7.1.3 |
| 框架 | `framework-arduinoespressif32` 4.20017.260907（即 arduino-esp32 3.x） |
| 串口 | CP2102 → `COM3`，波特率 115200 |

`platformio.ini` 里只设了三件事：平台版本、开发板、框架，外加串口监视器波特率。

> **为什么不用 Arduino IDE**：PlatformIO 用 `lib_deps` 声明依赖，换电脑时 `pio run` 一条命令即可重建环境；Arduino IDE 的库装在全局目录里，不可复现。

## 构建与烧录

```bash
pio run                 # 只编译
pio run -t upload       # 编译并烧录
pio device monitor      # 打开串口监视器（115200）
```

烧录成功的两个关键标志：

```
Chip is ESP32-D0WD-V3 (revision v3.1)     ← 握手成功，芯片型号识别正确
Hash of data verified.                    ← 写入内容校验通过
```

若卡在 `Connecting........_____`：**按住板上 `BOOT` 键 → 重新上传 → 看到 `Writing at ...` 后松开**。

## 目录结构

```
esp32-pomodoro/
├── platformio.ini        构建配置（平台 / 开发板 / 框架 / 串口波特率）
├── src/main.cpp          主程序，阶段 1 目前是串口自检
├── include/              项目私有头文件（当前未使用）
├── lib/                  项目私有库（当前未使用）
├── test/                 单元测试（当前未使用）
└── docs/                 方案文档与学习资料
```

## 已知环境坑（备忘）

1. **PlatformIO 下载极慢**：PIO 用 Python `requests`，**不读 Windows 的系统代理（WinINET）**。必须在 `HTTP_PROXY` / `HTTPS_PROXY` 环境变量里配置代理，否则 115 MB 的工具链要下 80 分钟。
2. **`command 'platformio-ide.build' not found`**：多半是某个卡死的 `pio` 进程持有着包锁，导致扩展的 `activate()` 不返回、命令没注册。处理：完全退出 VSCode → 杀掉所有 `platformio` python 进程 → 删 `~/.platformio/.cache/downloads/*.lock`。
3. **`无法打开源文件 "Arduino.h"`**：这是 C/C++ 扩展（IntelliSense）的误报，不是编译错误。先构建一次，PIO 会生成 `.vscode/c_cpp_properties.json`，波浪线自动消失。**不要手工改 `includePath`**。
4. **Micro-USB 线必须能传数据**：板子红灯亮只代表 VBUS 通了，**不代表数据线通**。判断数据线是否可用，看系统能否报出 USB 设备的 `VID`/`PID`（USB 枚举必须走 D+/D-）。

## 后续扩展方向

可设置专注时长 → 用 NVS 保存设置 → 蜂鸣器提醒 → 外壳与固定 → Wi-Fi 对时与统计网页。
