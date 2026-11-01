#include "workshop_platform.h"
#include "workshop_config.h"
#include "workshop_events.h"
#include "workshop_temperature.h"
#include <stdio.h>

typedef enum {
    STATE_SENSOR_ERROR = 0,
    STATE_COLD,
    STATE_NORMAL,
    STATE_WARNING,
    STATE_ALARM
} monitor_state_t;

static const char *state_name(monitor_state_t s) {
    static const char *n[] = {"SENSOR_ERROR", "COLD", "NORMAL", "WARNING", "ALARM"};
    return n[s];
}

static const char *led_name(monitor_state_t s) {
    static const char *n[] = {"MAGENTA_FLASH", "BLUE", "GREEN", "YELLOW", "RED_FLASH"};
    return n[s];
}

static monitor_state_t classify(float c) {
    if (c < COLD_THRESHOLD_C)    return STATE_COLD;
    if (c < WARNING_THRESHOLD_C) return STATE_NORMAL;
    if (c < ALARM_THRESHOLD_C)   return STATE_WARNING;
    return STATE_ALARM;
}

static void apply_led(monitor_state_t state, bool flash_on) {
    if (state == STATE_SENSOR_ERROR) {
        platform_led_set(flash_on ? LED_MAGENTA : LED_OFF);
    } else if (state == STATE_COLD) {
        platform_led_set(LED_BLUE);
    } else if (state == STATE_NORMAL) {
        platform_led_set(LED_GREEN);
    } else if (state == STATE_WARNING) {
        platform_led_set(LED_YELLOW);
    } else {
        platform_led_set(flash_on ? LED_RED : LED_OFF);
    }
}

int main(void) {
    button_filter_t button;
    button_event_t last_event = BUTTON_EVENT_NONE;
    uint32_t event_count = 0U;
    float c = 0.0F;
    bool flash = false;
    char line[180];

    platform_init();
    uint32_t now = platform_millis();
    button_filter_init(&button, platform_button_raw_pressed(), now);

    /* Sample immediately at startup so LED state and t=0 CSV telemetry are accurate */
    bool valid = platform_temperature_read_c(&c);
    monitor_state_t state = valid ? classify(c) : STATE_SENSOR_ERROR;
    apply_led(state, flash);

    platform_console_write("timestamp_ms,temperature_c,temperature_f,state,button_event,event_count,led_mode\r\n");
    if (valid) {
        snprintf(line, sizeof(line), "%lu,%.2f,%.2f,%s,%s,%lu,%s\r\n",
                 (unsigned long)now, (double)c, (double)celsius_to_fahrenheit(c),
                 state_name(state), button_event_name(last_event),
                 (unsigned long)event_count, led_name(state));
    } else {
        snprintf(line, sizeof(line), "%lu,ERROR,ERROR,%s,%s,%lu,%s\r\n",
                 (unsigned long)now, state_name(state),
                 button_event_name(last_event), (unsigned long)event_count,
                 led_name(state));
    }
    platform_console_write(line);

    uint32_t last_sample = now;
    uint32_t last_report = now;
    uint32_t last_flash  = now;

    while (1) {
        now = platform_millis();

        if (platform_button_edge_pending()) {
            platform_button_clear_edge();
        }

        button_event_t e = button_filter_update(&button, platform_button_raw_pressed(), now);
        if (e != BUTTON_EVENT_NONE) {
            last_event = e;
            if (e == BUTTON_EVENT_LONG_PRESS) {
                event_count = 0U;
            } else {
                event_count++;
            }
        }

        if ((uint32_t)(now - last_sample) >= TEMP_SAMPLE_INTERVAL_MS) {
            last_sample = now;
            valid = platform_temperature_read_c(&c);
            state = valid ? classify(c) : STATE_SENSOR_ERROR;
            apply_led(state, flash);
        }

        if ((uint32_t)(now - last_flash) >= LED_FLASH_INTERVAL_MS) {
            last_flash = now;
            flash = !flash;
            apply_led(state, flash);
        }

        if ((uint32_t)(now - last_report) >= TELEMETRY_INTERVAL_MS) {
            last_report = now;
            if (valid) {
                snprintf(line, sizeof(line), "%lu,%.2f,%.2f,%s,%s,%lu,%s\r\n",
                         (unsigned long)now, (double)c, (double)celsius_to_fahrenheit(c),
                         state_name(state), button_event_name(last_event),
                         (unsigned long)event_count, led_name(state));
            } else {
                snprintf(line, sizeof(line), "%lu,ERROR,ERROR,%s,%s,%lu,%s\r\n",
                         (unsigned long)now, state_name(state),
                         button_event_name(last_event), (unsigned long)event_count,
                         led_name(state));
            }
            platform_console_write(line);
            last_event = BUTTON_EVENT_NONE;
        }
    }
}
