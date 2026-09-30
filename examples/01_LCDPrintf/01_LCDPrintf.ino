/*
 * Example 01 - LCD printf() - BYU-Idaho E-Badge
 *
 * This version uses the BYUI_EBadge library. The LCD starts automatically
 * when printf() first writes a character, so students do not need TFT setup
 * code or an LCD object in this sketch.
 *
 * Prep: Install the BYUI EBadge library.
 */

#include <BYUI_EBadge.h>


void setup() {
  EBadge.clear();
  const char *name = "e-Badge";
  char grade = 'A';
  int apples = 4;
  float tempC = 22.5;
  int badgeNumber = 42;


  printf("Lesson 01: Variables\n");
  delay(500);

  printf("\n");
  printf("\n");
  printf("\n");
  delay(500);

  printf("This ");
  printf("line ");
  printf("is not split\n");
  delay(500);


  printf("Text: %s\n", name);          // %s = text
  delay(500);

  printf("Wide text: |%12s|\n", name); // %12s = text in a 12-character field
  delay(500);

  printf("Char: %c\n", grade);         // %c = one character
  delay(500);

  printf("Int: %6d\n", apples);        // %d = whole number, width 6
  delay(500);

  printf("Float: %7.2f\n", tempC);     // %.2f = 2 digits after the decimal
  delay(500);

  printf("Hex: 0x%04x\n", badgeNumber); // %x = hexadecimal, padded to 4 digits
  delay(500);

  printf("Tabs:\tA\tB\tC\n");
  delay(500);

  // This message has no newline until the end. It should wrap automatically
  // onto the next LCD row instead of running off the right edge.
  printf("Wrap test: this sentence is longer than one LCD line.\n");
  delay(500);

  // This also tests a string whose first character is a newline.
  printf("\nScroll test starts here:\n");
  for (int line = 1; line <= 35; line++) {
    printf("Line %02d is a long line that wraps and scrolls.\n", line);
    delay(500);
  }

  printf("\nPress RESET to rerun the program.\n");
}

void loop() {

}
