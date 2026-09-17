# ESP32 入门学习资料

整理日期：2026-09-15

适用背景：已有 C/C++、TypeScript、Python、Java 编程基础，希望通过 ESP32 制作能够独立运行的小设备。

建议路线：以 **Arduino 框架 + C++** 入门，用 Random Nerd Tutorials 做实践主线，用中文视频补电路基础，官方文档随用随查；完成一个小项目后，再学习 ESP-IDF。

## 1. Random Nerd Tutorials：ESP32 实践主线

- [ESP32 入门指南](https://randomnerdtutorials.com/getting-started-with-esp32/)
- [Arduino IDE 2 安装 ESP32 开发环境](https://randomnerdtutorials.com/installing-esp32-arduino-ide-2-0/)
- [ESP32 教程与项目目录](https://randomnerdtutorials.com/projects-esp32/)

英文图文教程，可配合浏览器翻译阅读。从开发板选择、环境安装、程序烧录开始，后续涵盖 GPIO、传感器、屏幕和网络应用，提供接线图与示例代码。适合已经会编程、但不熟悉硬件的学习者。

它是按主题组织的教程库，可按以下顺序挑选实验：

1. LED：数字输出和程序烧录。
2. 按钮：数字输入、上拉电阻和消抖。
3. ADC / PWM：模拟量读取和亮度控制。
4. I²C 传感器：外设通信和数据读取。
5. OLED：显示传感器数据。
6. Wi-Fi / Web Server：通过手机或电脑查看设备数据。

站内免费教程足够开始入门，暂时不需要购买付费课程。

## 2. 太极创客：中文电路与接线基础

- [零基础入门学用 Arduino：教程目录](http://www.taichi-maker.com/homepage/arduino-tutorial-index/)
- [B 站第一部分视频合辑](https://www.bilibili.com/video/BV164411J7GE/)

讲解细致，包含 LED、面包板、按钮、电位器、模拟输入和 PWM 等基础实验。已有 C/C++ 基础，可以跳过变量、循环、函数等语法章节，重点看硬件相关内容：

| 章节 | 内容 |
| --- | --- |
| 1-10～1-14 | LED、面包板、按钮及接线 |
| 1-25～1-30 | PWM、模拟输入、电位器控制 LED 亮度 |

这是 Arduino 基础课，不是 ESP32 专门课程。可以学习电路原理，但实际接线必须按所用 ESP32 开发板的引脚图调整。ESP32 GPIO 通常使用 3.3V 电平，不要直接接入 5V 信号。

## 3. Arduino-ESP32 官方文档：安装与 API 参考

- [官方入门入口](https://docs.espressif.com/projects/arduino-esp32/en/latest/getting_started.html)

由乐鑫维护，适合确认开发板支持情况、安装方式和接口用法，不必从头通读。

第三方教程出现编译错误时，先核对教程与本地使用的 **Arduino-ESP32 开发板包版本**。旧教程和新版之间可能存在 API 差异。Arduino IDE 版本与 Arduino-ESP32 开发板包版本是两回事。

## 4. ESP-IDF 官方中文指南：后续进阶

- [ESP-IDF 中文入门指南](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32/get-started/index.html)

ESP-IDF 是乐鑫官方开发框架。入门指南介绍开发环境配置、工程构建、固件烧录和日志查看，完整文档还包含外设、FreeRTOS、网络和电源管理等内容。

建议先使用 Arduino 框架完成传感器读取、屏幕显示等小项目，再进入 ESP-IDF。阅读时选择与开发板一致的芯片型号，并核对文档与安装框架的版本。

