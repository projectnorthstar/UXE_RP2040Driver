/*
 * Copyright (c) 2026 Shenzhen Sudo Electronics Technology Limited
 * www.sudotronic.com
 * License: MIT
 */



#include <stddef.h>
#include "pico/stdlib.h"

void user_controls_init_before_power_on(void);
void user_controls_run_after_power_on(void);

#define UXE_RESET_PIN (1)
#define UXE_3V3_EN (9)
#define UXE_1V1_EN (10)
#define DISP_3V3_EN (11)
#define DISP_BIAS_EN (25)
#define USB3_1V2_EN (12)
#define DISP_1V8_EN (13)
#define UXE_INIT_PIN (14)

static void initialize_fixed_power_pins(void) {
    // EN: Power-on sequence control. Do not modify.
    // 中文：电源上电时序控制，请勿更改。

    const uint pins[] = {
        UXE_RESET_PIN, UXE_3V3_EN, UXE_1V1_EN,
        DISP_3V3_EN, DISP_BIAS_EN, USB3_1V2_EN,
        DISP_1V8_EN, UXE_INIT_PIN
    };
    for (size_t i = 0; i < sizeof(pins) / sizeof(pins[0]); ++i) {
        const uint pin = pins[i];
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, 0);
    }
}

static void run_validated_power_on_sequence(void) {

    gpio_put(UXE_1V1_EN, 1);
    sleep_ms(1);

    gpio_put(UXE_3V3_EN, 1);
    sleep_ms(21);

    gpio_put(DISP_BIAS_EN, 1);
    gpio_put(DISP_3V3_EN, 1);
    sleep_ms(1);

    gpio_put(USB3_1V2_EN, 1);
    gpio_put(DISP_1V8_EN, 1);
    sleep_ms(2);

    gpio_put(UXE_RESET_PIN, 1);
    sleep_ms(1000);
}

void uxe_initialize(void) {

    initialize_fixed_power_pins();
    user_controls_init_before_power_on();
    run_validated_power_on_sequence();
    user_controls_run_after_power_on();
}
