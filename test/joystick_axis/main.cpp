#include <Arduino.h>

constexpr int JOYSTICK_Y_PIN = 33;
constexpr uint32_t SAMPLE_INTERVAL_MS = 100;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  pinMode(JOYSTICK_Y_PIN, INPUT);
  Serial.println("Joystick Y-axis test ready");
}

void loop() {
  Serial.println(analogRead(JOYSTICK_Y_PIN));
  delay(SAMPLE_INTERVAL_MS);
}
