# Hardware test programs

These are manual hardware checks, not Unity unit tests. Each test has its own
PlatformIO environment so it can be built or uploaded without changing
`src/main.cpp`.

## TFT display

```powershell
pio run -e tft_display_test
pio run -e tft_display_test -t upload
```

The display cycles through red, green, and blue, then shows `TFT OK`.

## Joystick button

```powershell
pio run -e joystick_button_test
pio run -e joystick_button_test -t upload
```

The display shows `RELEASED` on blue and `PRESSED` on green.

## Main application

```powershell
pio run -e esp32dev
pio run -e esp32dev -t upload
```

## Joystick Y axis

```powershell
pio run -e joystick_axis_test
pio run -e joystick_axis_test -t upload
pio device monitor --baud 115200
```

Record the values while the joystick is centered, pushed up, and pushed down.
