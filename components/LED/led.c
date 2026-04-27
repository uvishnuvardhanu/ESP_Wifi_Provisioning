#include "led.h"
#include "driver/gpio.h"

#define LED_PIN 2  // onboard LED for most ESP32 dev boards

void led_init(void) {
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
}

void led_on(void) {
    gpio_set_level(LED_PIN, 1);
}

void led_off(void) {
    gpio_set_level(LED_PIN, 0);
}
