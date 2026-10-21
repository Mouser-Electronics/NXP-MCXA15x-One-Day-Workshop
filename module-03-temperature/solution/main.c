#include "workshop_platform.h"
#include "workshop_temperature.h"
#include <stdio.h>
int main(void){
 uint32_t last=0U; char line[96]; float c;
 platform_init(); platform_console_write("timestamp_ms,temperature_c,temperature_f\r\n");
 while(1){uint32_t now=platform_millis(); if((uint32_t)(now-last)>=1000U){last=now; if(platform_temperature_read_c(&c)){snprintf(line,sizeof line,"%lu,%.2f,%.2f\r\n",(unsigned long)now,(double)c,(double)celsius_to_fahrenheit(c));}else{snprintf(line,sizeof line,"%lu,ERROR,ERROR\r\n",(unsigned long)now);} platform_console_write(line);}}
}
