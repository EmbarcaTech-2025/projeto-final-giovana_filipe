#ifndef SERVO_H
#define SERVO_H

#include "pico/stdlib.h"

#define SERVO_PIN 2

void init_servo();
void servo_travar();
void servo_destravar();

#endif