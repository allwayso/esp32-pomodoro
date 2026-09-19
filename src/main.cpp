#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

constexpr int TFT_CS = 5;
constexpr int TFT_DC = 17;
constexpr int TFT_RST = 16;
constexpr int BUTTON_PIN = 32;
constexpr bool TEST_MODE = true;
constexpr uint32_t FOCUS_MS = TEST_MODE ? 10000UL : 25UL * 60UL * 1000UL;
constexpr uint32_t SHORT_BREAK_MS = TEST_MODE ? 5000UL : 5UL * 60UL * 1000UL;
constexpr uint32_t LONG_BREAK_MS = TEST_MODE ? 8000UL : 15UL * 60UL * 1000UL;
constexpr uint32_t DEBOUNCE_MS = 30;

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

enum class TimerState {
  Ready,
  Running,
  Paused,
  Done
};

enum class Phase {
  Focus,
  ShortBreak,
  LongBreak
};

TimerState state = TimerState::Ready;
Phase phase = Phase::Focus;
uint32_t remainingMs = FOCUS_MS;
uint32_t completedFocus = 0;
uint32_t lastTick = 0;
bool buttonRaw = HIGH;
bool buttonStable = HIGH;
uint32_t buttonChangedAt = 0;
uint32_t displayedSeconds = UINT32_MAX;
TimerState displayedState = TimerState::Ready;
bool stateHasBeenDrawn = false;
Phase displayedPhase = Phase::Focus;
uint32_t displayedCompletedFocus = UINT32_MAX;

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

const char* phaseText() {
  switch (phase) {
    case Phase::Focus:
      return "FOCUS";
    case Phase::ShortBreak:
      return "SHORT BREAK";
    case Phase::LongBreak:
      return "LONG BREAK";
  }

  return "";
}

uint32_t phaseDuration() {
  switch (phase) {
    case Phase::Focus:
      return FOCUS_MS;
    case Phase::ShortBreak:
      return SHORT_BREAK_MS;
    case Phase::LongBreak:
      return LONG_BREAK_MS;
  }

  return FOCUS_MS;
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

void drawLayout() {
  tft.fillScreen(ST77XX_BLACK);
}

void drawHeader() {
  if (displayedCompletedFocus == completedFocus && displayedPhase == phase) {
    return;
  }

  displayedPhase = phase;
  displayedCompletedFocus = completedFocus;
  tft.fillRect(0, 15, 240, 55, ST77XX_BLACK);

  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  tft.setCursor(20, 18);
  tft.print(phaseText());
  if (TEST_MODE) {
    tft.print(" TEST");
  }

  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(20, 48);
  tft.print("COMPLETED: ");
  tft.print(completedFocus);
}

void drawTime() {
  const uint32_t seconds = (remainingMs + 999UL) / 1000UL;
  if (seconds == displayedSeconds) {
    return;
  }

  displayedSeconds = seconds;
  tft.fillRect(35, 85, 180, 60, ST77XX_BLACK);

  char timeText[8];
  snprintf(timeText, sizeof(timeText), "%02lu:%02lu",
           static_cast<unsigned long>(seconds / 60),
           static_cast<unsigned long>(seconds % 60));

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(5);
  tft.setCursor(43, 92);
  tft.print(timeText);
}

void drawState() {
  if (stateHasBeenDrawn && state == displayedState) {
    return;
  }

  displayedState = state;
  stateHasBeenDrawn = true;
  tft.fillRect(0, 165, 240, 45, ST77XX_BLACK);

  tft.setTextColor(stateColor());
  tft.setTextSize(3);
  tft.setCursor(48, 175);
  tft.print(stateText());
}

void updateDisplay() {
  drawHeader();
  drawTime();
  drawState();
}

void goToNextPhase() {
  if (phase == Phase::Focus) {
    phase = completedFocus % 4 == 0 ? Phase::LongBreak : Phase::ShortBreak;
  } else {
    phase = Phase::Focus;
  }

  remainingMs = phaseDuration();
  state = TimerState::Ready;
  displayedSeconds = UINT32_MAX;
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
      goToNextPhase();
      break;
  }

  Serial.println(stateText());
  updateDisplay();
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  SPI.begin(18, -1, 23, TFT_CS);
  tft.init(240, 240);
  tft.setRotation(0);

  lastTick = millis();
  drawLayout();
  updateDisplay();
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
      if (phase == Phase::Focus) {
        ++completedFocus;
      }
      Serial.println("DONE");
    } else {
      remainingMs -= elapsed;
    }
  }

  if (buttonPressed(now)) {
    handleButtonPress();
  }

  updateDisplay();
}
