#include "servo.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include <stdio.h>

// Pino PWM para o servomotor
#define SERVO_PIN 2

// Configuração do PWM para 50Hz (20ms de período)
#define PWM_WRAP 20000
#define PWM_CLKDIV 125.0f

// Níveis de PWM para os ângulos
// 0.5ms (500us) -> 500 * (20000 / 20000) = 500
// 1.5ms (1500us) -> 1500 * (20000 / 20000) = 1500
// 2.5ms (2500us) -> 2500 * (20000 / 20000) = 2500
// Considerando que o duty cycle é um percentual do período,
// os valores de 0 a 180º variam de 500 a 2500.

#define SERVO_LEVEL_0_DEG 500
#define SERVO_LEVEL_90_DEG 1500
#define SERVO_LEVEL_180_DEG 2500

static uint slice_num;

void init_servo() {
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(SERVO_PIN);

    pwm_set_clkdiv(slice_num, PWM_CLKDIV);
    pwm_set_wrap(slice_num, PWM_WRAP);
    pwm_set_enabled(slice_num, true);

    // Posição inicial (travado)
    servo_travar();
}

void servo_travar() {
    // Altere para a posição de travamento desejada
    pwm_set_gpio_level(SERVO_PIN, SERVO_LEVEL_90_DEG);
    printf("Servo travado (90 graus)\n");
}

void servo_destravar() {
    // Altere para a posição de destravamento desejada
    pwm_set_gpio_level(SERVO_PIN, SERVO_LEVEL_0_DEG);
    printf("Servo destravado (0 graus)\n");
}