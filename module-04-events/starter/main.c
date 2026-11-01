#include "workshop_platform.h"
#include "workshop_events.h"
#include <stdio.h>

int main(void) {
    platform_init();

    /* TODO 1: Initialize button_filter_t and print CSV header "timestamp_ms,button_event,event_count\r\n". */
    /* TODO 2: In while(1), check platform_button_edge_pending() or active debounce state,
     * classify SHORT_PRESS (<1500 ms) vs LONG_PRESS (>=1500 ms), increment count on short press
     * (reset on long press), and print CSV line on each event. */
    while (1) {
    }
}
