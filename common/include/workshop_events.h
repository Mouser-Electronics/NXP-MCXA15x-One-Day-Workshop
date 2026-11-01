#ifndef WORKSHOP_EVENTS_H
#define WORKSHOP_EVENTS_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_SHORT_PRESS,
    BUTTON_EVENT_LONG_PRESS
} button_event_t;

typedef struct {
    bool stable_pressed;
    bool candidate_pressed;
    uint32_t candidate_since_ms;
    uint32_t pressed_since_ms;
} button_filter_t;

void button_filter_init(button_filter_t *filter, bool raw, uint32_t now_ms);
button_event_t button_filter_update(button_filter_t *filter, bool raw, uint32_t now_ms);
const char *button_event_name(button_event_t event);

#endif /* WORKSHOP_EVENTS_H */
