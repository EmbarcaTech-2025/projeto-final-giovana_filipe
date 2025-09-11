#include "keypad.h"

char keypad_keys[4][4] = {
    {'D', 'C', 'B', 'A'},
    {'#', '9', '6', '3'},
    {'0', '8', '5', '2'},
    {'*', '7', '4', '1'}
};

uint keypad_rows[4] = {KEYPAD_ROW1, KEYPAD_ROW2, KEYPAD_ROW3, KEYPAD_ROW4};
uint keypad_cols[4] = {KEYPAD_COL1, KEYPAD_COL2, KEYPAD_COL3, KEYPAD_COL4};

void keypad_init() {
    for (int r = 0; r < 4; r++) {
        gpio_init(keypad_rows[r]);
        gpio_set_dir(keypad_rows[r], GPIO_OUT);
        gpio_put(keypad_rows[r], 1);
    }
    for (int c = 0; c < 4; c++) {
        gpio_init(keypad_cols[c]);
        gpio_set_dir(keypad_cols[c], GPIO_IN);
        gpio_pull_up(keypad_cols[c]);
    }
}

char read_keypad() {
    for (int r = 0; r < 4; r++) {
        gpio_put(keypad_rows[r], 0);

        for (int c = 0; c < 4; c++) {
            if (gpio_get(keypad_cols[c]) == 0) {
                sleep_ms(200); // debounce
                gpio_put(keypad_rows[r], 1);
                return keypad_keys[r][c];
            }
        }

        gpio_put(keypad_rows[r], 1);
    }
    return 0; // Nenhuma tecla pressionada
}
