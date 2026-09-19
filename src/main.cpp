#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

constexpr int TFT_CS = 5;    // D5
constexpr int TFT_DC = 17;   // TX2
constexpr int TFT_RST = 16;  // RX2

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);

  SPI.begin(18, -1, 23, 5);
  tft.init(240, 240);
  tft.setRotation(0);

  tft.fillScreen(ST77XX_RED);
  delay(1000);
  tft.fillScreen(ST77XX_GREEN);
  delay(1000);
  tft.fillScreen(ST77XX_BLUE);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.setCursor(55, 105);
  tft.print("TFT OK");
}

void loop() {
}
