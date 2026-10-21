# NXP MCXA15x One-Day Workshop

This repository supports a one-day, onboard-hardware-only workshop for the NXP FRDM-MCXA156 and FRDM-MCXA153 development boards. Participants build five progressively integrated C applications in MCUXpresso IDE.

## Workshop Path

1. GPIO and RGB LED control
2. Pulse-width modulation and a breathing LED
3. P3T1755DP temperature acquisition and CSV telemetry
4. Interrupts, timers, debouncing, and button events
5. Nonblocking environmental monitor capstone

## Hardware

- One FRDM-MCXA156 or FRDM-MCXA153 board
- The USB cable supplied with the board kit
- A Windows or Linux computer supplied by the attendee

No external sensor, breadboard, jumper wire, resistor, or LED is required.

## Software

- MCUXpresso IDE: https://www.nxp.com/mcuxpresso/ide
- MCUXpresso SDK: install the newest production SDK for the selected board from inside MCUXpresso IDE
- Optional Tera Term: https://teratermproject.github.io/
- Optional Git: https://git-scm.com/downloads

## Recommended Setup

Import the closest NXP SDK example for each board, then replace or add the workshop application files. Keep the SDK-generated `board.c`, `board.h`, `clock_config.c`, `pin_mux.c`, startup files, linker files, and device drivers in the project.

The code in this repository is divided into application logic and a thin board-support layer. Board-specific pin and peripheral choices belong in `board-support/` so the teaching code remains common.

## Repository Use

- `starter/`: exercise instructions and incomplete application file
- `solution/`: completed reference application
- `common/`: reusable interfaces and capstone logic
- `board-support/`: board adaptation notes and implementation contract
- `docs/`: schedule, setup, CSV schema, validation, and troubleshooting

## Serial Settings

- 115200 baud
- 8 data bits
- No parity
- 1 stop bit
- No flow control

## Important Board Difference

The FRDM-MCXA153 supports the documented P3T1755DP I3C exercise. The FRDM-MCXA156 onboard sensor is configured for I2C-compatible operation by default. The required workshop path therefore uses I2C-compatible transfers on both boards. I3C is an optional FRDM-MCXA153 extension.

## Build Status

The application layer is implementation-complete, but the MCUXpresso projects must be generated against the current board SDK so NXP supplies the exact startup, clock, pin-mux, linker, and debug files. Hardware validation on both physical boards is required before publication.

## License

MIT. See `LICENSE`.
