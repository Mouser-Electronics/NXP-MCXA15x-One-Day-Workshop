# Build a Nonblocking Environmental Monitor with NXP MCX A15x MCUs

**Complete Workshop Project Article & Step-by-Step Student Lab Walkthrough**  
*Target Environment: MCUXpresso IDE v25.6 [Build 136] | FRDM-MCXA156 SDK 26.09.00 (and FRDM-MCXA153)*

---

## Abstract

This project-based workshop introduces embedded C development with the **NXP FRDM-MCXA156** and **FRDM-MCXA153** development boards. Participants use **MCUXpresso IDE v25.6** and **MCUXpresso SDK 26.09.00** drivers to control the onboard RGB light-emitting diode (LED), generate pulse-width modulation (PWM), read the onboard **P3T1755DP** temperature sensor over its factory-configured $\text{I}^2\text{C}$-compatible bus (`LPI2C0`), process interrupt-driven button events, and produce comma-separated values (CSV) telemetry. The capstone combines those features into a cooperative, nonblocking environmental monitor that continues sampling, reporting, and responding to user input without pausing the microcontroller.

---

## 1. Introduction

A useful microcontroller workshop must progress beyond isolated peripheral demonstrations. Each exercise in this workshop contributes a reusable component to the final application:

1. **Module 1** establishes a repeatable edit-build-flash-debug workflow while introducing GPIO inputs, active-low RGB LED outputs, and hardware abstraction.
2. **Module 2** adds hardware PWM duty-cycle control and nonblocking timed brightness transitions.
3. **Module 3** introduces register-based sensor transactions over $\text{I}^2\text{C}$ (`LPI2C0`), fixed-point two's-complement conversion, and structured CSV serial telemetry.
4. **Module 4** separates fast GPIO edge interrupt capture from main-loop debounce filtering and hold-duration classification.
5. **Module 5** integrates all previous modules into a multi-rate, nonblocking environmental monitor that coordinates sensor sampling, serial reporting, alarm LED flashing, and asynchronous button input without a real-time operating system (RTOS) or blocking delay loops.

### Hardware & Board Compatibility Strategy

The required workshop path uses **only hardware installed on the development board**—no external sensor, breadboard, jumper wire, or soldering is required:

* **FRDM-MCXA156 (Primary Reference Board):** Features an Arm Cortex-M33 core running at up to $96\text{ MHz}$, $1\text{ MB}$ Flash, $128\text{ KB}$ SRAM, onboard **MCU-Link** CMSIS-DAP debug probe (`J21`), active-low RGB LED (`D12`: Red on `P3_12`, Green on `P3_13`, Blue on `P3_0`), user push buttons (`SW3` on `P0_6` and `SW2` on `P1_7`), and the onboard **P3T1755DP** digital temperature sensor (`U5` at $7\text{-bit}$ address `0x48` on `P0_16`/`P0_17`).
* **Why Standard $\text{I}^2\text{C}$ (`LPI2C0`) Is Required on FRDM-MCXA156:** Although the P3T1755DP sensor chip supports both $\text{I}^2\text{C}$ and $\text{I3C}$ protocols, the factory resistor network populated on the FRDM-MCXA156 configures the sensor bus for **$\text{I}^2\text{C}$-compatible operation (`LPI2C0`) by default**. Using native $\text{I3C}$ on the FRDM-MCXA156 would require physically removing surface-mount resistors, which is not feasible in a classroom setting. Therefore, the required workshop path uses standard $\text{I}^2\text{C}$-compatible transfers (`LPI2C0`) on unmodified boards, while native $\text{I3C}$ is provided as an optional extension for the FRDM-MCXA153.

---

## 2. Learning Objectives

By completing this workshop, participants will be able to:
* Create, import, configure, build, flash, and debug an MCUXpresso SDK 26.09.00 project in MCUXpresso IDE v25.6.
* Control active-low RGB LED outputs and read active-low push-button GPIO inputs through a clean hardware abstraction layer (`workshop_platform.h`).
* Configure hardware PWM duty cycle ($0\%$ to $100\%$) and implement a nonblocking $20\text{ ms}$ breathing effect.
* Read a register-based digital temperature sensor (`P3T1755DP`, $7\text{-bit}$ address `0x48`, register `0x00`) over `LPI2C0` and convert its signed $12\text{-bit}$ fixed-point value ($0.0625^\circ\text{C/LSB}$).
* Explain the practical electrical and protocol differences between $\text{I}^2\text{C}$ and $\text{I3C}$ on the FRDM-MCXA156 and FRDM-MCXA153 boards.
* Capture asynchronous button edges with a minimal GPIO Interrupt Service Routine (ISR) and debounce the switch in the main loop without blocking delays.
* Classify short-press ($< 1500\text{ ms}$) and long-press ($\ge 1500\text{ ms}$) user events.
* Schedule multiple periodic tasks ($1000\text{ ms}$ sampling, $1000\text{ ms}$ telemetry, $250\text{ ms}$ alarm flashing) using wrap-safe unsigned timestamp comparisons.
* Generate and validate structured CSV telemetry suitable for serial logging and spreadsheet plotting.

---

## 3. Required Hardware, Software, and Schedule

### Hardware & Software Checklist

| Item | Requirement |
| :--- | :--- |
| **Development Board** | One **NXP FRDM-MCXA156** (primary) or **FRDM-MCXA153** |
| **USB Cable** | Data-capable USB Type-C cable connected to the **MCU-Link** port (`J21` on FRDM-MCXA156) |
| **Computer** | Windows 10/11 or Linux PC supplied by the attendee |
| **IDE** | **MCUXpresso IDE v25.6 [Build 136]** |
| **SDK** | **FRDM-MCXA156 SDK 26.09.00** (or FRDM-MCXA153 SDK) installed in MCUXpresso IDE |
| **Serial Terminal** | Built-in MCUXpresso **Terminal** view or **Tera Term** ($115200\text{ baud}$, $8\text{-N-}1$, no flow control) |
| **Workshop Files** | Downloaded ZIP archive or Git clone of `NXP-MCXA15x-One-Day-Workshop` |

### Recommended One-Day Workshop Schedule

| Time | Activity | Duration |
| :--- | :--- | :--- |
| 8:30 – 8:50 | Welcome, board check, and repository access | $20\text{ min}$ |
| 8:50 – 10:00 | Module 1: GPIO and first debug session | $70\text{ min}$ |
| 10:00 – 10:15 | Break | $15\text{ min}$ |
| 10:15 – 11:15 | Module 2: PWM and RGB LED | $60\text{ min}$ |
| 11:15 – 12:30 | Module 3: Temperature and CSV telemetry | $75\text{ min}$ |
| 12:30 – 1:15 | Lunch | $45\text{ min}$ |
| 1:15 – 2:25 | Module 4: Interrupts and button events | $70\text{ min}$ |
| 2:25 – 2:40 | Break | $15\text{ min}$ |
| 2:40 – 4:20 | Module 5: Capstone environmental monitor | $100\text{ min}$ |
| 4:20 – 5:00 | Validation, troubleshooting, and review | $40\text{ min}$ |

---

## 4. Architecture: Portable Application vs. Board Adapter

To keep the student exercises focused on embedded software architecture rather than repetitive register boilerplate, the codebase is split into two layers:

1. **Portable Application Layer (`common/` and `module-01` through `module-05`):**
   * `common/include/workshop_platform.h`: Declares the board-independent functions for millisecond timing, button reading, RGB LED control, PWM brightness, temperature sensing, and serial output.
   * `common/include/workshop_config.h`: Defines timing constants (`TEMP_SAMPLE_INTERVAL_MS = 1000U`, `BUTTON_DEBOUNCE_MS = 30U`, `LONG_PRESS_TIME_MS = 1500U`, `LED_FLASH_INTERVAL_MS = 250U`) and temperature thresholds (`COLD_THRESHOLD_C = 18.0F`, `WARNING_THRESHOLD_C = 27.0F`, `ALARM_THRESHOLD_C = 30.0F`).
   * `common/include/workshop_events.h` & `common/source/workshop_events.c`: Implements the nonblocking $30\text{ ms}$ debounce state machine (`button_filter_t`) and hold-duration classifier.
   * `common/include/workshop_temperature.h` & `common/source/workshop_temperature.c`: Implements `celsius_to_fahrenheit()`.
2. **Board Support Layer (`common/source/workshop_platform.c`):**
   * Maps the `workshop_platform.h` functions to the NXP MCUXpresso SDK 26.09.00 drivers (`fsl_gpio.h`, `fsl_port.h`, `fsl_clock.h`, `fsl_lpi2c.h`, `fsl_pwm.h`/`fsl_ctimer.h`, and `fsl_debug_console.h`) generated for the FRDM-MCXA156 or FRDM-MCXA153.

### Why the Cooperative Scheduler Does Not Block

Traditional introductory microcontroller code uses blocking delay loops (`SDK_DelayAtLeastUs()` or `for` loops) that freeze the CPU. During a $1000\text{ ms}$ blocking delay, the processor cannot detect button presses or flash a $250\text{ ms}$ alarm LED.

In this workshop, `SysTick_Handler()` increments a `volatile uint32_t` millisecond counter (`g_systick_ms`) every $1\text{ ms}$. Each periodic task in `main()` compares the current time `now` against its previous execution timestamp `last` using unsigned 32-bit subtraction:

```c
uint32_t now = platform_millis();
if ((uint32_t)(now - last_sample) >= TEMP_SAMPLE_INTERVAL_MS) {
    last_sample = now;
    /* Run periodic task immediately and return to main loop */
}
```

Because unsigned arithmetic is modulo $2^{32}$, subtracting `last_sample` from `now` yields the exact elapsed milliseconds even if the 32-bit counter wraps around from `0xFFFFFFFF` ($4,294,967,295\text{ ms} \approx 49.7\text{ days}$) to `0x00000000`. When the elapsed difference **reaches or exceeds** the desired interval (`>=`), the task executes in microseconds and records `now` as its new timestamp.

---

## 5. Step-by-Step Walkthrough: IDE Setup, GitHub Files & Build Configuration

Follow these exact steps to prepare your workspace in **MCUXpresso IDE v25.6 [Build 136]** with **FRDM-MCXA156 SDK 26.09.00**.

### Step 5.1: Launch Workspace and Verify SDK 26.09.00 Installation
1. Start **MCUXpresso IDE v25.6**.
2. In the **MCUXpresso IDE Launcher** dialog, choose a clean workspace directory path without special characters (for example, `C:\mcux_workshop_ws`) and click **Launch**.
3. Look at the **Installed SDKs** view in the bottom panel of the workbench:
   * Verify that **`SDK_2.x_FRDM-MCXA156`** (Version `26.09.00` / Manifest version `3.x`) is listed.
   * If it is not listed, drag your downloaded `SDK_2_x_FRDM-MCXA156.zip` archive directly from Windows Explorer / Linux File Manager into the **Installed SDKs** view and click **OK** when prompted.

### Step 5.2: Download or Clone the Workshop GitHub Repository
1. If you have Git installed, open a command prompt or terminal and run:
   ```bash
   git clone https://github.com/Mouser-Electronics/NXP-MCXA15x-One-Day-Workshop.git
   ```
   *(Alternatively, extract the `NXP-MCXA15x-One-Day-Workshop.zip` archive provided by your instructor to your Desktop or Documents folder.)*
2. Verify that you can see the `common/`, `board-support/`, and `module-01-gpio/` through `module-05-capstone/` folders.

### Step 5.3: Import a Known-Good Base SDK Project
Rather than writing startup vectors and linker scripts from scratch, you will import a board SDK example so NXP's generated `board.c`, `clock_config.c`, `pin_mux.c`, startup code, and CMSIS-DAP debug launch configurations are created automatically:

1. Connect the **FRDM-MCXA156** board to your computer using the USB Type-C port labeled **`MCU-LINK`** (`J21`, located on the top-left edge of the board).
2. In the **Quickstart Panel** (bottom-left pane of MCUXpresso IDE), click **Import SDK example(s)...**.
3. In the **Board and/or Device selection page**, type `mcxa156` in the filter box, click the **frdmmcxa156** board image (it will highlight in blue), and click **Next >**.
4. In the **Import Wizard** page, configure the following settings:
   * **Project name suffix:** Type `_workshop` (so your project is named `frdmmcxa156_gpio_led_output_workshop` or `frdmmcxa156_hello_world_workshop`).
   * **Project Options -> SDK Debug Console:** Select the **`UART`** radio button. *(Critical: As explained in UG10055 Section 5, this sets `SDK_DEBUGCONSOLE_UART` and links the `nohost` library so `PRINTF` and `platform_console_write()` transmit over the MCU-Link USB Virtual COM port instead of halting when no debugger is attached.)*
   * **Project Options -> Copy sources:** Ensure **Copy sources** is **checked** so edits stay local to your project.
   * **Examples list:** Expand **`driver_examples -> gpio`** and check **`gpio_led_output`** (or expand **`demo_apps`** and check **`hello_world`**).
5. Click **Finish**. Your new project now appears in the **Project Explorer** pane on the left.

### Step 5.4: Add the Required SDK Drivers (`lpi2c`, `gpio`, `port`, `pwm`/`ctimer`)
So that your single project has all NXP SDK drivers needed across Modules 1 through 5 (`fsl_gpio.c`, `fsl_lpi2c.c`, `fsl_pwm.c` or `fsl_ctimer.c`, and `fsl_debug_console.c`):

1. Click on your project name in **Project Explorer** to select it.
2. In the top IDE toolbar, click the **Manage SDK Components** icon (blue/yellow cube icon), or right-click the project and select **SDK Management -> Manage SDK Components**.
3. Expand the **Driver** category and ensure the following checkboxes are **checked**:
   * `clock`
   * `common`
   * `gpio`
   * `lpi2c` *(Required for Module 3 & Module 5 P3T1755DP $\text{I}^2\text{C}$ sensor reads)*
   * `lpuart` *(Required for serial console output)*
   * `port` *(Required for pin multiplexing and pull-up configuration)*
   * `pwm` (eFlexPWM) and/or `ctimer` *(Required for Module 2 hardware PWM)*
4. Click **OK** and then click **Yes** to update the project files.

### Step 5.5: Add the GitHub Workshop Files to Your Project
1. First, test the unmodified SDK example: in the **Quickstart Panel**, click **Build** (hammer icon) and confirm `0 errors` in the **Console** tab.
2. Open your computer's file manager to your extracted `NXP-MCXA15x-One-Day-Workshop` folder.
3. Select the **`common`** folder and the **`board-support`** folder, right-click, and click **Copy**.
4. Return to MCUXpresso IDE, right-click the top-level project name in **Project Explorer**, and click **Paste**.
   *(If you drag and drop the folders instead, select **Copy files and folders** in the File Operation dialog and click **OK**.)*
5. Confirm that your **Project Explorer** tree now shows:
   * `common/source/workshop_platform.c`
   * `common/include/workshop_config.h`
   * `common/include/workshop_events.h`
   * `common/include/workshop_platform.h`
   * `common/include/workshop_temperature.h`
   * `common/source/workshop_events.c`
   * `common/source/workshop_temperature.c`
6. Expand the **`source/`** folder inside your project in **Project Explorer**. Right-click the default SDK example file (for example, `gpio_led_output.c` or `hello_world.c`), select **Delete**, and click **OK** so it does not cause a `multiple definition of 'main'` linker error.

### Step 5.6: Configure the `Debug` Build Configuration
As described in *MCUXpresso IDE User Guide (UG10055, Section 5.2.1)*, MCUXpresso projects have **`Debug`** and **`Release`** build configurations. We will use the **`Debug`** configuration (`-O0` optimization) so local variables and breakpoints are never optimized out during debugging.

1. Right-click your project in **Project Explorer** and select **Properties** (or click **Edit project settings** in the **Quickstart Panel**).
2. In the left navigation tree, expand **C/C++ Build** and click **Settings**.
3. At the top of the dialog, verify that **Configuration:** is set to **`Debug [Active]`**.
4. **Add the `common/include` Header Search Path:**
   * Under the **Tool Settings** tab, expand **MCU C Compiler** and click **Includes**.
   * In the upper **Include paths (-I)** panel, click the **Add...** icon (page with a green `+` sign).
   * Click the **Workspace...** button, expand your project, select **`common/include`**, and click **OK**, then **OK** again.
   * Confirm that `"${workspace_loc:/${ProjName}/common/include}"` now appears in the Include paths list.
5. **Verify Optimization and Debug Level:**
   * Under **MCU C Compiler -> Optimization**, verify **Optimization Level** is **`None (-O0)`**.
   * Under **MCU C Compiler -> Debugging**, verify **Debug Level** is **`Default (-g)`** or **`Maximum (-g3)`**.
6. **Verify Floating-Point `printf` / `snprintf` Support (`%.2f`):**
   * By default, embedded C libraries (`Redlib` and `Newlib-nano`) omit floating-point formatters to save Flash memory, which causes `snprintf(line, sizeof(line), "%.2f", c)` in Modules 3 and 5 to print blank fields unless floating-point `printf` is enabled.
   * `common/source/workshop_platform.c` automatically includes `__asm(".global _printf_float");` (per *UG10055 Section 19.5*) to enable float formatting in `Newlib-nano`.
   * If your project uses **Redlib**, ensure `CR_INTEGER_PRINTF` is **not** defined under **MCU C Compiler -> Preprocessor -> Defined symbols (-D)**; if it is present, select `CR_INTEGER_PRINTF` and click the **Delete** (`X`) icon.
7. Click **Apply and Close**.
8. **Verify Source Exclusion Flag:** Look at the `common` and `board-support` folders in **Project Explorer**. If either folder icon has a grey slash through it (indicating Eclipse marked a newly pasted folder as *Excluded from build*), right-click that folder, select **Resource Configurations -> Exclude from build...**, uncheck **Debug** and **Release**, and click **OK**.

---

## 6. Reference Implementation: `common/source/workshop_platform.c`

Confirm that `common/source/workshop_platform.c` in your project matches the complete **FRDM-MCXA156 SDK 26.09.00** implementation below. Notice how it:
* Configures **SysTick** for a $1\text{ ms}$ interrupt timebase (`platform_millis()`).
* Initializes the active-low RGB LED pins (`P3_12` Red, `P3_13` Green, `P3_0` Blue) to **OFF** (`1U`) at startup and hides the active-low polarity inside `platform_led_set()`.
* Configures user push button **SW3** (`P0_6`) with a both-edge GPIO interrupt (`GPIO0_IRQHandler`) that sets `g_button_edge_pending = true` and exits immediately.
* Configures and reads the onboard **P3T1755DP** temperature sensor at $7\text{-bit}$ address `0x48` over **`LPI2C0`** (`P0_16` SDA, `P0_17` SCL) without any board modifications.

```c
#include "workshop_platform.h"
#include "board.h"
#include "clock_config.h"
#include "pin_mux.h"
#include "fsl_pwm.h"
#include "fsl_common.h"
#include "fsl_clock.h"
#include "fsl_reset.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
#include "fsl_lpi2c.h"
#include "fsl_debug_console.h"
#include <string.h>

/* Force linker inclusion of floating-point printf/snprintf support (UG10055 Sec 19.5) */
#if defined(__GNUC__)
__asm(".global _printf_float");
#endif

/* P3T1755DP Sensor Constants (Board Adapter Contract) */
#define P3T1755_I2C_ADDR_7BIT       (0x48U)
#define P3T1755_REG_TEMP            (0x00U)
#define P3T1755_LSB_CELSIUS         (0.0625F)
#define WORKSHOP_LPI2C_BAUDRATE_HZ  (100000U)

#ifndef BOARD_SENSOR_LPI2C_BASEADDR
#define BOARD_SENSOR_LPI2C_BASEADDR LPI2C0
#endif

/* Default FRDM-MCXA156 SW3 (P0_6) and RGB LED (D12: Red=P3_12, Green=P3_13, Blue=P3_0) */
#ifndef BOARD_SW_GPIO
  #if defined(BOARD_SW3_GPIO)
    #define BOARD_SW_GPIO        BOARD_SW3_GPIO
    #define BOARD_SW_GPIO_PIN    BOARD_SW3_GPIO_PIN
    #define BOARD_SW_IRQ         BOARD_SW3_IRQ
    #define BOARD_SW_IRQ_HANDLER BOARD_SW3_IRQ_HANDLER
  #else
    #define BOARD_SW_GPIO        GPIO0
    #define BOARD_SW_GPIO_PIN    (6U)
    #define BOARD_SW_IRQ         GPIO0_IRQn
    #define BOARD_SW_IRQ_HANDLER GPIO0_IRQHandler
  #endif
#endif

#ifndef BOARD_LED_RED_GPIO
  #define BOARD_LED_RED_GPIO        GPIO3
  #define BOARD_LED_RED_GPIO_PIN    (12U)
  #define BOARD_LED_GREEN_GPIO      GPIO3
  #define BOARD_LED_GREEN_GPIO_PIN  (13U)
  #define BOARD_LED_BLUE_GPIO       GPIO3
  #define BOARD_LED_BLUE_GPIO_PIN   (0U)
#endif

static volatile uint32_t g_systick_ms = 0U;
static volatile bool g_button_edge_pending = false;

void SysTick_Handler(void) {
    g_systick_ms++;
}

void BOARD_SW_IRQ_HANDLER(void) {
    uint32_t flags = GPIO_GpioGetInterruptFlags(BOARD_SW_GPIO);
    GPIO_GpioClearInterruptFlags(BOARD_SW_GPIO, flags);
    g_button_edge_pending = true;
    SDK_ISR_EXIT_BARRIER;
}

static void write_led_pins(bool red_on, bool green_on, bool blue_on) {
    /* Active-low RGB LED: 0U = ON, 1U = OFF */
    GPIO_PinWrite(BOARD_LED_RED_GPIO, BOARD_LED_RED_GPIO_PIN, red_on ? 0U : 1U);
    GPIO_PinWrite(BOARD_LED_GREEN_GPIO, BOARD_LED_GREEN_GPIO_PIN, green_on ? 0U : 1U);
    GPIO_PinWrite(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PIN, blue_on ? 0U : 1U);
}

static void init_sensor_lpi2c0_pins_and_bus(void) {
    /* Enable clocks and release resets for PORT0, PORT3, GPIO0, GPIO3, and LPI2C0 */
    CLOCK_EnableClock(kCLOCK_GatePORT0);
    CLOCK_EnableClock(kCLOCK_GatePORT3);
    CLOCK_EnableClock(kCLOCK_GateGPIO0);
    CLOCK_EnableClock(kCLOCK_GateGPIO3);

    RESET_ReleasePeripheralReset(kPORT0_RST_SHIFT_RSTn);
    RESET_ReleasePeripheralReset(kPORT3_RST_SHIFT_RSTn);
    RESET_ReleasePeripheralReset(kGPIO0_RST_SHIFT_RSTn);
    RESET_ReleasePeripheralReset(kGPIO3_RST_SHIFT_RSTn);

    /* Ensure RGB LED pins (P3_12, P3_13, P3_0) and SW3 (P0_6) are muxed to GPIO (Alt 0) */
#if defined(ENABLE_WORKSHOP_PWM)
    /* Route to physical hardware PWM across FlexPWM0 and FlexPWM1 */
    PORT_SetPinMux(PORT3, 12U, kPORT_MuxAlt7); /* Red:   PWM1_SM2_PWMA */
    PORT_SetPinMux(PORT3, 13U, kPORT_MuxAlt7); /* Green: PWM1_SM2_PWMB */
    PORT_SetPinMux(PORT3, 0U,  kPORT_MuxAlt5); /* Blue:  PWM0_SM0_PWMA */
#else
    /* Standard GPIO fallback */
    PORT_SetPinMux(PORT3, 12U, kPORT_MuxAlt0);
    PORT_SetPinMux(PORT3, 13U, kPORT_MuxAlt0);
    PORT_SetPinMux(PORT3, 0U,  kPORT_MuxAlt0);
#endif

    PORT_SetPinMux(PORT0, 6U,  kPORT_MuxAlt0); // Keep SW3 button as GPIO

    /* Route P0_16 (LPI2C0_SDA) and P0_17 (LPI2C0_SCL) to Alt 2 with input buffer & pull-up enabled */
    const port_pin_config_t lpi2c_pin_cfg = {
        .pullSelect          = kPORT_PullUp,
        .pullValueSelect     = kPORT_LowPullResistor,
        .slewRate            = kPORT_FastSlewRate,
        .passiveFilterEnable = kPORT_PassiveFilterDisable,
        .openDrainEnable     = kPORT_OpenDrainEnable,
        .driveStrength       = kPORT_LowDriveStrength,
        .driveStrength1      = kPORT_NormalDriveStrength,
        .mux                 = kPORT_MuxAlt2,
        .inputBuffer         = kPORT_InputBufferEnable,
        .invertInput         = kPORT_InputNormal,
        .lockRegister        = kPORT_UnlockRegister
    };
    PORT_SetPinConfig(PORT0, 16U, &lpi2c_pin_cfg);
    PORT_SetPinConfig(PORT0, 17U, &lpi2c_pin_cfg);

    /* Attach FRO_HF_DIV clock to LPI2C0 and initialize master at 100 kHz */
    CLOCK_SetClockDiv(kCLOCK_DivLPI2C0, 1U);
    CLOCK_AttachClk(kFRO_HF_DIV_to_LPI2C0);
    RESET_ReleasePeripheralReset(kLPI2C0_RST_SHIFT_RSTn);

    lpi2c_master_config_t masterCfg;
    LPI2C_MasterGetDefaultConfig(&masterCfg);
    masterCfg.baudRate_Hz = WORKSHOP_LPI2C_BAUDRATE_HZ;
    LPI2C_MasterInit(BOARD_SENSOR_LPI2C_BASEADDR, &masterCfg, CLOCK_GetLpi2cClkFreq(0U));
}

#if defined(ENABLE_WORKSHOP_PWM)
static void init_rgb_pwm(void) {
    pwm_config_t pwmConfig;
    pwm_signal_param_t pwmSignal_Blue[1];
    pwm_signal_param_t pwmSignal_RedGreen[2];

    uint32_t pwmSourceClockInHz = SystemCoreClock;

    /* 1. Wake up BOTH FlexPWM hardware blocks! */
    CLOCK_EnableClock(kCLOCK_GateFLEXPWM0);
    CLOCK_EnableClock(kCLOCK_GateFLEXPWM1);

    /* 2. Power up Submodule 0 (Blue) and Submodule 2 (Red/Green) */
    CLOCK_EnableClock(kCLOCK_GatePWM0SM0);
    CLOCK_EnableClock(kCLOCK_GatePWM1SM2);

    /* 3. Unclamp the hardware resets */
    RESET_ReleasePeripheralReset(kFLEXPWM0_RST_SHIFT_RSTn);
    RESET_ReleasePeripheralReset(kFLEXPWM1_RST_SHIFT_RSTn);

    /* 4. Get the default config and apply the prescaler/debug overrides */
    PWM_GetDefaultConfig(&pwmConfig);
    pwmConfig.reloadLogic = kPWM_ReloadPwmFullCycle;
    pwmConfig.prescale = kPWM_Prescale_Divide_8;
    pwmConfig.enableDebugMode = true;

    /* Decouple Channel A and B so Red and Green don't fight! */
    pwmConfig.pairOperation = kPWM_Independent;

    /* Initialize SM0 on FLEXPWM0 and SM2 on FLEXPWM1 */
    PWM_Init(FLEXPWM0, kPWM_Module_0, &pwmConfig);
    PWM_Init(FLEXPWM1, kPWM_Module_2, &pwmConfig);

    /* ---------------------------------------------------------------------- */
    /* THE VIBE CHECK (Hardware Overrides)                                    */
    /* ---------------------------------------------------------------------- */

    /* Clear the Disable Mapping registers for BOTH controllers */
    FLEXPWM0->SM[0].DISMAP[0] = 0x0000;
    FLEXPWM0->SM[0].DISMAP[1] = 0x0000;

    FLEXPWM1->SM[2].DISMAP[0] = 0x0000;
    FLEXPWM1->SM[2].DISMAP[1] = 0x0000;

    /* Manually force the Master Output Enable switches ON */
    FLEXPWM0->OUTEN |= PWM_OUTEN_PWMA_EN(1U << 0);   /* SM0 PWMA (Blue)  */

    FLEXPWM1->OUTEN |= PWM_OUTEN_PWMA_EN(1U << 2) |  /* SM2 PWMA (Red)   */
                       PWM_OUTEN_PWMB_EN(1U << 2);   /* SM2 PWMB (Green) */

    /* ---------------------------------------------------------------------- */

    /* 5. Configure the Channels (Default 100% duty cycle = OFF for active-low LEDs) */

    /* Setup Blue (FLEXPWM0, SM0, Channel A) */
    memset(pwmSignal_Blue, 0, sizeof(pwmSignal_Blue));
    pwmSignal_Blue[0].pwmChannel = kPWM_PwmA;
    pwmSignal_Blue[0].level = kPWM_HighTrue;
    pwmSignal_Blue[0].dutyCyclePercent = 100U;
    PWM_SetupPwm(FLEXPWM0, kPWM_Module_0, pwmSignal_Blue, 1, kPWM_SignedCenterAligned, 1000U, pwmSourceClockInHz);

    /* Setup Red and Green (FLEXPWM1, SM2, Channels A and B) */
    memset(pwmSignal_RedGreen, 0, sizeof(pwmSignal_RedGreen));
    pwmSignal_RedGreen[0].pwmChannel = kPWM_PwmA;
    pwmSignal_RedGreen[0].level = kPWM_HighTrue;
    pwmSignal_RedGreen[0].dutyCyclePercent = 100U;

    pwmSignal_RedGreen[1].pwmChannel = kPWM_PwmB;
    pwmSignal_RedGreen[1].level = kPWM_HighTrue;
    pwmSignal_RedGreen[1].dutyCyclePercent = 100U;
    PWM_SetupPwm(FLEXPWM1, kPWM_Module_2, pwmSignal_RedGreen, 2, kPWM_SignedCenterAligned, 1000U, pwmSourceClockInHz);

    /* 6. Load the config and hit play! */
    PWM_SetPwmLdok(FLEXPWM0, kPWM_Control_Module_0, true);
    PWM_SetPwmLdok(FLEXPWM1, kPWM_Control_Module_2, true);

    PWM_StartTimer(FLEXPWM0, kPWM_Control_Module_0);
    PWM_StartTimer(FLEXPWM1, kPWM_Control_Module_2);
}
#endif

void platform_init(void) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    init_sensor_lpi2c0_pins_and_bus();

#if defined(ENABLE_WORKSHOP_PWM)
    /* Start the hardware timers so the APB bus is ready for duty cycle updates */
    init_rgb_pwm();
#else
    /* Fallback: Initialize active-low RGB LED outputs to OFF (1U) */
    gpio_pin_config_t led_cfg = { kGPIO_DigitalOutput, 1U };
    GPIO_PinInit(BOARD_LED_RED_GPIO, BOARD_LED_RED_GPIO_PIN, &led_cfg);
    GPIO_PinInit(BOARD_LED_GREEN_GPIO, BOARD_LED_GREEN_GPIO_PIN, &led_cfg);
    GPIO_PinInit(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PIN, &led_cfg);
    platform_led_set(LED_OFF);
#endif

    /* Initialize SW3 button input and enable dual-edge interrupt */
    gpio_pin_config_t sw_cfg = { kGPIO_DigitalInput, 0U };
    GPIO_PinInit(BOARD_SW_GPIO, BOARD_SW_GPIO_PIN, &sw_cfg);
    GPIO_SetPinInterruptConfig(BOARD_SW_GPIO, BOARD_SW_GPIO_PIN, kGPIO_InterruptEitherEdge);
    EnableIRQ(BOARD_SW_IRQ);

    /* Start 1 ms SysTick counter */
    (void)SysTick_Config(SystemCoreClock / 1000U);
}

uint32_t platform_millis(void) {
    return g_systick_ms;
}

bool platform_button_raw_pressed(void) {
    /* Active-low push button: physical press pulls pin to 0U */
    return (GPIO_PinRead(BOARD_SW_GPIO, BOARD_SW_GPIO_PIN) == 0U);
}

bool platform_button_edge_pending(void) {
    return g_button_edge_pending;
}

void platform_button_clear_edge(void) {
    g_button_edge_pending = false;
}

void platform_led_set(led_color_t color) {
    /* Route all legacy solid color calls through the unified PWM/GPIO percent function! */
    switch (color) {
        case LED_RED:     platform_led_set_rgb_percent(100U, 0U,   0U);   break;
        case LED_GREEN:   platform_led_set_rgb_percent(0U,   100U, 0U);   break;
        case LED_BLUE:    platform_led_set_rgb_percent(0U,   0U,   100U); break;
        case LED_YELLOW:  platform_led_set_rgb_percent(100U, 100U, 0U);   break;
        case LED_CYAN:    platform_led_set_rgb_percent(0U,   100U, 100U); break;
        case LED_MAGENTA: platform_led_set_rgb_percent(100U, 0U,   100U); break;
        case LED_WHITE:   platform_led_set_rgb_percent(100U, 100U, 100U); break;
        case LED_OFF:
        default:          platform_led_set_rgb_percent(0U,   0U,   0U);   break;
    }
}

void platform_led_set_rgb_percent(uint8_t red, uint8_t green, uint8_t blue) {
    /* Clamp duty cycle percentages to [0, 100] */
    uint8_t r = (red > 100U)   ? 100U : red;
    uint8_t g = (green > 100U) ? 100U : green;
    uint8_t b = (blue > 100U)  ? 100U : blue;

#if defined(ENABLE_WORKSHOP_PWM)
    /* Route ALL duty cycle updates to their physical controllers! */
    PWM_UpdatePwmDutycycle(FLEXPWM1, kPWM_Module_2, kPWM_PwmA, kPWM_SignedCenterAligned, (uint8_t)(100U - r)); /* Red */
    PWM_UpdatePwmDutycycle(FLEXPWM1, kPWM_Module_2, kPWM_PwmB, kPWM_SignedCenterAligned, (uint8_t)(100U - g)); /* Green */
    PWM_UpdatePwmDutycycle(FLEXPWM0, kPWM_Module_0, kPWM_PwmA, kPWM_SignedCenterAligned, (uint8_t)(100U - b)); /* Blue */

    /* Tell the hardware to load the updated values! */
    PWM_SetPwmLdok(FLEXPWM1, kPWM_Control_Module_2, true);
    PWM_SetPwmLdok(FLEXPWM0, kPWM_Control_Module_0, true);
#else
    write_led_pins(r >= 50U, g >= 50U, b >= 50U);
#endif
}

bool platform_temperature_read_c(float *temperature_c) {
    if (temperature_c == NULL) {
        return false;
    }

    uint8_t rx_buf[2] = {0U, 0U};
    lpi2c_master_transfer_t xfer;
    memset(&xfer, 0, sizeof(xfer));

    xfer.flags          = kLPI2C_TransferDefaultFlag;
    xfer.slaveAddress   = P3T1755_I2C_ADDR_7BIT;
    xfer.direction      = kLPI2C_Read;
    xfer.subaddress     = P3T1755_REG_TEMP;
    xfer.subaddressSize = 1U;
    xfer.data           = rx_buf;
    xfer.dataSize       = sizeof(rx_buf);

    status_t status = LPI2C_MasterTransferBlocking(BOARD_SENSOR_LPI2C_BASEADDR, &xfer);
    if (status != kStatus_Success) {
        return false;
    }

    /* Combine MSB and LSB as signed 16-bit integer, arithmetic-shift right 4 bits for signed 12-bit value */
    int16_t raw16 = (int16_t)(((uint16_t)rx_buf[0] << 8U) | (uint16_t)rx_buf[1]);
    int16_t raw12 = (int16_t)(raw16 >> 4);
    *temperature_c = (float)raw12 * P3T1755_LSB_CELSIUS;
    return true;
}

void platform_console_write(const char *text) {
    if (text != NULL) {
        PRINTF("%s", text);
    }
}

```

---

## 7. Module 1: Get Started with GPIO & First Debug Session (70 Mins)

### Overview
The first module establishes the complete edit-build-flash-debug cycle while introducing digital GPIO inputs, active-low RGB LED outputs, and hardware abstraction. You will build a color selector in which each valid short press of push button **SW3** (`P0_6`) advances the onboard RGB LED (`D12`) through **Red $\rightarrow$ Green $\rightarrow$ Blue $\rightarrow$ Off**.

> **Hardware Note (Active-Low Polarity):** On the FRDM-MCXA156, the RGB LED anodes are tied to $3.3\text{ V}$ (`VDD`) and the cathodes are driven by MCU pins `P3_12` (Red), `P3_13` (Green), and `P3_0` (Blue). Writing a logical `0` ($0\text{ V}$) turns an LED channel **ON**, and writing a logical `1` ($3.3\text{ V}$) turns it **OFF**. Similarly, pressing push button **SW3** (`P0_6`) pulls the pin to `0` (`GND`). Our `workshop_platform.c` adapter encapsulates this active-low polarity so your application code works with intuitive logical states (`LED_RED`, `LED_GREEN`, `LED_BLUE`, `LED_OFF`, and `true` when pressed).

### Step-by-Step Procedure

1. **Add the Module 1 Starter File:**
   * Copy `module-01-gpio/starter/main.c` from your GitHub repository folder into your MCUXpresso project's `source/` folder (replacing any existing `main.c`).
   * Double-click `source/main.c` in **Project Explorer** to open it in the editor:
     ```c
     #include "workshop_platform.h"
     #include "workshop_events.h"

     int main(void) {
         platform_init();

         /* TODO 1: Declare a button_filter_t variable and a color index (0 = OFF, 1 = RED, 2 = GREEN, 3 = BLUE). */
         /* TODO 2: Initialize the button filter with button_filter_init(). */

         while (1) {
             /* TODO 3: Poll button_filter_update() and advance the RGB color on each BUTTON_EVENT_SHORT_PRESS. */
         }
     }
     ```
2. **Explore the Project Files:**
   * Briefly inspect `board/pin_mux.c`, `board/clock_config.c`, `board/board.c`, and `startup/startup_mcxa156.c` in **Project Explorer** to see how MCUXpresso SDK initializes the Cortex-M33 vector table, $96\text{ MHz}$ FRO clock, and pins before `main()` runs.
3. **Complete the Starter Exercise:**
   * Implement the three `TODO` items so that `color` starts at `0U` (`LED_OFF`), `button_filter_init()` initializes the debounce structure, and each `BUTTON_EVENT_SHORT_PRESS` returned by `button_filter_update()` advances `color = (uint8_t)((color + 1U) % 4U)` and calls `platform_led_set((led_color_t)color)`.
   * Compare your code against the reference solution (`module-01-gpio/solution/main.c`):
     ```c
     #include "workshop_platform.h"
     #include "workshop_events.h"

     int main(void) {
         button_filter_t button;
         uint8_t color = 0U; /* 0: LED_OFF, 1: LED_RED, 2: LED_GREEN, 3: LED_BLUE */

         platform_init();
         button_filter_init(&button, platform_button_raw_pressed(), platform_millis());

         while (1) {
             button_event_t e = button_filter_update(&button, platform_button_raw_pressed(), platform_millis());
             if (e == BUTTON_EVENT_SHORT_PRESS) {
                 color = (uint8_t)((color + 1U) % 4U);
                 platform_led_set((led_color_t)color);
             }
         }
     }
     ```
4. **Build the Project:**
   * Click **Build** in the **Quickstart Panel**. Confirm `0 errors, 0 warnings` in the **Console** view.
5. **Start Your First Debug Session & Inspect Variables:**
   * Click **Debug** in the **Quickstart Panel**. When the **Debug Emulator Selection** dialog opens, select **MCU-Link CMSIS-DAP** and click **OK**.
   * The debugger flashes the image and halts on the first line of `main()`.
   * Set a breakpoint on the line `color = (uint8_t)((color + 1U) % 4U);` by double-clicking in the blue margin to the left of the line number.
   * Click **Resume** (`F8` or green play icon). Press and release **SW3** on the FRDM-MCXA156 board.
   * Execution halts at your breakpoint! Open the **Variables** tab (top-right pane) and inspect `button.stable_pressed`, `e`, and `color`. Open the **Peripherals+** tab (next to Project Explorer) and check `GPIO3` to see the memory-mapped register state.
   * Remove the breakpoint (double-click it again), click **Resume** (`F8`), and click **Terminate** (red square icon) to let the board run standalone.

### Module 1 Validation Checkpoint
* [x] Project builds with zero errors.
* [x] Debugger connects, halts at `main()`, and hits the button-press breakpoint via MCU-Link.
* [x] Short presses of **SW3** cycle the RGB LED through **Red $\rightarrow$ Green $\rightarrow$ Blue $\rightarrow$ Off**.
* [x] Holding **SW3** down continuously does not repeatedly cycle colors.

---

## 8. Module 2: Generate PWM and a Breathing RGB LED (60 Mins)

### Overview
Digital GPIO outputs can only switch an LED channel fully ON (`0%` pin level) or fully OFF (`100%` pin level). In Module 2, you use Pulse-Width Modulation (PWM) to vary the perceived brightness of the RGB LED between $0\%$ and $100\%$ and create a smooth, nonblocking "breathing" Cyan effect that updates every $20\text{ ms}$.

> **Hardware Routing Note:** Board and SDK revisions expose specific hardware timer/PWM routing options for the RGB LED pins (`P3_12`, `P3_13`, `P3_0`). You can inspect the exact timer/PWM signals available for these pins using **MCUXpresso Config Tools -> Open Pins** (right-click your project -> **MCUXpresso Config Tools -> Open Pins**). Because the RGB LED is active-low, `platform_led_set_rgb_percent(red, green, blue)` clamps inputs above `100U` to `100U` and inverts the duty cycle (`100U - level`) so `0%` corresponds to fully off and `100%` corresponds to maximum brightness.

### Step-by-Step Procedure

1. **Inspect Pin Routing in the Pins Tool (Optional Exploration):**
   * Right-click your project in **Project Explorer** and select **MCUXpresso Config Tools -> Open Pins**.
   * Filter by `P3_12`, `P3_13`, and `P3_0` in the **Pins** table view to see how each RGB LED pin can be routed to GPIO or timer/PWM peripherals (`PWM0`/`PWM1` or `CTIMER`). Close the Config Tools perspective to return to **C/C++ (Develop)** perspective (top-right toolbar).
2. **Replace `source/main.c` with Module 2 Starter:**
   * Copy `module-02-pwm/starter/main.c` from your GitHub repository folder and paste it over `source/main.c` in your project:
     ```c
     #include "workshop_platform.h"
     #include <stdint.h>

     int main(void) {
         platform_init();

         /* TODO 1: Create state variables for last update timestamp (uint32_t), brightness level (0-100), and step direction (+1 or -1). */

         while (1) {
             /* TODO 2: Every 20 ms (using wrap-safe unsigned subtraction), update platform_led_set_rgb_percent(0U, level, level)
              * and reverse direction at 0% and 100%. Do not use blocking delays. */
         }
     }
     ```
3. **Implement the Nonblocking $20\text{ ms}$ Ramp:**
   * Complete the `TODO` items using `(uint32_t)(now - last) >= 20U`. Notice how `level` is checked at the boundaries (`level >= 100U` sets `direction = -1`; `level == 0U` sets `direction = 1`) before adding `direction` so unsigned underflow never occurs.
   * Compare your implementation with `module-02-pwm/solution/main.c`:
     ```c
     #include "workshop_platform.h"
     #include <stdint.h>

     int main(void) {
         uint32_t last = 0U;
         uint8_t level = 0U;
         int8_t direction = 1;

         platform_init();

         while (1) {
             uint32_t now = platform_millis();
             if ((uint32_t)(now - last) >= 20U) {
                 last = now;
                 platform_led_set_rgb_percent(0U, level, level);

                 if (level >= 100U) {
                     direction = -1;
                 } else if (level == 0U) {
                     direction = 1;
                 }
                 level = (uint8_t)((int16_t)level + direction);
             }
         }
     }
     ```
4. **Build, Debug, and Experiment:**
   * Click **Build** and **Debug** in the **Quickstart Panel**.
   * Set a breakpoint inside the `if ((uint32_t)(now - last) >= 20U)` block and press **Resume (`F8`)** three times while watching `now`, `last`, and `level` in the **Variables** view. Verify that `now` advances by $20\text{ ms}$ on each hit and `level` increments by `1`.
   * Remove the breakpoint, resume execution, and try changing `platform_led_set_rgb_percent(0U, level, level)` (Cyan) to `(level, 0U, level)` (Magenta) or `(level, level, 0U)` (Yellow).

### Module 2 Validation Checkpoint
* [x] LED transitions smoothly without blocking the main loop.
* [x] Debugger confirms `level` updates only when `(uint32_t)(now - last) >= 20U`.
* [x] Duty cycle remains strictly bounded between $0\%$ and $100\%$.

---

## 9. Module 3: Read Temperature Over $\text{I}^2\text{C}$ (`LPI2C0`) & Produce CSV Telemetry (75 Mins)

### Overview
The onboard **NXP P3T1755DP** digital temperature sensor reports temperature in a two-byte register (`0x00`) at $7\text{-bit}$ slave address `0x48`. On the unmodified **FRDM-MCXA156**, the sensor is accessed through **`LPI2C0`** (`P0_16` SDA, `P0_17` SCL).

The $16\text{-bit}$ register value returned by the P3T1755DP contains a **signed $12\text{-bit}$ two's-complement number** left-aligned in bits `[15:4]` (with bits `[3:0]` reserved as zero) at a resolution of **$0.0625^\circ\text{C}$ per LSB** ($\frac{1}{16}^\circ\text{C}$):

$$\text{raw}_{16} = (\text{MSB} \ll 8) \mid \text{LSB}$$

$$\text{raw}_{12} = \frac{\text{raw}_{16}}{16} \quad (\text{signed arithmetic right shift } \gg 4)$$

$$T_{^\circ\text{C}} = \text{raw}_{12} \times 0.0625^\circ\text{C}, \qquad T_{^\circ\text{F}} = \left(T_{^\circ\text{C}} \times \frac{9}{5}\right) + 32$$

For example, if `rx_buf[0] = 0x17` and `rx_buf[1] = 0x80`:
* $\text{raw}_{16} = \text{0x1780} = 6016$
* $\text{raw}_{12} = \text{0x1780} \gg 4 = \text{0x0178} = 376$
* $T_{^\circ\text{C}} = 376 \times 0.0625 = 23.50^\circ\text{C}$ and $T_{^\circ\text{F}} = 74.30^\circ\text{F}$

### Step-by-Step Procedure

1. **Inspect the $\text{I}^2\text{C}$ Transfer & Conversion in `workshop_platform.c`:**
   * Open `common/source/workshop_platform.c` and review `platform_temperature_read_c()`.
   * Notice how `LPI2C_MasterTransferBlocking(LPI2C0, &xfer)` transmits the $1\text{-byte}$ register subaddress `0x00` to $7\text{-bit}$ address `0x48` and reads `2` bytes into `rx_buf`. If the bus transfer fails (`status != kStatus_Success`), it immediately returns `false`.
2. **Replace `source/main.c` with Module 3 Starter:**
   * Copy `module-03-temperature/starter/main.c` from your GitHub repository folder and paste it over `source/main.c` in your project:
     ```c
     #include "workshop_platform.h"
     #include "workshop_temperature.h"
     #include <stdio.h>

     int main(void) {
         platform_init();

         /* TODO 1: Print the 3-column CSV header ("timestamp_ms,temperature_c,temperature_f\r\n") once after reset. */

         while (1) {
             /* TODO 2: Every 1000 ms, read P3T1755DP via platform_temperature_read_c(&c).
              * If valid, format Celsius and Fahrenheit to 2 decimal places; otherwise print ERROR,ERROR. */
         }
     }
     ```
3. **Implement the $1000\text{ ms}$ Telemetry Loop:**
   * Complete `TODO 1` and `TODO 2` using `platform_temperature_read_c(&c)`, `celsius_to_fahrenheit(c)`, `snprintf()`, and `platform_console_write()`.
   * Compare your code with the reference solution (`module-03-temperature/solution/main.c`):
     ```c
     #include "workshop_platform.h"
     #include "workshop_temperature.h"
     #include <stdio.h>

     int main(void) {
         uint32_t last = 0U;
         char line[96];
         float c = 0.0F;

         platform_init();
         platform_console_write("timestamp_ms,temperature_c,temperature_f\r\n");

         while (1) {
             uint32_t now = platform_millis();
             if ((uint32_t)(now - last) >= 1000U) {
                 last = now;
                 if (platform_temperature_read_c(&c)) {
                     snprintf(line, sizeof(line), "%lu,%.2f,%.2f\r\n",
                              (unsigned long)now,
                              (double)c,
                              (double)celsius_to_fahrenheit(c));
                 } else {
                     snprintf(line, sizeof(line), "%lu,ERROR,ERROR\r\n", (unsigned long)now);
                 }
                 platform_console_write(line);
             }
         }
     }
     ```
4. **Open the Serial Terminal at $115200\text{ Baud}$ (`8-N-1`):**
   * **Using Tera Term:** Open Tera Term, select **Serial**, choose the **MCU-Link VCom Port (COMx)**, and open **Setup -> Serial port...** to verify **Speed: `115200`**, **Data: `8 bit`**, **Parity: `none`**, **Stop: `1 bit`**, **Flow control: `none`**.
   * **Or Using MCUXpresso Built-In Terminal:** Click the **Open a Terminal** icon inside the **Terminal** tab at the bottom of MCUXpresso IDE, select **Serial Terminal**, choose your COM port, set **Baud Rate** to **`115200`**, and click **OK**.
5. **Build, Flash, and Test Thermal Response:**
   * Build and run the project. Press the hardware Reset button (`SW1`) on the board and verify the CSV output in your terminal:
     ```text
     timestamp_ms,temperature_c,temperature_f
     1000,23.50,74.30
     2000,23.56,74.41
     3000,23.62,74.52
     ```
   * Gently place your fingertip on or near the onboard `P3T1755DP` sensor package (`U5`) and watch the Celsius and Fahrenheit values rise smoothly each second.

### Optional Extension (FRDM-MCXA153 Only): Native $\text{I3C}$ Comparison
*Participants using an **FRDM-MCXA153** board (which supports native $\text{I3C}$ without hardware modifications) who finish early may optionally import `driver_examples -> i3c -> master_read_sensor_p3t1755` into a separate project to compare $\text{I3C}$ dynamic address assignment (`RSTDAA` / `SETDASA`) against the static $7\text{-bit}$ `0x48` $\text{I}^2\text{C}$ transaction used in the required workshop path.*

### Module 3 Validation Checkpoint
* [x] `platform_temperature_read_c()` returns `true` (`kStatus_Success`) over `LPI2C0`.
* [x] Room-temperature readings are plausible ($\approx 20^\circ\text{C}\text{--}26^\circ\text{C}$) and increase when warmed.
* [x] CSV header (`timestamp_ms,temperature_c,temperature_f`) appears once after reset.
* [x] Each CSV record contains three comma-separated fields with two decimal places.

---

## 10. Module 4: Process Interrupts, Timers, and Button Events (70 Mins)

### Overview
Mechanical push buttons exhibit contact bounce—rapid electrical chatter lasting several milliseconds when pressed or released. Furthermore, an Interrupt Service Routine (ISR) should never execute blocking debounce delays, `snprintf()` formatting, or UART transmissions.

In Module 4, you separate **interrupt capture** from **main-loop event processing**:
1. **In the GPIO ISR (`BOARD_SW_IRQ_HANDLER` in `workshop_platform.c`):** Clears the hardware interrupt flag, sets `g_button_edge_pending = true`, and exits in less than $1\ \mu\text{s}$.
2. **In the Main Loop (`workshop_events.c` & `main.c`):** Checks `platform_button_edge_pending()` and runs the nonblocking $30\text{ ms}$ debounce filter (`button_filter_update()`). Once a stable press-and-release sequence completes:
   * A hold duration $< 1500\text{ ms}$ emits **`SHORT_PRESS`** and **increments** the event counter (`count++`).
   * A hold duration $\ge 1500\text{ ms}$ emits **`LONG_PRESS`** and **resets** the event counter (`count = 0U`).

### Step-by-Step Procedure

1. **Inspect `common/source/workshop_events.c`:**
   * Open `common/source/workshop_events.c` and trace how `button_filter_update()` resets `candidate_since_ms` whenever the raw pin bounces, waits until `(uint32_t)(now - f->candidate_since_ms) >= BUTTON_DEBOUNCE_MS` ($30\text{ ms}$), records `pressed_since_ms` on stable press, and compares `(uint32_t)(now - f->pressed_since_ms) >= LONG_PRESS_TIME_MS` ($1500\text{ ms}$) on stable release.
2. **Replace `source/main.c` with Module 4 Starter:**
   * Copy `module-04-events/starter/main.c` from your GitHub repository folder and paste it over `source/main.c`:
     ```c
     #include "workshop_platform.h"
     #include "workshop_events.h"
     #include <stdio.h>

     int main(void) {
         platform_init();

         /* TODO 1: Initialize button_filter_t and print CSV header "timestamp_ms,button_event,event_count\r\n". */
         /* TODO 2: In while(1), check platform_button_edge_pending() or active debounce state,
          * classify SHORT_PRESS (<1500 ms) vs LONG_PRESS (>=1500 ms), increment count on short press
          * (reset on long press), and print CSV line on each event. */
         while (1) {
         }
     }
     ```
3. **Complete the Event Loop:**
   * Implement `TODO 1` and `TODO 2` and compare your code with `module-04-events/solution/main.c`:
     ```c
     #include "workshop_platform.h"
     #include "workshop_events.h"
     #include <stdio.h>

     int main(void) {
         button_filter_t b;
         uint32_t count = 0U;
         char line[80];
         bool filtering_active = false;

         platform_init();
         button_filter_init(&b, platform_button_raw_pressed(), platform_millis());
         platform_console_write("timestamp_ms,button_event,event_count\r\n");

         while (1) {
             uint32_t now = platform_millis();

             /* Acknowledge asynchronous GPIO edge interrupt flag from ISR */
             if (platform_button_edge_pending()) {
                 platform_button_clear_edge();
                 filtering_active = true;
             }

             /* Run nonblocking debounce & hold-duration classifier when edge occurred or button is active */
             if (filtering_active || b.stable_pressed || (b.candidate_pressed != b.stable_pressed)) {
                 button_event_t e = button_filter_update(&b, platform_button_raw_pressed(), now);
                 if (e != BUTTON_EVENT_NONE) {
                     if (e == BUTTON_EVENT_LONG_PRESS) {
                         count = 0U;
                     } else {
                         count++;
                     }
                     snprintf(line, sizeof(line), "%lu,%s,%lu\r\n",
                              (unsigned long)now,
                              button_event_name(e),
                              (unsigned long)count);
                     platform_console_write(line);
                     filtering_active = false;
                 }
             }
         }
     }
     ```
4. **Build, Flash, and Verify in Terminal:**
   * Build and run the program. In your serial terminal ($115200\text{ baud}$), tap **SW3** three times briefly ($< 1.5\text{ s}$), then hold **SW3** down for $2\text{ seconds}$ and release:
     ```text
     timestamp_ms,button_event,event_count
     1420,SHORT_PRESS,1
     2850,SHORT_PRESS,2
     4110,SHORT_PRESS,3
     7390,LONG_PRESS,0
     ```

### Module 4 Validation Checkpoint
* [x] GPIO ISR records an edge (`g_button_edge_pending = true`) and exits immediately.
* [x] One physical press-and-release produces exactly one debounced event.
* [x] Presses shorter than $1.5\text{ s}$ print `SHORT_PRESS` and increment `event_count`.
* [x] Presses held at least $1.5\text{ s}$ print `LONG_PRESS` and reset `event_count` to `0`.

---

## 11. Module 5: Capstone — Nonblocking Environmental Monitor (100 Mins)

### Overview
In the capstone module, you combine GPIO, sensor acquisition, button event classification, LED state visualization, and $7\text{-field}$ CSV telemetry into a cooperative, multi-rate system.

#### System State & Threshold Specification (`workshop_config.h`)

| Temperature / Sensor Condition | Monitor State (`state`) | RGB LED Behavior (`led_mode`) |
| :--- | :--- | :--- |
| `platform_temperature_read_c()` returns `false` | `SENSOR_ERROR` | Flashing Magenta at $250\text{ ms}$ (`MAGENTA_FLASH`) |
| $T_{^\circ\text{C}} < 18.0^\circ\text{C}$ (`COLD_THRESHOLD_C`) | `COLD` | Solid Blue (`BLUE`) |
| $18.0^\circ\text{C} \le T_{^\circ\text{C}} < 27.0^\circ\text{C}$ | `NORMAL` | Solid Green (`GREEN`) |
| $27.0^\circ\text{C} \le T_{^\circ\text{C}} < 30.0^\circ\text{C}$ (`WARNING_THRESHOLD_C`) | `WARNING` | Solid Yellow (`YELLOW`) |
| $T_{^\circ\text{C}} \ge 30.0^\circ\text{C}$ (`ALARM_THRESHOLD_C`) | `ALARM` | Flashing Red at $250\text{ ms}$ (`RED_FLASH`) |

#### Cooperative Task Rates
1. **Startup ($t = 0\text{ ms}$):** Read the sensor immediately before entering `while(1)` so the initial LED state and $t = 0\text{ ms}$ telemetry row are accurate (preventing any false `SENSOR_ERROR` flash during the first second after boot).
2. **Button Event Task (Continuous / Every Loop Iteration):** Acknowledge GPIO edge interrupts and run `button_filter_update()`. Latch any `SHORT_PRESS` (increments `event_count`) or `LONG_PRESS` (resets `event_count = 0U`) in `last_event` until the next $1000\text{ ms}$ telemetry report prints it, then clear `last_event = BUTTON_EVENT_NONE`.
3. **Temperature Sampling Task (`TEMP_SAMPLE_INTERVAL_MS = 1000U`):** Read the P3T1755DP over `LPI2C0`, classify `state`, and immediately update the LED.
4. **Alarm / Error Flash Task (`LED_FLASH_INTERVAL_MS = 250U`):** Toggle `flash = !flash` and call `apply_led(state, flash)` so `ALARM` (`RED_FLASH`) and `SENSOR_ERROR` (`MAGENTA_FLASH`) blink at $2\text{ Hz}$ ($250\text{ ms}$ ON / $250\text{ ms}$ OFF) without affecting sensor or serial timing.
5. **CSV Telemetry Task (`TELEMETRY_INTERVAL_MS = 1000U`):** Output the $7\text{-field}$ CSV record:
   ```text
   timestamp_ms,temperature_c,temperature_f,state,button_event,event_count,led_mode
   ```

### Step-by-Step Procedure

1. **Replace `source/main.c` with Module 5 Starter:**
   * Copy `module-05-capstone/starter/main.c` from your GitHub repository folder into your project's `source/main.c`.
2. **Implement the State Machine and Multi-Rate Scheduler:**
   * Write the helper functions `state_name()`, `led_name()`, `classify()`, and `apply_led()`, followed by the nonblocking `main()` loop.
   * Compare your implementation against the complete reference solution (`module-05-capstone/solution/main.c`):

```c
#include "workshop_platform.h"
#include "workshop_config.h"
#include "workshop_events.h"
#include "workshop_temperature.h"
#include <stdio.h>

typedef enum {
    STATE_SENSOR_ERROR = 0,
    STATE_COLD,
    STATE_NORMAL,
    STATE_WARNING,
    STATE_ALARM
} monitor_state_t;

static const char *state_name(monitor_state_t s) {
    static const char *n[] = {"SENSOR_ERROR", "COLD", "NORMAL", "WARNING", "ALARM"};
    return n[s];
}

static const char *led_name(monitor_state_t s) {
    static const char *n[] = {"MAGENTA_FLASH", "BLUE", "GREEN", "YELLOW", "RED_FLASH"};
    return n[s];
}

static monitor_state_t classify(float c) {
    if (c < COLD_THRESHOLD_C)    return STATE_COLD;
    if (c < WARNING_THRESHOLD_C) return STATE_NORMAL;
    if (c < ALARM_THRESHOLD_C)   return STATE_WARNING;
    return STATE_ALARM;
}

static void apply_led(monitor_state_t state, bool flash_on) {
    if (state == STATE_SENSOR_ERROR) {
        platform_led_set(flash_on ? LED_MAGENTA : LED_OFF);
    } else if (state == STATE_COLD) {
        platform_led_set(LED_BLUE);
    } else if (state == STATE_NORMAL) {
        platform_led_set(LED_GREEN);
    } else if (state == STATE_WARNING) {
        platform_led_set(LED_YELLOW);
    } else {
        platform_led_set(flash_on ? LED_RED : LED_OFF);
    }
}

int main(void) {
    button_filter_t button;
    button_event_t last_event = BUTTON_EVENT_NONE;
    uint32_t event_count = 0U;
    float c = 0.0F;
    bool flash = false;
    char line[180];

    platform_init();
    uint32_t now = platform_millis();
    button_filter_init(&button, platform_button_raw_pressed(), now);

    /* Sample immediately at startup so LED state and t=0 CSV telemetry are accurate */
    bool valid = platform_temperature_read_c(&c);
    monitor_state_t state = valid ? classify(c) : STATE_SENSOR_ERROR;
    apply_led(state, flash);

    platform_console_write("timestamp_ms,temperature_c,temperature_f,state,button_event,event_count,led_mode\r\n");
    if (valid) {
        snprintf(line, sizeof(line), "%lu,%.2f,%.2f,%s,%s,%lu,%s\r\n",
                 (unsigned long)now, (double)c, (double)celsius_to_fahrenheit(c),
                 state_name(state), button_event_name(last_event),
                 (unsigned long)event_count, led_name(state));
    } else {
        snprintf(line, sizeof(line), "%lu,ERROR,ERROR,%s,%s,%lu,%s\r\n",
                 (unsigned long)now, state_name(state),
                 button_event_name(last_event), (unsigned long)event_count,
                 led_name(state));
    }
    platform_console_write(line);

    uint32_t last_sample = now;
    uint32_t last_report = now;
    uint32_t last_flash  = now;

    while (1) {
        now = platform_millis();

        if (platform_button_edge_pending()) {
            platform_button_clear_edge();
        }

        button_event_t e = button_filter_update(&button, platform_button_raw_pressed(), now);
        if (e != BUTTON_EVENT_NONE) {
            last_event = e;
            if (e == BUTTON_EVENT_LONG_PRESS) {
                event_count = 0U;
            } else {
                event_count++;
            }
        }

        if ((uint32_t)(now - last_sample) >= TEMP_SAMPLE_INTERVAL_MS) {
            last_sample = now;
            valid = platform_temperature_read_c(&c);
            state = valid ? classify(c) : STATE_SENSOR_ERROR;
            apply_led(state, flash);
        }

        if ((uint32_t)(now - last_flash) >= LED_FLASH_INTERVAL_MS) {
            last_flash = now;
            flash = !flash;
            apply_led(state, flash);
        }

        if ((uint32_t)(now - last_report) >= TELEMETRY_INTERVAL_MS) {
            last_report = now;
            if (valid) {
                snprintf(line, sizeof(line), "%lu,%.2f,%.2f,%s,%s,%lu,%s\r\n",
                         (unsigned long)now, (double)c, (double)celsius_to_fahrenheit(c),
                         state_name(state), button_event_name(last_event),
                         (unsigned long)event_count, led_name(state));
            } else {
                snprintf(line, sizeof(line), "%lu,ERROR,ERROR,%s,%s,%lu,%s\r\n",
                         (unsigned long)now, state_name(state),
                         button_event_name(last_event), (unsigned long)event_count,
                         led_name(state));
            }
            platform_console_write(line);
            last_event = BUTTON_EVENT_NONE;
        }
    }
}
```

3. **Test All System States & Concurrency:**
   * Build, flash, and monitor the serial terminal at $115200\text{ baud}$.
   * **Normal & Button Test:** At room temperature ($\approx 23.5^\circ\text{C}$), confirm the RGB LED is solid **Green** (`NORMAL`). Press **SW3** briefly and confirm the next $1000\text{ ms}$ row reports `SHORT_PRESS` and increments `event_count`.
   * **Warning & Alarm Test:** Warm the P3T1755DP sensor with a finger (or temporarily lower `WARNING_THRESHOLD_C` to `24.5F` and `ALARM_THRESHOLD_C` to `26.0F` in `common/include/workshop_config.h` and rebuild) to observe the transition from **Green (`NORMAL`) $\rightarrow$ Yellow (`WARNING`) $\rightarrow$ Flashing Red (`ALARM` / `RED_FLASH`)**. While the LED is flashing Red at $250\text{ ms}$, press **SW3** (both short and long press) to verify that button events and $1000\text{ ms}$ CSV output continue with zero pause or jitter.
   * **Error Handling Test:** Temporarily change `P3T1755_I2C_ADDR_7BIT` in `workshop_platform.c` from `0x48U` to `0x49U`, rebuild, and verify that the system immediately enters `SENSOR_ERROR`, flashes **Magenta (`MAGENTA_FLASH`)** every $250\text{ ms}$, and outputs `ERROR,ERROR,SENSOR_ERROR`. Restore `0x48U` and rebuild when finished.

#### Expected Capstone Serial Output
```text
timestamp_ms,temperature_c,temperature_f,state,button_event,event_count,led_mode
0,23.50,74.30,NORMAL,NONE,0,GREEN
1000,23.56,74.41,NORMAL,NONE,0,GREEN
2000,27.18,80.92,WARNING,SHORT_PRESS,1,YELLOW
3000,30.12,86.22,ALARM,NONE,1,RED_FLASH
4000,ERROR,ERROR,SENSOR_ERROR,NONE,1,MAGENTA_FLASH
5000,17.50,63.50,COLD,LONG_PRESS,0,BLUE
```

---

## 12. Troubleshooting Guide & Complete Validation Matrix

### Common Classroom Troubleshooting

| Symptom | Cause | Solution |
| :--- | :--- | :--- |
| **`fatal error: workshop_platform.h: No such file or directory`** | `common/include` path was not added to the active Build Configuration. | Open **Project -> Properties -> C/C++ Build -> Settings -> MCU C Compiler -> Includes** and add `"${workspace_loc:/${ProjName}/common/include}"`. |
| **`undefined reference to 'platform_init'` (or `button_filter_init`)** | `board-support` or `common` folder is excluded from build or missing. | Right-click `board-support` and `common` in **Project Explorer** -> **Resource Configurations -> Exclude from build...** -> uncheck **Debug** and **Release**. |
| **`multiple definition of 'main'` linker error** | The original SDK example `.c` file is still inside `source/`. | Delete `gpio_led_output.c` / `hello_world.c` from `source/` so only your module's `main.c` remains. |
| **Temperature columns print blank (`1000,,`)** | Floating-point `printf`/`snprintf` is disabled in the C library. | Ensure `__asm(".global _printf_float");` is in `workshop_platform.c` and `CR_INTEGER_PRINTF` is removed from **MCU C Compiler -> Preprocessor**. |
| **Temperature reads `ERROR,ERROR`** | Wrong slave address or attempting to use native $\text{I3C}$ on an unmodified FRDM-MCXA156. | Verify `P3T1755_I2C_ADDR_7BIT` is `0x48U` and `LPI2C0` (`P0_16`/`P0_17`) is used on FRDM-MCXA156. |
| **LED colors appear inverted (e.g., Cyan instead of Red)** | Active-low RGB polarity was written directly in `main.c`. | Always call `platform_led_set()` in `workshop_platform.c`, which writes `0U` for ON and `1U` for OFF. |
| **No output in serial terminal** | Wrong COM port, wrong baud rate, or project imported with `Semihost` instead of `UART`. | Verify **MCU-Link VCom Port** at **$115200\text{ baud}$ (`8-N-1`)**, and check **Quickstart Panel -> Quick Settings >> -> SDK Debug Console -> UART console**. |

### Final Workshop Validation Matrix

| Module | Verification Item | Pass Criteria |
| :--- | :--- | :--- |
| **Module 1** | Build, debug, and GPIO cycle | Zero build errors; short press cycles `RED -> GREEN -> BLUE -> OFF`; holding button does not auto-repeat. |
| **Module 2** | Nonblocking PWM ramp | Brightness updates every $20\text{ ms}$ without blocking `while(1)`; duty cycle clamped to $[0\%, 100\%]$. |
| **Module 3** | `LPI2C0` sensor read & 3-field CSV | Reads P3T1755DP (`0x48`, reg `0x00`) once per second; prints header once and `timestamp_ms,temperature_c,temperature_f` rows. |
| **Module 4** | Interrupt capture & press classifier | GPIO ISR sets `volatile` edge flag and exits immediately; $30\text{ ms}$ filter rejects bounce; `<1500 ms` increments count, `>=1500 ms` resets count to `0`. |
| **Module 5** | Multi-rate environmental monitor | Samples and prints $7\text{-field}$ CSV every $1000\text{ ms}$; flashes `ALARM` (Red) or `SENSOR_ERROR` (Magenta) every $250\text{ ms}$ while processing button events with zero blocking delays. |
