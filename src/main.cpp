  #include <Arduino.h>
  #include <SPI.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_ST7789.h>

  constexpr int TFT_CS = 5;
  constexpr int TFT_DC = 17;
  constexpr int TFT_RST = 16;
  constexpr int BUTTON_PIN = 32;

  Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

  void drawButtonState(bool pressed) {
    tft.fillScreen(pressed ? ST77XX_GREEN : ST77XX_BLUE);

    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(3);
    tft.setCursor(35, 90);
    tft.print(pressed ? "PRESSED" : "RELEASED");
  }

  void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    SPI.begin(18, -1, 23, 5);
    tft.init(240, 240);
    tft.setRotation(0);

    drawButtonState(false);
    Serial.println("Joystick button test ready");
  }

  void loop() {
    static bool previousPressed = false;
    bool pressed = digitalRead(BUTTON_PIN) == LOW;

    if (pressed != previousPressed) {
      delay(30);
      pressed = digitalRead(BUTTON_PIN) == LOW;

      if (pressed != previousPressed) {
        previousPressed = pressed;
        drawButtonState(pressed);
        Serial.println(pressed ? "PRESSED" : "RELEASED");
      }
    }
  }