#include "workshop_events.h"
#include "workshop_config.h"

void button_filter_init(button_filter_t *f, bool raw, uint32_t now) {
    f->stable_pressed     = raw;
    f->candidate_pressed  = raw;
    f->candidate_since_ms = now;
    f->pressed_since_ms   = now;
}

button_event_t button_filter_update(button_filter_t *f, bool raw, uint32_t now) {
    /* Reset debounce timer whenever the raw pin state changes */
    if (raw != f->candidate_pressed) {
        f->candidate_pressed  = raw;
        f->candidate_since_ms = now;
    }

    /* Accept candidate state only after it remains stable for BUTTON_DEBOUNCE_MS (30 ms) */
    if ((f->candidate_pressed != f->stable_pressed) &&
        ((uint32_t)(now - f->candidate_since_ms) >= BUTTON_DEBOUNCE_MS)) {
        f->stable_pressed = f->candidate_pressed;

        if (f->stable_pressed) {
            /* Button was just stably pressed: record press start timestamp */
            f->pressed_since_ms = now;
        } else {
            /* Button was just stably released: classify hold duration */
            uint32_t held_ms = (uint32_t)(now - f->pressed_since_ms);
            return (held_ms >= LONG_PRESS_TIME_MS)
                       ? BUTTON_EVENT_LONG_PRESS
                       : BUTTON_EVENT_SHORT_PRESS;
        }
    }

    return BUTTON_EVENT_NONE;
}

const char *button_event_name(button_event_t e) {
    if (e == BUTTON_EVENT_SHORT_PRESS) {
        return "SHORT_PRESS";
    }
    if (e == BUTTON_EVENT_LONG_PRESS) {
        return "LONG_PRESS";
    }
    return "NONE";
}
