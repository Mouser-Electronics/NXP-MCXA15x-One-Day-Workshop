#include "workshop_platform.h"
#include "workshop_temperature.h"
#include <stdio.h>

int main(void) {
    platform_init();

    /* TODO 1: Print the 3-column CSV header ("timestamp_ms,temperature_c,temperature_f\r\n") once after reset. */

    while (1) {
        /* TODO 2: Every 1000 ms, read P3T1755DP via platform_temperature_read_c(&c).
         * If valid, format Celsius and Fahrenheit to 2 decimal places; otherwise print ERROR,ERROR. */
    }
}
