#include "workshop_platform.h"
#include "workshop_events.h"
#include <stdio.h>
int main(void){button_filter_t b; unsigned count=0; char line[80]; platform_init(); button_filter_init(&b,platform_button_raw_pressed(),platform_millis()); while(1){uint32_t now=platform_millis(); button_event_t e=button_filter_update(&b,platform_button_raw_pressed(),now); if(e!=BUTTON_EVENT_NONE){count++; snprintf(line,sizeof line,"%lu,%s,%u\r\n",(unsigned long)now,button_event_name(e),count); platform_console_write(line);}}}
