# BYUI EBadge

`BYUI EBadge` is a beginner-friendly Arduino library for the BYU-Idaho
E-Badge. It turns the built-in ST7789 display into a small text console.

## Student quick start

```cpp
#include <BYUI_EBadge.h>

void setup() {
  printf("Hello, e-Badge!\n");
  printf("Value: %6.2f\tHex: %04x\n", 3.30, 42);
}

void loop() {
}
```

No display object or `begin()` call is needed. The first `printf()`, `print()`,
`println()`, or `EBadge.clear()` initializes the LCD. Do not call these from a
global variable initializer; use them in `setup()` or `loop()`.

Both format-only statements such as `printf("Ready\n")` and formatted
statements such as `printf("Score: %d\n", score)` write to the LCD. For an
advanced sketch with a format string stored in a `const char*` variable,
`printf(format, value)` works as well.

The library also handles compiler-optimized `puts()` and `putchar()` forms,
so a plain newline-terminated message and a standalone `printf("\n")` both
reach the LCD.

## Reading from the Serial Monitor

`scanf()` starts `Serial` at 115200 baud on its first use. It waits for the
student to type one line in Arduino IDE's Serial Monitor and press Enter, then
echoes that response on the LCD and parses the line. It is intentionally
blocking: the rest of the sketch pauses while it waits for input.

```cpp
int age;
char name[20];

printf("Type your age: ");
if (scanf("%d", &age) == 1) {
  printf("Next year: %d\n", age + 1);
}

printf("Type your first name: ");
if (scanf("%19s", name) == 1) {
  printf("Hello, %s\n", name);
}
```

The input implementation supports `%d`, `%f`, `%c`, `%x`, `%s`, and widths.
For `%s`, always specify a width one smaller than the character-array size:
use `%19s` with `char name[20]`. A response longer than 127 characters is
discarded and `scanf()` returns `EOF`.

## Console behavior

- Uses the built-in display in landscape orientation at text size 2 by default.
- Wraps text at the right edge.
- Scrolls upward when text reaches the bottom.
- Treats `\n` as a new line, ignores `\r`, and moves to a four-column tab stop
  for `\t`.
- Supports the ESP32 Arduino core's `printf` formatting, including `%d`, `%f`,
  `%c`, `%x`, `%s`, width such as `%6d` or `%12s`, and precision such as `%.2f`.
- Uses the default fixed-width Adafruit font. At the default text size 2, the
  console holds 26 columns by 15 rows. Use `EBadge.setTextSize(1)` for a
  denser 53-column by 30-row console.
- Scrolls by transferring each buffered text row as one SPI block, avoiding
  the slow full-screen clear and per-character redraw used by the first version.

## Optional configuration

The global `EBadge` object is available for settings. These calls also start
the display if needed.

```cpp
EBadge.setTextSize(2);
EBadge.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
EBadge.clear();
```

The library intentionally routes global `printf()` to the LCD. Use
`Serial.printf()` when output should go to the computer instead; member calls
such as `Serial.printf()` and `tft.printf()` are unaffected because the library
does not use a preprocessor macro.

## Installation and updates

Install the whole `BYUI_EBadge` folder in the Arduino sketchbook's `libraries`
folder, or package it as a ZIP and use **Sketch > Include Library > Add .ZIP
Library**. Update the `version` field in `library.properties` and distribute a
new release ZIP when the library changes.

The included **01 LCDPrintf** example appears in Arduino IDE under
**File > Examples > BYUI EBadge** after installation.
