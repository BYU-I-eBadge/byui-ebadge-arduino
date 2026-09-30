/*
 * Example 02: Read student input from the Serial Monitor
 *
 * Open Tools > Serial Monitor and set its speed to 115200 baud.
 * Each scanf() waits until the student types a response and presses Enter.
 * The entered response is echoed to the e-Badge LCD like a console.
 */

#include <BYUI_EBadge.h>

char name[20];
int age;
float height;
char letter;
unsigned int favoriteNumber;

void setup() {
  printf("Open Serial Monitor: 115200 baud.\n");

  printf("Type your first name: ");
  if (scanf("%19s", name) == 1) {
    printf("Hello, %s!\n", name);
  }

  printf("Type your age: ");
  if (scanf("%d", &age) == 1) {
    printf("Next year you will be %d.\n", age + 1);
  }

  printf("Type your height in meters: ");
  if (scanf("%f", &height) == 1) {
    printf("Height: %.2f m\n", height);
  }

  printf("Type one letter: ");
  if (scanf("%c", &letter) == 1) {
    printf("Letter: %c\n", letter);
  }

  printf("Type a hexadecimal number, such as FF: ");
  if (scanf("%x", &favoriteNumber) == 1) {
    printf("You typed 0x%X.\n", favoriteNumber);
  }
}

void loop() {
  // scanf() is demonstrated once in setup().
}
