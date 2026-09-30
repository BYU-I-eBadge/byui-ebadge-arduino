#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// A text console for the built-in 320 x 240 landscape E-Badge display.
// The class uses the default fixed-width Adafruit font so it can accurately
// wrap and scroll text one character cell at a time.
class BYUI_EBadgeConsole : public Print {
 public:
  BYUI_EBadgeConsole();

  // Starts the display the first time it is needed. Calling begin() yourself
  // is optional; printf(), print(), println(), and clear() start it too.
  bool begin();

  // Print-compatible output. Print::printf() supplies standard printf
  // formatting, including %d, %f, %c, %x, %s, width, and precision.
  using Print::printf;
  using Print::write;
  size_t write(uint8_t character) override;

  // Console appearance and control.
  void clear();
  void setTextSize(uint8_t size);
  void setTextColor(uint16_t foreground, uint16_t background = ST77XX_BLACK);
  void setTabWidth(uint8_t columns);
  bool isReady() const;

  // Advanced use only. Drawing directly to this display does not update the
  // console's text buffer. Call clear() before returning to console output.
  Adafruit_ST7789 &display();

 private:
  static const uint8_t kMaxColumns = 53;  // 320 / 6 at text size 1
  static const uint8_t kMaxRows = 30;     // 240 / 8 at text size 1

  Adafruit_ST7789 tft_;
  char lines_[kMaxRows][kMaxColumns + 1];
  uint16_t foreground_;
  uint16_t background_;
  uint8_t textSize_;
  uint8_t tabWidth_;
  uint8_t columns_;
  uint8_t rows_;
  uint8_t column_;
  uint8_t row_;
  bool started_;
  GFXcanvas16 *lineCanvas_;
  uint16_t canvasWidth_;
  uint16_t canvasHeight_;

  void updateLayout();
  void clearBuffer();
  void drawCell(uint8_t row, uint8_t column, char character);
  bool ensureLineCanvas();
  void drawBufferedRow(uint8_t row);
  void redrawSlow();
  void redraw();
  void newLine();
  void scrollUp();
  void putCharacter(char character);
};

// The built-in E-Badge LCD console. Most beginner sketches only need printf().
extern BYUI_EBadgeConsole EBadge;

// The library supplies the global printf() implementation so ordinary
// printf("text") and printf("value: %d", value) both write to the LCD.
// This is a real function, not a macro, so Serial.printf(...) and tft.printf(...)
// remain valid.
extern "C" int printf(const char *format, ...);

// GCC may optimize printf("text\\n") into puts("text").  Providing puts()
// too makes plain text output use the LCD console just like formatted output.
extern "C" int puts(const char *text);

// GCC may optimize printf("\\n") into putchar('\\n'), or split a string
// with a leading newline into putchar() and puts(). Route that form to the
// LCD console too.
extern "C" int putchar(int character);

// Reads one Enter-terminated line from the Arduino Serial Monitor and parses
// it with standard scanf-style specifiers. This function blocks while waiting
// for the student to type a line. Serial starts automatically at 115200 baud.
extern "C" int scanf(const char *format, ...);

// Forces the linker to include the implementation that owns the global printf
// function, even in a sketch that only calls printf() and never mentions EBadge.
extern "C" void BYUI_EBadge_forceLink();
namespace byui_ebadge_internal {
static void (*const forceLink)() __attribute__((used)) = BYUI_EBadge_forceLink;
}
