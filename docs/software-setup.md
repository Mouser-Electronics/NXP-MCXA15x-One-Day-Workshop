# Software Setup

1. Install the current production MCUXpresso IDE from NXP.
2. Start the IDE and select a writable workspace folder.
3. Open the SDK management view and install the newest production SDK for `FRDM-MCXA156` or `FRDM-MCXA153`.
4. Connect the board to the computer through the MCU-Link USB Type-C connector.
5. Import the SDK `hello_world` example and run it to verify build, flash, debug, and virtual serial operation.
6. Optionally install Tera Term and configure 115200, 8-N-1, no flow control.
7. Obtain this repository with Git or download and extract the ZIP archive.

## Git Option

```bash
git clone https://github.com/Mouser-Electronics/NXP-MCXA15x-One-Day-Workshop.git
```

The public URL becomes valid after the repository is created. Until then, use the supplied ZIP archive.

## Classroom Preflight

- IDE launches without administrator elevation.
- Correct board SDK is installed.
- Board enumerates as an MCU-Link debug probe and virtual COM port.
- `hello_world` builds with zero errors.
- Debug session starts and stops normally.
- Terminal receives text at 115200 baud.
