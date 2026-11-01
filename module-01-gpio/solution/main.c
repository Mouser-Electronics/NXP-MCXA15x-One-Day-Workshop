#include "workshop_platform.h"
#include "workshop_events.h"

int main(void) {
    button_filter_t button;
    uint8_t color = 0U; /* 0: LED_OFF, 1: LED_RED, 2: LED_GREEN, 3: LED_BLUE */

    platform_init();
    button_filter_init(&button, platform_button_raw_pressed(), platform_millis());

    while (1) {
        button_event_t e = button_filter_update(&button, platform_button_raw_pressed(), platform_millis());
        if (e == BUTTON_EVENT_SHORT_PRESS) {
            color = (uint8_t)((color + 1U) % 4U);
            platform_led_set((led_color_t)color);
        }
    }
}
