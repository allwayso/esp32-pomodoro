# ESP32 桌面番茄钟

基于 ESP32-WROOM-32、1.54 英寸 ST7789 TFT 和摇杆模块制作的桌面番茄钟。程序烧录后可使用普通 USB 电源脱离电脑运行。

## 当前功能

- 专注 25 分钟、短休息 5 分钟，每完成 4 次专注后进入 15 分钟长休息
- 按下摇杆：开始、暂停、继续；阶段完成后进入下一阶段
- 向下推动摇杆：重置当前阶段并回到 `READY`
- TFT 显示当前阶段、剩余时间、运行状态和已完成专注次数
- 局部刷新屏幕，避免倒计时过程中整屏闪烁

当前版本不发声、不联网，也不保存断电前进度。重新上电会从第一次专注开始。

## 硬件与接线

| 模块引脚 | ESP32 开发板丝印 | GPIO / 电源 |
| --- | --- | --- |
| TFT GND | `GND` | GND |
| TFT VCC | `3V3` | 3.3V |
| TFT SCL | `D18` | GPIO18，SPI SCK |
| TFT SDA | `D23` | GPIO23，SPI MOSI |
| TFT RST | `RX2` | GPIO16 |
| TFT DC | `TX2` | GPIO17 |
| TFT CS | `D5` | GPIO5 |
| TFT BL | `3V3` | 3.3V，与 TFT VCC 共用电源轨 |
| 摇杆 GND | `GND` | GND |
| 摇杆 `5V` | `3V3` | 使用 3.3V 供电，不接 5V |
| 摇杆 SW | `D32` | GPIO32 |
| 摇杆 VRy | `D33` | GPIO33，模拟输入 |

摇杆 `VRx` 暂未使用。ESP32 GPIO 不耐受 5V；接线或改线前先拔掉 USB。

## 操作方式

| 当前状态 | 操作 | 结果 |
| --- | --- | --- |
| `READY` | 按下摇杆 | 开始倒计时 |
| `RUNNING` | 按下摇杆 | 暂停 |
| `PAUSED` | 按下摇杆 | 继续 |
| `DONE` | 按下摇杆 | 进入下一阶段的 `READY` 状态 |
| 任意状态 | 向下推动摇杆 | 重置当前阶段，不清除已完成次数 |

## 构建与烧录

项目使用 PlatformIO。若终端已配置 `pio`：

```powershell
pio run -e esp32dev
pio run -e esp32dev -t upload
pio device monitor --baud 115200
```

若 PowerShell 找不到 `pio`，使用完整路径：

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -e esp32dev -t upload
```

打开串口监视器时会占用 COM 端口，重新上传前先按 `Ctrl+C` 退出监视器。

## 脱离电脑运行

烧录完成后，程序保存在 ESP32 的 Flash 中。拔掉电脑 USB，再使用手机充电器或充电宝通过开发板 USB 接口供电，即可自动启动。

断电时设备不能运行；再次供电后计时和完成次数会重置。不要将裸锂电池直接接到开发板，电池供电需要充电、保护和稳压电路。

## 硬件测试程序

硬件测试位于 `test/`，每项都有独立 PlatformIO 环境，不需要覆盖主程序：

```powershell
pio run -e tft_display_test -t upload
pio run -e joystick_button_test -t upload
pio run -e joystick_axis_test -t upload
```

详见 [`test/README.md`](test/README.md)。

## 目录结构

```text
esp32-pomodoro/
|-- platformio.ini
|-- src/main.cpp
|-- test/
|   |-- tft_display/main.cpp
|   |-- joystick_button/main.cpp
|   `-- joystick_axis/main.cpp
`-- docs/
```

`docs/ESP32番茄钟制作指南.md` 是项目早期针对 SSD1306 OLED、双按键和 LED 的设计记录，当前实物实现以本 README 和 `src/main.cpp` 为准。

## 后续方向

- NVS 保存断电前状态或用户设置
- 可调专注与休息时长
- 蜂鸣器或振动提醒
- 外壳与可靠固定
- Wi-Fi 对时和统计页面
