#include "workshop_platform.h"
#include <stdint.h>

int main(void) {
    platform_init();

    /* TODO 1: Create state variables for last update timestamp (uint32_t), brightness level (0-100), and step direction (+1 or -1). */

    while (1) {
        /* TODO 2: Every 20 ms (using wrap-safe unsigned subtraction), update platform_led_set_rgb_percent(0U, level, level)
         * and reverse direction at 0% and 100%. Do not use blocking delays. */
    }
}
