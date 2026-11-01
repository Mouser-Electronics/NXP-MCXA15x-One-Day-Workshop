#include "workshop_platform.h"
#include "workshop_config.h"
#include "workshop_events.h"
#include "workshop_temperature.h"
#include <stdio.h>

int main(void) {
    platform_init();
    /* TODO: Integrate temperature sampling (1000 ms), button events (short press increments, long press resets),
     * LED state machine + 250 ms alarm/error flashing, and 7-field CSV telemetry (1000 ms) without blocking delays. */
    while (1) {
    }
}
