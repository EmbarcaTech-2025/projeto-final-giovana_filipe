#ifndef KEYPAD_H
#define KEYPAD_H

#include "pico/stdlib.h"

// Inicializa teclado matricial
void init_keypad(void);

// Lê tecla pressionada (retorna '\0' se nada)
char read_keypad(void);

// Funções relacionadas à senha
void reset_senha(void);
bool adicionar_digito(char tecla);
bool verificar_senha(void);
const char *get_senha_digitada(void);
int get_senha_pos(void);

#endif
