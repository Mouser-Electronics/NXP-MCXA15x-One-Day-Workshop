# CSV Telemetry Format

Header:

```text
timestamp_ms,temperature_c,temperature_f,state,button_event,event_count,led_mode
```

Example:

```text
0,23.50,74.30,NORMAL,NONE,0,GREEN
1000,23.56,74.41,NORMAL,NONE,0,GREEN
2000,27.18,80.92,WARNING,SHORT_PRESS,1,YELLOW
3000,30.12,86.22,ALARM,NONE,1,RED_FLASH
```

Temperatures are printed with two decimal places. `timestamp_ms` is milliseconds since reset and wraps according to the unsigned 32-bit system tick. The application uses wrap-safe subtraction for interval checks.
