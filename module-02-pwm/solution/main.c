#include "workshop_platform.h"
#include <stdint.h>
int main(void){
 uint32_t last=0U; uint8_t level=0U; int8_t direction=1;
 platform_init();
 while(1){uint32_t now=platform_millis(); if((uint32_t)(now-last)>=20U){last=now; platform_led_set_rgb_percent(0U,level,level); if(level>=100U)direction=-1; else if(level==0U)direction=1; level=(uint8_t)((int16_t)level+direction);}}
}
