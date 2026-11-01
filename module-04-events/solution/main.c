#include "workshop_platform.h"
#include "workshop_events.h"
#include <stdio.h>

int main(void) {
    button_filter_t b;
    uint32_t count = 0U;
    char line[80];
    bool filtering_active = false;

    platform_init();
    button_filter_init(&b, platform_button_raw_pressed(), platform_millis());
    platform_console_write("timestamp_ms,button_event,event_count\r\n");

    while (1) {
        uint32_t now = platform_millis();

        /* Acknowledge asynchronous GPIO edge interrupt flag from ISR */
        if (platform_button_edge_pending()) {
            platform_button_clear_edge();
            filtering_active = true;
        }

        /* Run nonblocking debounce & hold-duration classifier when edge occurred or button is active */
        if (filtering_active || b.stable_pressed || (b.candidate_pressed != b.stable_pressed)) {
            button_event_t e = button_filter_update(&b, platform_button_raw_pressed(), now);
            if (e != BUTTON_EVENT_NONE) {
                if (e == BUTTON_EVENT_LONG_PRESS) {
                    count = 0U;
                } else {
                    count++;
                }
                snprintf(line, sizeof(line), "%lu,%s,%lu\r\n",
                         (unsigned long)now,
                         button_event_name(e),
                         (unsigned long)count);
                platform_console_write(line);
                filtering_active = false;
            }
        }
    }
}
