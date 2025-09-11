#ifndef KEYPAD_H
#define KEYPAD_H

#include "pico/stdlib.h"

// === Definições do teclado matricial ===
#define KEYPAD_ROW1 4   // GPIO08
#define KEYPAD_ROW2 8   // GPIO09
#define KEYPAD_ROW3 9   // GPIO04
#define KEYPAD_ROW4 16  // GPIO20
#define KEYPAD_COL1 17  // GPIO19
#define KEYPAD_COL2 18  // GPIO18
#define KEYPAD_COL3 19  // GPIO16
#define KEYPAD_COL4 20  // GPIO17

extern char keypad_keys[4][4];

void keypad_init();
char keypad_get_key();
char read_keypad(void);

#endif
