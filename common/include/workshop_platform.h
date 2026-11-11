#ifndef WORKSHOP_PLATFORM_H
#define WORKSHOP_PLATFORM_H

#define ENABLE_WORKSHOP_PWM 1
#define BOARD_PWM_BASEADDR PWM0 /* Swap PWM0 for your exact eFlexPWM timer base if different! */

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    LED_OFF = 0,
    LED_RED,
    LED_GREEN,
    LED_BLUE,
    LED_YELLOW,
    LED_CYAN,
    LED_MAGENTA,
    LED_WHITE
} led_color_t;

void platform_init(void);
uint32_t platform_millis(void);
bool platform_button_raw_pressed(void);
bool platform_button_edge_pending(void);
void platform_button_clear_edge(void);
void platform_led_set(led_color_t color);
void platform_led_set_rgb_percent(uint8_t red, uint8_t green, uint8_t blue);
bool platform_temperature_read_c(float *temperature_c);
void platform_console_write(const char *text);

#endif /* WORKSHOP_PLATFORM_H */
