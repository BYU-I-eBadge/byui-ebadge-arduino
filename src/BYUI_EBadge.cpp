#include "BYUI_EBadge.h"

#include <stdio.h>
#include <stdarg.h>

namespace {
constexpr int kTftCs = 0;
constexpr int kTftReset = 1;
constexpr int kTftDc = 45;
constexpr int kTftMosi = 3;
constexpr int kTftSclk = 46;
constexpr uint8_t kDefaultTextSize = 2;
constexpr uint8_t kDefaultTabWidth = 4;
constexpr unsigned long kDefaultSerialBaud = 115200;
constexpr size_t kSerialInputBufferSize = 128;

bool serialStarted = false;

// Reads one complete line. Carriage-return/line-feed pairs from the Serial
// Monitor count as one Enter key press. Extra characters are discarded so a
// too-long line cannot overrun the fixed input buffer.
bool readSerialLine(char *buffer, size_t bufferSize) {
  size_t length = 0;
  bool hasText = false;
  bool overflowed = false;

  while (true) {
    while (Serial.available() == 0) {
      delay(1);
    }

    const int received = Serial.read();
    if (received < 0) {
      continue;
    }

    const char character = static_cast<char>(received);
    if (character == '\r' || character == '\n') {
      // Ignore the second character of CR/LF, and any leftover newline before
      // the next response. A blank line still waits for the next response.
      if (!hasText) {
        continue;
      }
      // Echo the completed Serial Monitor response to the LCD so a prompt and
      // its response appear together like a terminal console.
      EBadge.write('\n');
      break;
    }

    hasText = true;
    EBadge.write(character);
    if (length + 1 < bufferSize) {
      buffer[length++] = character;
    } else {
      overflowed = true;
    }
  }

  buffer[length] = '\0';
  return !overflowed;
}
}

BYUI_EBadgeConsole EBadge;

extern "C" void BYUI_EBadge_forceLink() {
}

// Replace the usual C-library stdout target with the E-Badge LCD console.
// Print::vprintf() uses vsnprintf() and then sends the formatted characters to
// this object's virtual write() method, which provides wrapping and scrolling.
extern "C" int printf(const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  size_t written = EBadge.vprintf(format, arguments);
  va_end(arguments);
  return static_cast<int>(written);
}

// The compiler commonly turns printf("literal\\n") into puts("literal").
// Keep that optimized form on the LCD console as well.
extern "C" int puts(const char *text) {
  if (text == nullptr) {
    return EOF;
  }

  size_t written = EBadge.print(text);
  written += EBadge.write('\n');
  return static_cast<int>(written);
}

// Keep compiler-optimized single-character output, especially a newline, on
// the LCD console. A newline is handled by BYUI_EBadgeConsole::write().
extern "C" int putchar(int character) {
  EBadge.write(static_cast<uint8_t>(character));
  return character;
}

// Read an entire Serial Monitor response before parsing it. This gives each
// scanf() call a predictable beginner-friendly interaction: prompt, type a
// response, then press Enter.
extern "C" int scanf(const char *format, ...) {
  if (!serialStarted) {
    Serial.begin(kDefaultSerialBaud);
    serialStarted = true;
  }

  char input[kSerialInputBufferSize];
  if (!readSerialLine(input, sizeof(input))) {
    return EOF;
  }

  va_list arguments;
  va_start(arguments, format);
  const int assignments = vsscanf(input, format, arguments);
  va_end(arguments);
  return assignments;
}

BYUI_EBadgeConsole::BYUI_EBadgeConsole()
    : tft_(kTftCs, kTftDc, kTftReset),
      foreground_(ST77XX_WHITE),
      background_(ST77XX_BLACK),
      textSize_(kDefaultTextSize),
      tabWidth_(kDefaultTabWidth),
      columns_(kMaxColumns),
      rows_(kMaxRows),
      column_(0),
      row_(0),
      started_(false),
      lineCanvas_(nullptr),
      canvasWidth_(0),
      canvasHeight_(0) {
  clearBuffer();
}

bool BYUI_EBadgeConsole::begin() {
  if (started_) {
    return true;
  }

  SPI.begin(kTftSclk, -1, kTftMosi);
  tft_.init(240, 320);
  tft_.setRotation(1);  // Landscape: 320 x 240 pixels.
  tft_.setTextWrap(false);
  started_ = true;
  updateLayout();
  clearBuffer();
  tft_.fillScreen(background_);
  return true;
}

void BYUI_EBadgeConsole::clear() {
  begin();
  clearBuffer();
  column_ = 0;
  row_ = 0;
  tft_.fillScreen(background_);
}

void BYUI_EBadgeConsole::setTextSize(uint8_t size) {
  if (size == 0) {
    size = 1;
  }

  textSize_ = size;
  if (started_) {
    updateLayout();
    clear();
  }
}

void BYUI_EBadgeConsole::setTextColor(uint16_t foreground,
                                       uint16_t background) {
  foreground_ = foreground;
  background_ = background;
  if (started_) {
    redraw();
  }
}

void BYUI_EBadgeConsole::setTabWidth(uint8_t columns) {
  tabWidth_ = columns == 0 ? 1 : columns;
}

bool BYUI_EBadgeConsole::isReady() const {
  return started_;
}

Adafruit_ST7789 &BYUI_EBadgeConsole::display() {
  begin();
  return tft_;
}

size_t BYUI_EBadgeConsole::write(uint8_t character) {
  begin();

  if (character == '\r') {
    return 1;  // println() emits \r\n; the \r is not a second line break.
  }

  if (character == '\n') {
    newLine();
    return 1;
  }

  if (character == '\t') {
    uint8_t spaces = tabWidth_ - (column_ % tabWidth_);
    while (spaces-- > 0) {
      putCharacter(' ');
    }
    return 1;
  }

  putCharacter(static_cast<char>(character));
  return 1;
}

void BYUI_EBadgeConsole::updateLayout() {
  const uint16_t characterWidth = 6 * textSize_;
  const uint16_t characterHeight = 8 * textSize_;

  columns_ = tft_.width() / characterWidth;
  rows_ = tft_.height() / characterHeight;

  if (columns_ > kMaxColumns) {
    columns_ = kMaxColumns;
  }
  if (rows_ > kMaxRows) {
    rows_ = kMaxRows;
  }
  if (columns_ == 0) {
    columns_ = 1;
  }
  if (rows_ == 0) {
    rows_ = 1;
  }

  // The cached canvas must match the current text height.
  if (lineCanvas_ != nullptr) {
    delete lineCanvas_;
    lineCanvas_ = nullptr;
    canvasWidth_ = 0;
    canvasHeight_ = 0;
  }
}

void BYUI_EBadgeConsole::clearBuffer() {
  for (uint8_t row = 0; row < kMaxRows; row++) {
    for (uint8_t column = 0; column < kMaxColumns; column++) {
      lines_[row][column] = ' ';
    }
    lines_[row][kMaxColumns] = '\0';
  }
}

void BYUI_EBadgeConsole::drawCell(uint8_t row, uint8_t column,
                                  char character) {
  const int16_t x = column * 6 * textSize_;
  const int16_t y = row * 8 * textSize_;
  tft_.drawChar(x, y, character, foreground_, background_, textSize_);
}

bool BYUI_EBadgeConsole::ensureLineCanvas() {
  const uint16_t width = tft_.width();
  const uint16_t height = 8 * textSize_;

  if (lineCanvas_ != nullptr && canvasWidth_ == width &&
      canvasHeight_ == height) {
    return true;
  }

  if (lineCanvas_ != nullptr) {
    delete lineCanvas_;
    lineCanvas_ = nullptr;
  }

  lineCanvas_ = new GFXcanvas16(width, height);
  if (lineCanvas_ == nullptr || lineCanvas_->getBuffer() == nullptr) {
    delete lineCanvas_;
    lineCanvas_ = nullptr;
    canvasWidth_ = 0;
    canvasHeight_ = 0;
    return false;
  }

  canvasWidth_ = width;
  canvasHeight_ = height;
  lineCanvas_->setTextWrap(false);
  return true;
}

void BYUI_EBadgeConsole::drawBufferedRow(uint8_t row) {
  lineCanvas_->fillScreen(background_);
  lineCanvas_->setTextColor(foreground_, background_);
  lineCanvas_->setTextSize(textSize_);
  lineCanvas_->setCursor(0, 0);

  // Print only the visible character cells for this row.
  char saved = lines_[row][columns_];
  lines_[row][columns_] = '\0';
  lineCanvas_->print(lines_[row]);
  lines_[row][columns_] = saved;

  tft_.startWrite();
  tft_.setAddrWindow(0, row * canvasHeight_, canvasWidth_, canvasHeight_);
  tft_.writePixels(lineCanvas_->getBuffer(),
                   static_cast<uint32_t>(canvasWidth_) * canvasHeight_);
  tft_.endWrite();
}

void BYUI_EBadgeConsole::redrawSlow() {
  tft_.fillScreen(background_);
  for (uint8_t row = 0; row < rows_; row++) {
    for (uint8_t column = 0; column < columns_; column++) {
      const char character = lines_[row][column];
      if (character != ' ') {
        drawCell(row, column, character);
      }
    }
  }
}

void BYUI_EBadgeConsole::redraw() {
  begin();
  if (!ensureLineCanvas()) {
    redrawSlow();
    return;
  }

  // Each visible text row is transferred as one contiguous SPI block. This
  // avoids clearing the display and redrawing hundreds of individual glyphs
  // every time the console scrolls.
  for (uint8_t row = 0; row < rows_; row++) {
    drawBufferedRow(row);
  }
}

void BYUI_EBadgeConsole::newLine() {
  column_ = 0;
  row_++;
  if (row_ >= rows_) {
    scrollUp();
    row_ = rows_ - 1;
  }
}

void BYUI_EBadgeConsole::scrollUp() {
  for (uint8_t row = 0; row < rows_ - 1; row++) {
    for (uint8_t column = 0; column < kMaxColumns; column++) {
      lines_[row][column] = lines_[row + 1][column];
    }
  }

  for (uint8_t column = 0; column < kMaxColumns; column++) {
    lines_[rows_ - 1][column] = ' ';
  }
  redraw();
}

void BYUI_EBadgeConsole::putCharacter(char character) {
  if (column_ >= columns_) {
    newLine();
  }

  lines_[row_][column_] = character;
  drawCell(row_, column_, character);
  column_++;
}
