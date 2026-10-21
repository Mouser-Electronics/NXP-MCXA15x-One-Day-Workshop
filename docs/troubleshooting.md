# Troubleshooting

## Board Does Not Appear

1. Confirm that the USB cable supports data.
2. Use the MCU-Link USB connector, not the target USB connector.
3. Try a direct computer port rather than an unpowered hub.
4. Close other debug sessions.
5. Reconnect the board and restart MCUXpresso IDE.

## Project Does Not Build

1. Confirm that the project targets the same board as the connected hardware.
2. Confirm that the board SDK is installed.
3. Clean the project and rebuild.
4. Verify that board, clock, pin-mux, startup, and linker files remain in the project.
5. Compare include paths and preprocessor symbols with the imported SDK example.

## LED Behavior Is Inverted

The onboard RGB LED channels are active-low. A logical high turns a channel off, and a logical low turns it on. Use the board adapter rather than writing raw values in the application.

## Temperature Read Fails

1. Confirm the 7-bit address is `0x48`.
2. Confirm the sensor is accessed over the board's supported bus configuration.
3. Verify that the correct SDK example supplied the clock and pin initialization.
4. Check the transfer status before converting the received bytes.
5. On FRDM-MCXA156, do not assume factory-default I3C operation; use the I2C-compatible workshop path.

## Serial Output Is Missing

1. Select the MCU-Link virtual COM port.
2. Use 115200 baud, 8-N-1, no flow control.
3. Close the terminal before starting a debug tool that requires exclusive access.
4. Reset the board after opening the terminal.

## Button Generates Several Events

Increase `BUTTON_DEBOUNCE_MS` from 30 ms to 40 or 50 ms. Do not add a blocking delay inside the interrupt service routine.
