#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

constexpr int TFT_CS = 5;
constexpr int TFT_DC = 17;
constexpr int TFT_RST = 16;
constexpr int BUTTON_PIN = 32;
constexpr uint32_t TIMER_MS = 10000;
constexpr uint32_t DEBOUNCE_MS = 30;
constexpr uint32_t DRAW_INTERVAL_MS = 100;

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

enum class TimerState {
  Ready,
  Running,
  Paused,
  Done
};

TimerState state = TimerState::Ready;
uint32_t remainingMs = TIMER_MS;
uint32_t lastTick = 0;
uint32_t lastDraw = 0;
bool buttonRaw = HIGH;
bool buttonStable = HIGH;
uint32_t buttonChangedAt = 0;

bool buttonPressed(uint32_t now) {
  const bool reading = digitalRead(BUTTON_PIN);

  if (reading != buttonRaw) {
    buttonRaw = reading;
    buttonChangedAt = now;
  }

  if (buttonRaw != buttonStable && now - buttonChangedAt >= DEBOUNCE_MS) {
    buttonStable = buttonRaw;
    return buttonStable == LOW;
  }

  return false;
}

const char* stateText() {
  switch (state) {
    case TimerState::Ready:
      return "READY";
    case TimerState::Running:
      return "RUNNING";
    case TimerState::Paused:
      return "PAUSED";
    case TimerState::Done:
      return "DONE";
  }

  return "";
}

uint16_t stateColor() {
  switch (state) {
    case TimerState::Ready:
      return ST77XX_CYAN;
    case TimerState::Running:
      return ST77XX_GREEN;
    case TimerState::Paused:
      return ST77XX_YELLOW;
    case TimerState::Done:
      return ST77XX_RED;
  }

  return ST77XX_WHITE;
}

void drawScreen() {
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(3);
  tft.setCursor(45, 30);
  tft.print("FOCUS");

  const uint32_t seconds = (remainingMs + 999UL) / 1000UL;
  char timeText[8];
  snprintf(timeText, sizeof(timeText), "%02lu:%02lu",
           static_cast<unsigned long>(seconds / 60),
           static_cast<unsigned long>(seconds % 60));

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(5);
  tft.setCursor(43, 92);
  tft.print(timeText);

  tft.setTextColor(stateColor());
  tft.setTextSize(3);
  tft.setCursor(48, 175);
  tft.print(stateText());
}

void handleButtonPress() {
  switch (state) {
    case TimerState::Ready:
    case TimerState::Paused:
      state = TimerState::Running;
      break;
    case TimerState::Running:
      state = TimerState::Paused;
      break;
    case TimerState::Done:
      remainingMs = TIMER_MS;
      state = TimerState::Ready;
      break;
  }

  Serial.println(stateText());
  drawScreen();
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  SPI.begin(18, -1, 23, TFT_CS);
  tft.init(240, 240);
  tft.setRotation(0);

  lastTick = millis();
  lastDraw = lastTick;
  drawScreen();
  Serial.println("Pomodoro countdown ready");
}

void loop() {
  const uint32_t now = millis();
  const uint32_t elapsed = now - lastTick;
  lastTick = now;

  if (state == TimerState::Running) {
    if (elapsed >= remainingMs) {
      remainingMs = 0;
      state = TimerState::Done;
      Serial.println("DONE");
      drawScreen();
    } else {
      remainingMs -= elapsed;
    }
  }

  if (buttonPressed(now)) {
    handleButtonPress();
  }

  if (now - lastDraw >= DRAW_INTERVAL_MS) {
    lastDraw = now;
    drawScreen();
  }
}
