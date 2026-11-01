# Validation Checklist

## Module 1

- Project builds with zero errors.
- Debug session starts on the selected board.
- Short button presses cycle red, green, blue, and off.
- Holding the button does not repeatedly cycle colors.

## Module 2

- LED brightness changes smoothly.
- The main loop remains responsive during the fade.
- PWM duty-cycle limits remain between 0 and 100 percent.

## Module 3

- Sensor read returns success.
- Room-temperature readings are plausible and stable.
- CSV header (`timestamp_ms,temperature_c,temperature_f`) appears once after reset.
- CSV records contain three fields (seven-field records are validated in Module 5).
- Fahrenheit conversion equals Celsius times 9/5 plus 32 within rounding tolerance.

## Module 4

- Interrupt service routine records an edge and exits quickly.
- One physical press produces one debounced event.
- A press shorter than 1.5 seconds reports `SHORT_PRESS`.
- A press of at least 1.5 seconds reports `LONG_PRESS`.

## Module 5

- Temperature sampling continues while button events occur.
- Telemetry appears once per second.
- LED state follows the configured thresholds.
- Alarm flashing does not pause sensor sampling or serial output.
- Long press resets the event counter.
- Unsigned tick wrap does not break interval checks.
