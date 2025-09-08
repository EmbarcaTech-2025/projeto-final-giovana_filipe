#include "keypad.h"
#include <string.h>
#include <stdio.h>
#include "servo.h"  // Incluir o novo arquivo de servo

// === Definições do teclado matricial ===
#define KEYPAD_ROW1 4  // GPIO08
#define KEYPAD_ROW2 8  // GPIO09
#define KEYPAD_ROW3 9  // GPIO04
#define KEYPAD_ROW4 16 // GPIO20
#define KEYPAD_COL1 17 // GPIO19
#define KEYPAD_COL2 18 // GPIO18
#define KEYPAD_COL3 19 // GPIO16
#define KEYPAD_COL4 20 // GPIO17

// === Matriz do teclado matricial ===
char keypad_keys[4][4] = {
    {'D', 'C', 'B', 'A'},
    {'#', '9', '6', '3'},
    {'0', '8', '5', '2'},
    {'*', '7', '4', '1'}
};

// Senha
static char senha_correta[5] = "1234";
static char senha_digitada[5] = "";
static int senha_pos = 0;

// Inicializa o teclado
void init_keypad(void) {
    gpio_init(KEYPAD_ROW1);
    gpio_init(KEYPAD_ROW2);
    gpio_init(KEYPAD_ROW3);
    gpio_init(KEYPAD_ROW4);
    gpio_set_dir(KEYPAD_ROW1, GPIO_OUT);
    gpio_set_dir(KEYPAD_ROW2, GPIO_OUT);
    gpio_set_dir(KEYPAD_ROW3, GPIO_OUT);
    gpio_set_dir(KEYPAD_ROW4, GPIO_OUT);
    gpio_put(KEYPAD_ROW1, 1);
    gpio_put(KEYPAD_ROW4, 1);

    gpio_init(KEYPAD_COL1);
    gpio_init(KEYPAD_COL2);
    gpio_init(KEYPAD_COL3);
    gpio_init(KEYPAD_COL4);
    gpio_set_dir(KEYPAD_COL1, GPIO_IN);
    gpio_set_dir(KEYPAD_COL2, GPIO_IN);
    gpio_set_dir(KEYPAD_COL3, GPIO_IN);
    gpio_set_dir(KEYPAD_COL4, GPIO_IN);
    gpio_pull_up(KEYPAD_COL1);
    gpio_pull_up(KEYPAD_COL2);
    gpio_pull_up(KEYPAD_COL3);
    gpio_pull_up(KEYPAD_COL4);

    printf("Teclado matricial inicializado\n");
}

// Leitura de tecla
char read_keypad(void) {
    uint gpio_rows[4] = {KEYPAD_ROW1, KEYPAD_ROW2, KEYPAD_ROW3, KEYPAD_ROW4};
    uint gpio_cols[4] = {KEYPAD_COL1, KEYPAD_COL2, KEYPAD_COL3, KEYPAD_COL4};

    for (int row = 0; row < 4; row++) {
        for (int i = 0; i < 4; i++) gpio_put(gpio_rows[i], 1);
        gpio_put(gpio_rows[row], 0);
        sleep_us(300);

        for (int col = 0; col < 4; col++) {
            if (gpio_get(gpio_cols[col]) == 0) {
                sleep_ms(200); // debounce
                gpio_put(gpio_rows[row], 1);
                return keypad_keys[row][col];
            }
        }
    }
    return '\0';
}

// Resetar senha
void reset_senha(void) {
    memset(senha_digitada, 0, sizeof(senha_digitada));
    senha_pos = 0;
}

// Adicionar dígito à senha
bool adicionar_digito(char tecla) {
    if (senha_pos < 4 && tecla >= '0' && tecla <= '9') {
        senha_digitada[senha_pos++] = tecla;
        return true;
    }
    return false;
}

// Verificar senha
bool verificar_senha(void) {
    printf("Verificando senha...\n");
    printf("Senha digitada: ");
    for (int i = 0; i < senha_pos; i++) {
        printf("%c", senha_digitada[i]);
    }
    printf("\n");
    printf("Senha correta: %s\n", senha_correta);
    printf("Posição: %d/4\n", senha_pos);
    
    // Garantir que a senha digitada está terminada com null
    if (senha_pos == 4) {
        senha_digitada[4] = '\0';
        
        if (strcmp(senha_digitada, senha_correta) == 0) {
            printf("Senha CORRETA - Destravando servo\n");
            reset_senha();
            servo_destravar();
            return true;
        }
    }
    
    printf("Senha INCORRETA ou incompleta\n");
    reset_senha();
    return false;
}

// Acesso à senha digitada (para mostrar no display)
const char *get_senha_digitada(void) {
    return senha_digitada;
}

int get_senha_pos(void) {
    return senha_pos;
}