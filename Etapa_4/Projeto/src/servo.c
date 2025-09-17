#include "servo.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include <stdio.h>

#define PWM_WRAP 20000
#define PWM_CLKDIV 125.0f

#define SERVO_LEVEL_0_DEG   1110
#define SERVO_LEVEL_90_DEG  1200
#define SERVO_LEVEL_180_DEG 1800

static uint slice_num;

void init_servo() {
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(SERVO_PIN);

    pwm_set_clkdiv(slice_num, PWM_CLKDIV);
    pwm_set_wrap(slice_num, PWM_WRAP);
    pwm_set_enabled(slice_num, true);

    printf("Servo inicializado (sem movimento)\n");
}

// Função auxiliar para mandar um pulso e parar
static void servo_pulso(uint16_t level) {
    pwm_set_gpio_level(SERVO_PIN, level);
    sleep_ms(300);  // tempo para o servo chegar (~0.5s)
    pwm_set_gpio_level(SERVO_PIN, 0); // para de mandar sinal
}

void servo_travar() {
    servo_pulso(SERVO_LEVEL_90_DEG);
    printf("Servo travado (90 graus)\n");
}

void servo_destravar() {
    servo_pulso(SERVO_LEVEL_180_DEG);
    printf("Servo destravado (0 graus)\n");
}
