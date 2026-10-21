# Board Adapter Contract

Create a board-specific MCUXpresso project by importing the current SDK examples for the selected board. Retain its generated board, clock, pin-mux, startup, linker, and debug files. Implement `workshop_platform.h` in `workshop_platform.c` with these rules:

- `platform_millis()`: return a free-running 1 ms unsigned counter.
- `platform_button_raw_pressed()`: return `true` while SW3 is physically pressed.
- `platform_led_set()`: hide the active-low RGB LED polarity.
- `platform_led_set_rgb_percent()`: set three 0-100 duty-cycle values through hardware PWM channels selected by the Pins tool. If a board revision cannot route all RGB channels to hardware PWM, document the available channel and use the supported channels only.
- `platform_temperature_read_c()`: perform an I2C-compatible read from 7-bit address `0x48`, temperature register `0x00`; combine the two bytes as a signed 12-bit value and multiply by 0.0625 degrees Celsius.
- `platform_console_write()`: send a null-terminated string through the SDK debug console or LPUART connected to the MCU-Link virtual COM port.

Do not place blocking delays in the application layer. Use the SDK example's peripheral and pin configuration as the source of truth for the installed SDK release.

Optional extension: derive the temperature project from the SDK `i3c/master_read_sensor_p3t1755` example and compare dynamic address assignment with the shared I2C-compatible implementation.
