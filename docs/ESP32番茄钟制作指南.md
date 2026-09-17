# 从零制作 ESP32 桌面番茄钟

整理日期：2026-09-15。适用于会 C/C++、刚开始接触硬件的学习者。

目标：做出一个脱离电脑也能运行的 USB 供电番茄钟。使用 ESP32、OLED 屏幕、两个按钮和一个提示灯，全程使用面包板免焊接。先完成可靠的小设备，再考虑外壳、声音和联网。

> 本文代码按下述硬件与 Arduino 库编写，尚未经过实物烧录验证。请按分阶段检查和验收步骤排查，不要一次接完所有器件再找问题。

## 1. 成品有哪些功能

- 默认专注 25 分钟，短休息 5 分钟。
- 完成 4 次专注后，安排 15 分钟长休息。
- A 键：开始 / 暂停 / 继续。
- B 键：将当前阶段恢复到完整时长，并回到待开始状态。
- 时间到：屏幕显示 `DONE`，LED 闪烁提醒；按 A 确认，进入下一阶段的待开始状态，再按 A 开始。
- 屏幕显示阶段、剩余时间、运行状态和已完成专注次数。

第一版不发声、不联网、不保存断电前进度。重新供电会回到第一次专注。待开始、暂停和结束等待确认时，都不会自动扣时间。

## 2. 完整硬件清单

以下参考金额只是采购预算估计，不是实时商品报价。优先买有清晰引脚图和售后资料的器件。

| 器材 | 数量 | 购买规格 / 搜索词 | 参考预算 |
| --- | --- | --- | --- |
| ESP32 开发板 | 1 | **经典 ESP32-WROOM-32 / ESP32-WROOM-32E，ESP32 DevKit 类开发板，排针已焊接**；需引出 GPIO 18、19、21、22、23 | 20～45 元 |
| OLED 显示模块 | 1 | **0.96 英寸、128×64、SSD1306、I²C 四针、支持 3.3V 供电、排针已焊接**；四针为 GND/VCC/SCL/SDA | 10～25 元 |
| 830 孔面包板 | 2 | 两块配合使用，避免较宽 ESP32 开发板插入后遮住所有接线孔 | 合计 10～20 元 |
| 杜邦线 | 公对母、公对公各 20 根 | 约 20cm；实际接线用量少于总数，预留替换 | 合计 8～15 元 |
| 轻触按钮 | 2，建议买 5 个 | 6×6mm 四脚、直插、常开、自复位；不要买贴片型 | 1～5 元 |
| 红色 LED | 1，建议买 5 个 | 3mm 或 5mm 两脚直插普通 LED | 1～3 元 |
| 电阻 | 1，建议买 10 个 | **1kΩ，1/4W，直插**，LED 限流用 | 1～3 元 |
| USB 数据线 | 1 | 接口与开发板一致，必须支持数据传输；Type-C 或 Micro-USB 按板子选 | 5～15 元 |
| USB 供电适配器 | 1 | 正规 5V、额定输出至少 1A，有兼容 USB 插口；已有充电头可复用 | 10～25 元 |

总预算约 **70～160 元**，随店铺、运费和已有器材变化。下载程序时由电脑 USB 供电即可，独立运行才需要充电头。额定 1A 是电源可提供的能力，并不表示设备持续消耗 1A。

可选工具：带通断蜂鸣档的数字万用表，用来检查按钮脚和接线；第一版不需要电烙铁、电池、蜂鸣器、电机驱动或额外的按钮上拉电阻。

### 采购前确认

- 本文固定采用**经典 ESP32**。先不要替换成 C3、S3 或 ESP32-CAM：它们也能做番茄钟，但引脚、板型和 USB 设置不同。
- OLED 不要买成 SH1106、128×32 或 SPI 七针版本，否则显示库配置或接线需要修改。
- 开发板和 OLED 都购买已焊排针版本，才能免焊接完成。
- 不同厂家开发板的排针顺序不一致，只按丝印和商家引脚图接线，不能数“第几个孔”。

## 3. 先认识几个必要概念

| 名称 | 在这个项目中的作用 |
| --- | --- |
| 3V3 | 给 OLED 提供 3.3V 电源 |
| GND | 公共地，所有模块需要共享参考电位 |
| GPIO | 程序可读取或控制的芯片引脚 |
| I²C | 屏幕通信总线，SDA 传数据，SCL 传时钟 |
| INPUT_PULLUP | 使用芯片内部上拉电阻，按钮松开读 HIGH，按下接地读 LOW |
| 消抖 | 忽略机械按钮接触瞬间的短暂跳变，避免按一次触发多次 |
| millis() | 上电以来的毫秒计数，配合时间差实现非阻塞倒计时 |

ESP32 GPIO 不应直接接 5V。本文仅从开发板 USB 接口供电，OLED 接 3V3；改变接线前拔掉 USB，不同时另接外部电源到供电针脚。

## 4. 安装开发环境，先验证裸板

1. 安装 [Arduino IDE](https://www.arduino.cc/en/software)。可参考 [Arduino IDE 2 的 ESP32 安装图文教程](https://randomnerdtutorials.com/installing-esp32-arduino-ide-2-0/)。
2. 在 IDE 的首选项“附加开发板管理器网址”中加入下面的地址；已有其他地址时另加一项，不覆盖它们。

   ```text
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```

3. 打开开发板管理器，搜索 `esp32`，安装 **esp32 by Espressif Systems** 的稳定版本，记录所用版本。
4. 使用数据线连接开发板。对于本文选用的通用经典 ESP32 DevKit，可选择 **ESP32 Dev Module**；如果厂商明确指定其他板型，则按厂商说明设置。
5. 选择对应 COM 端口。可以通过插拔开发板，观察新增或消失的端口来确认。
6. 新建草图，保存为 `Pomodoro`，先上传下面的串口测试代码：

   ```cpp
   void setup() {
     Serial.begin(115200);
   }
   void loop() {
     Serial.println("ESP32 OK");
     delay(1000);
   }
   ```

7. 打开串口监视器，波特率选 `115200`。每秒看到 `ESP32 OK`，说明编译、下载和运行链路已经打通。

没有 COM 端口时，先换数据线和 USB 口，再根据板子上的 USB 转串口芯片型号安装厂商驱动，例如 CP210x 或 CH340。一直显示 `Connecting...` 时，可尝试在连接阶段按住 BOOT，开始写入后松开。上传结束不运行时按一下 EN / RESET。

## 5. 接线：先屏幕，再按钮和灯

### 5.1 面包板怎么使用

常见面包板中，每侧同一编号的一组 5 孔内部相连，中间沟槽左右不相连。长电源轨可能在中部断开，两侧电源轨也不一定互通。用万用表或面包板资料确认，不要仅凭红蓝颜色假定连通。

初学时可以把 ESP32 放在不导电的桌面上，用**母头接开发板排针，公头插面包板**。这样不必将较宽的开发板硬插进面包板。屏幕、按钮和电阻插在面包板上，再用公对公线连接。未使用的开发板排针不要碰到金属物品。

### 5.2 OLED 接线

拔掉 USB 后连接：

| OLED 引脚 | ESP32 引脚 |
| --- | --- |
| GND | GND |
| VCC | 3V3 |
| SDA | GPIO 21，可能标为 D21 |
| SCL | GPIO 22，可能标为 D22 |

**不同屏幕的 VCC、GND 排列可能相反，一定看丝印。**

在 Arduino IDE 的库管理器安装：

- `Adafruit SSD1306`
- `Adafruit GFX Library`
- 如果提示依赖 `Adafruit BusIO`，一并安装。

### 5.3 扫描屏幕地址

先上传这段代码，只接屏幕，打开 115200 波特率串口监视器，然后按 EN / RESET：

```cpp
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  delay(1000);
  int found = 0;
  for (uint8_t address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.printf("I2C device: 0x%02X\n", address);
      ++found;
    }
  }
  Serial.printf("Found %d device(s)\n", found);
}
void loop() {}
```

通常会看到 `0x3C`，也可能是 `0x3D`。记下地址，后续填入完整程序的 `OLED_ADDR`。如果显示找到 0 个设备，先检查供电、共地及 SDA/SCL，暂时不要继续。

### 5.4 按钮接线

| 器件 | 一端 | 另一端 |
| --- | --- | --- |
| A 按钮：开始 / 暂停 | GPIO 18 | GND |
| B 按钮：重置当前阶段 | GPIO 19 | GND |

四脚按钮实际通常只有两组电气连接：每组两个脚内部一直相通，按下才接通两组。用通断档确认，选择**松开不通、按下才通**的一对脚。不要把 GPIO 和 GND 接到本来就连通的一组，也不要让面包板同一组孔提前将两端短接。

按钮不用连接 3V3，程序会启用内部上拉。

### 5.5 LED 接线

```text
GPIO 23 ── 1kΩ 电阻 ── LED 正极（通常长脚）
                          LED 负极（通常短脚、外壳平边）── GND
```

电阻串在 LED 任意一侧均可，但不能省略。剪过脚的 LED 不能只靠脚长判断极性。

### 5.6 独立测试按钮与 LED

接好后先上传下面的程序。按 A 时 LED 应点亮，松开应熄灭；串口中 A/B 松开为 1，按下为 0。

```cpp
void setup() {
  Serial.begin(115200);
  pinMode(18, INPUT_PULLUP);
  pinMode(19, INPUT_PULLUP);
  pinMode(23, OUTPUT);
}
void loop() {
  int a = digitalRead(18);
  int b = digitalRead(19);
  digitalWrite(23, a == LOW ? HIGH : LOW);
  Serial.printf("A=%d B=%d\n", a, b);
  delay(100);
}
```

## 6. 上传完整番茄钟程序

用下面的代码**完整替换**草图原内容，不要与前面的 `setup()` / `loop()` 拼接。确认 `OLED_ADDR` 与扫描结果一致。

首次运行保留 `TEST_MODE = true`：专注 10 秒、短休息 5 秒、长休息 8 秒，方便验证全部流程。验收完成后改成 `false`，重新上传才是正式时长。

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

constexpr bool TEST_MODE = true;  // 验收完成后改为 false
constexpr uint8_t OLED_ADDR = 0x3C;
constexpr uint8_t LED_PIN = 23;
constexpr uint32_t FOCUS_MS = TEST_MODE ? 10000UL : 25UL * 60 * 1000;
constexpr uint32_t SHORT_MS = TEST_MODE ? 5000UL : 5UL * 60 * 1000;
constexpr uint32_t LONG_MS = TEST_MODE ? 8000UL : 15UL * 60 * 1000;

Adafruit_SSD1306 display(128, 64, &Wire, -1);

// 每次稳定的按下只产生一个事件，按住不连发。
struct Button {
  uint8_t pin;
  bool raw = HIGH;
  bool stable = HIGH;
  uint32_t changedAt = 0;

  explicit Button(uint8_t p) : pin(p) {}

  void begin() {
    pinMode(pin, INPUT_PULLUP);
    raw = stable = digitalRead(pin);
    changedAt = millis();
  }

  bool pressed(uint32_t now) {
    bool reading = digitalRead(pin);
    if (reading != raw) {
      raw = reading;
      changedAt = now;
    }
    if (raw != stable && uint32_t(now - changedAt) >= 30) {
      stable = raw;
      return stable == LOW;
    }
    return false;
  }
};

Button buttonA(18);
Button buttonB(19);

enum class Phase { Focus, ShortBreak, LongBreak };
enum class State { Ready, Running, Paused, Done };
uint32_t durationFor(Phase p);  // 显式声明，避免 IDE 自动原型早于枚举定义

Phase phase = Phase::Focus;
State state = State::Ready;
uint32_t remainingMs = FOCUS_MS;
uint32_t completedFocus = 0;
uint32_t lastTick = 0;
uint32_t lastDraw = 0;
uint32_t alarmStartedAt = 0;

uint32_t durationFor(Phase p) {
  if (p == Phase::Focus) return FOCUS_MS;
  if (p == Phase::ShortBreak) return SHORT_MS;
  return LONG_MS;
}

const char* phaseLabel() {
  if (phase == Phase::Focus) return "FOCUS";
  if (phase == Phase::ShortBreak) return "SHORT BREAK";
  return "LONG BREAK";
}

void drawScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(phaseLabel());
  if (TEST_MODE) display.print(" TEST");

  // 向上取整：剩 24 分 59.9 秒时仍显示 25:00。
  uint32_t seconds = (remainingMs + 999UL) / 1000UL;
  char timeText[16];
  snprintf(timeText, sizeof(timeText), "%02lu:%02lu",
           (unsigned long)(seconds / 60),
           (unsigned long)(seconds % 60));
  display.setTextSize(3);
  display.setCursor(18, 14);
  display.print(timeText);

  display.setTextSize(1);
  display.setCursor(0, 42);
  if (state == State::Ready) display.print("READY");
  else if (state == State::Running) display.print("RUNNING");
  else if (state == State::Paused) display.print("PAUSED");
  else display.print("DONE");
  display.print("  #");
  display.print(completedFocus);

  display.setCursor(0, 55);
  if (state == State::Done) display.print("A:Next  B:Repeat");
  else if (state == State::Running) display.print("A:Pause B:Reset");
  else display.print("A:Start B:Reset");
  display.display();
}

void resetCurrentPhase() {
  remainingMs = durationFor(phase);
  state = State::Ready;
}

void goToNextPhase() {
  if (phase == Phase::Focus) {
    phase = (completedFocus % 4 == 0)
              ? Phase::LongBreak : Phase::ShortBreak;
  } else {
    phase = Phase::Focus;
  }
  resetCurrentPhase();
}

void setup() {
  Serial.begin(115200);
  buttonA.begin();
  buttonB.begin();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED init failed");
    while (true) delay(1000);
  }
  display.setTextWrap(false);
  lastTick = millis();
  drawScreen();
}

void loop() {
  uint32_t now = millis();
  uint32_t elapsed = now - lastTick;  // 无符号差值可处理 millis 回绕
  lastTick = now;

  // 先计算刚过去的时间，再处理按钮，避免暂停时间被扣除。
  if (state == State::Running) {
    if (elapsed >= remainingMs) {
      remainingMs = 0;
      state = State::Done;
      alarmStartedAt = now;
      if (phase == Phase::Focus) ++completedFocus;
    } else {
      remainingMs -= elapsed;
    }
  }

  bool aPressed = buttonA.pressed(now);
  bool bPressed = buttonB.pressed(now);

  // 同时按下时 B 优先。已完成的专注计数不会被 B 撤销。
  if (bPressed) {
    resetCurrentPhase();
  } else if (aPressed) {
    if (state == State::Done) goToNextPhase();
    else if (state == State::Running) state = State::Paused;
    else state = State::Running;
  }

  bool alarmOn = state == State::Done &&
                ((uint32_t(now - alarmStartedAt) / 250UL) % 2 == 0);
  digitalWrite(LED_PIN, alarmOn ? HIGH : LOW);

  if (uint32_t(now - lastDraw) >= 100) {
    lastDraw = now;
    drawScreen();
  }
}
```

屏幕使用英文是为了直接使用库自带字体，避免第一版就引入中文字库和字体存储问题。测试程序中的 `delay()` 用于隔离验证；正式主循环使用时间差计时，能持续响应按钮。

## 7. 按这个清单验收

保持测试模式，逐项完成：

| 操作 | 预期结果 |
| --- | --- |
| 重新上电 | 显示 FOCUS、TEST、00:10、READY、#0 |
| 按 A | 开始倒计时，显示 RUNNING |
| 中途按 A，等几秒 | 显示 PAUSED，剩余时间不变 |
| 再按 A | 从暂停位置继续，不扣除暂停期间的时间 |
| 按 B | 当前阶段恢复完整时长，显示 READY |
| 让专注自然结束 | 显示 00:00、DONE、#1，LED 闪烁 |
| 等待几秒，不按按钮 | 保持 DONE，不自动开始休息 |
| 按 A | 进入 SHORT BREAK、00:05、READY，LED 熄灭 |
| 再按 A，等待休息结束，再按 A | 返回 FOCUS 的待开始状态 |
| 正常完成第 4 次专注，再按 A | 进入 LONG BREAK，测试时长 00:08 |
| 长按 A 不松开 | 只触发一次动作，不反复暂停 / 开始 |
| 拔电重新上电 | 回到第一次专注，完成次数清零 |

`#` 记录自然完成的专注次数。B 不清除这个累计数；在 DONE 状态按 B 会重新准备当前阶段，重复完成的专注也会计数。需要从第一个番茄重新开始时按 EN / RESET。

验收后将 `TEST_MODE` 改成 `false`，重新烧录。检查屏幕不再显示 TEST，第一次专注显示 `25:00`；再至少实际运行一轮，与手机计时做粗略对照。该方案足够日常计时，但不是精密时钟。

## 8. 脱离电脑运行

1. 拔掉电脑 USB，改接 5V USB 充电头。
2. 开发板会自动运行已烧录的程序，不需要 Arduino IDE 或电脑参与。
3. 保持底部绝缘，确认杜邦线不会轻易松动。

普通充电宝可能因负载太小自动关机，第一版优先使用 USB 充电头。面包板适合原型验证；准备长期使用时，再考虑焊接到洞洞板或制作 PCB，并加外壳固定。

## 9. 常见问题

| 现象 | 优先检查 |
| --- | --- |
| OLED 黑屏 | 扫描是否找到地址；地址是否填对；SDA/SCL 是否反接；是否确为 SSD1306 128×64 |
| OLED 黑屏但没有初始化报错 | `display.begin()` 成功不代表屏幕一定在线，重新跑 I²C 扫描确认物理连接 |
| 找不到 `Adafruit_SSD1306.h` | 在当前 Arduino IDE 库管理器安装相应库和依赖 |
| 按钮始终显示按下 | 按钮用了内部常通的两脚，或面包板将两端接到同一组孔 |
| 按钮毫无反应 | 是否共地；是否接 GPIO 18/19；是否认错丝印与物理脚位 |
| LED 不亮 | 极性是否接反；是否接 GPIO 23；先跑独立测试，正式程序仅在 DONE 状态闪烁 |
| 一直无法上传 | 数据线、COM 口、板型、BOOT 操作；必要时降低上传速度 |
| 接屏幕后反复重启 | 先断电检查电源短路和 VCC/GND，之后再尝试更可靠的线缆和电源 |
| 时间只有 10 秒 | 测试模式尚未关闭，修改后需要重新上传 |

## 10. 完成后再扩展

建议依次加入：可设置专注时长 → NVS 保存用户设置 → 蜂鸣器提醒 → 外壳和可靠固定 → Wi-Fi 时间同步或统计网页。

蜂鸣器不是本版必需器材。扩展声音时需按具体蜂鸣器的工作电压与电流选择驱动方式，不要直接把任意蜂鸣器接到 GPIO。电池供电也需要单独设计充电、保护和稳压方案。

## 参考资料

- [Random Nerd Tutorials：ESP32 入门](https://randomnerdtutorials.com/getting-started-with-esp32/)：开发板与开发环境概览。
- [Random Nerd Tutorials：Arduino IDE 2 安装 ESP32](https://randomnerdtutorials.com/installing-esp32-arduino-ide-2-0/)：环境安装的图文步骤。
- [Random Nerd Tutorials：ESP32 驱动 SSD1306 OLED](https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/)：屏幕接线、Adafruit 库安装和显示示例，可与第 5 节对照阅读。
- [Arduino-ESP32 官方文档](https://docs.espressif.com/projects/arduino-esp32/en/latest/getting_started.html)：开发板支持、安装和接口参考。
- [太极创客 Arduino 基础视频](https://www.bilibili.com/video/BV164411J7GE/)：按需补看面包板、LED 和按钮章节，接线以本文 ESP32 方案为准。
- 本仓库的 [ESP32 入门学习资料](ESP32入门学习资料.md)。

本文程序为本项目编写；外部资源用于补充概念与操作说明，不要求先看完所有教程。
