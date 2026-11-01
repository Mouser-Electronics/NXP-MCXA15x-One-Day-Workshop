#include "workshop_platform.h"
#include "workshop_events.h"

int main(void) {
    platform_init();

    /* TODO 1: Declare a button_filter_t variable and a color index (0 = OFF, 1 = RED, 2 = GREEN, 3 = BLUE). */
    /* TODO 2: Initialize the button filter with button_filter_init(). */

    while (1) {
        /* TODO 3: Poll button_filter_update() and advance the RGB color on each BUTTON_EVENT_SHORT_PRESS. */
    }
}
