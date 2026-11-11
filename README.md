# NXP MCXA15x One-Day Hands-On Workshop

This repository supports a one-day, onboard-hardware-only embedded systems workshop for the **NXP FRDM-MCXA156** (primary reference target) and **NXP FRDM-MCXA153** (supported target) development boards. Using **MCUXpresso IDE v25.6 [Build 136]** and **MCUXpresso SDK 26.09.00**, participants build five progressively integrated bare-metal C applications that culminate in a cooperative, nonblocking environmental telemetry and alarm monitor.

---

## Workshop Learning Path

| Module | Topic | Key Embedded Concepts |
| :--- | :--- | :--- |
| **Module 1** | **GPIO & First Debug Session** | Active-low RGB LED control, push-button polling, hardware abstraction, breakpoints, and variable inspection via MCU-Link. |
| **Module 2** | **PWM & Breathing RGB LED** | Hardware pulse-width modulation, duty-cycle clamping ($0\%$ to $100\%$), active-low polarity inversion, and nonblocking $20\text{ ms}$ updates. |
| **Module 3** | **P3T1755DP Temperature & CSV** | Standard $\text{I}^2\text{C}$ (`LPI2C0`) register reads ($7\text{-bit}$ address `0x48`, register `0x00`), signed $12\text{-bit}$ two's-complement conversion ($0.0625^\circ\text{C/LSB}$), and $3\text{-field}$ UART CSV telemetry. |
| **Module 4** | **Interrupts, Timers & Button Events** | Dual-edge GPIO Interrupt Service Routines (ISRs), `volatile` event flags, nonblocking $30\text{ ms}$ state-machine debouncing, and short-press ($< 1500\text{ ms}$) vs. long-press ($\ge 1500\text{ ms}$) classification. |
| **Module 5** | **Capstone Environmental Monitor** | Cooperative multi-rate scheduling ($1000\text{ ms}$ sensor sampling, $1000\text{ ms}$ $7\text{-field}$ CSV reporting, $250\text{ ms}$ alarm/error LED flashing, and asynchronous button counter control) with zero blocking delays. |

---

## Hardware & Software Requirements

### Hardware
* **Development Board:** One **NXP FRDM-MCXA156** (or **FRDM-MCXA153**) board.
* **USB Cable:** Data-capable USB Type-C cable connected to the onboard **MCU-Link** debug probe port (`J21` on FRDM-MCXA156).
* **Host Computer:** Windows 10/11 or Linux PC supplied by the attendee.
* **No Hardware Modifications Required:** No external sensor, breadboard, jumper wire, resistor removal, or soldering is required. On the **FRDM-MCXA156**, the onboard **P3T1755DP** temperature sensor is factory-configured for standard $\text{I}^2\text{C}$-compatible operation (`LPI2C0` on pins `P0_16`/`P0_17`) without removing any resistors. Native $\text{I3C}$ operation is covered only as a conceptual topic and an optional extension for the FRDM-MCXA153.

### Software
* **MCUXpresso IDE:** Version **25.06 [Build 136]** — [Download from NXP](https://www.nxp.com/mcuxpresso/ide)
* **MCUXpresso SDK:** **SDK_2.x_FRDM-MCXA156 (v26.09.00)** (or **FRDM-MCXA153**) installed directly inside MCUXpresso IDE via the *Installed SDKs* view or downloaded from [MCUXpresso SDK Builder](https://mcuxpresso.nxp.com/).
* **Serial Terminal:** [Tera Term](https://teratermproject.github.io/), PuTTY, or the built-in MCUXpresso IDE **Terminal** view configured for **$115200\text{ baud}$, $8$ data bits, no parity, $1$ stop bit ($8\text{-N-}1$), and no flow control**.
* **Git (Optional):** [Download Git](https://git-scm.com/downloads) to clone this repository, or download the repository as a `.zip` archive.

---

## Repository Directory Structure

The workshop separates portable application logic (`common/` and `module-01` through `module-05`) from board-specific SDK driver calls (`board-support/`) so the teaching code remains clean and readable:

```text
NXP-MCXA15x-One-Day-Workshop/
├── README.md                                          # Repository overview and quick-start guide
├── LICENSE                                            # MIT License
├── common/
│   ├── include/
│   │   ├── workshop_platform.h                        # Hardware abstraction layer (HAL) interface
│   │   ├── workshop_config.h                          # Timing intervals and temperature thresholds
│   │   ├── workshop_events.h                          # Nonblocking button debounce & event types
│   │   └── workshop_temperature.h                     # Temperature unit conversion interface
│   └── source/
│       ├── workshop_events.c                          # 30 ms debounce filter & hold-duration classifier
│       ├── workshop_platform.c                        # FRDM-MCXA156 / FRDM-MCXA153 LPI2C & GPIO adapter
│       └── workshop_temperature.c                     # Celsius to Fahrenheit conversion
├── board-support/
│   ├── frdm_mcxa153/
│   │   └── README.md                                  # FRDM-MCXA153 board adapter contract
│   └── frdm_mcxa156/
│       └── README.md                                  # FRDM-MCXA156 board adapter contract
├── module-01-gpio/
│   ├── starter/main.c                                 # Module 1 student starter template
│   └── solution/main.c                                # Module 1 reference solution
├── module-02-pwm/
│   ├── starter/main.c                                 # Module 2 student starter template
│   └── solution/main.c                                # Module 2 reference solution
├── module-03-temperature/
│   ├── starter/main.c                                 # Module 3 student starter template
│   └── solution/main.c                                # Module 3 reference solution
├── module-04-events/
│   ├── starter/main.c                                 # Module 4 student starter template
│   └── solution/main.c                                # Module 4 reference solution
├── module-05-capstone/
│   ├── starter/main.c                                 # Module 5 student starter template
│   └── solution/main.c                                # Module 5 reference solution
└── docs/
    ├── NXP_MCXA15x_Workshop_Project_Article.md        # Detailed student walkthrough & project article
    ├── workshop-schedule.md                           # 1-day classroom schedule
    ├── software-setup.md                              # Preflight checklist and installation notes
    ├── csv-format.md                                  # Telemetry CSV schema for Modules 3, 4, and 5
    ├── validation.md                                  # Module-by-module verification checklist
    └── troubleshooting.md                             # Common hardware, build, and serial fixes
```

---

## Quick-Start: Setting Up Your MCUXpresso IDE v25.6 Project

### Step 1: Clone or Download This Repository
Clone the repository using Git or download and extract the ZIP archive from GitHub to your computer:
```bash
git clone https://github.com/Mouser-Electronics/NXP-MCXA15x-One-Day-Workshop.git
```

### Step 2: Install the Board SDK (`v26.09.00`)
1. Launch **MCUXpresso IDE v25.6** and accept or select a writable workspace directory.
2. Locate the **Installed SDKs** tab in the bottom panel of the IDE.
3. Drag and drop your `SDK_2_x_FRDM-MCXA156.zip` archive into the **Installed SDKs** view and click **OK** to install part support, flash drivers, and peripheral examples.

### Step 3: Import the Base SDK Example Project
1. In the **Quickstart Panel** (bottom-left pane), click **Import SDK example(s)...**.
2. Click the **frdmmcxa156** board image and click **Next >**.
3. In the **Import Wizard**:
   * **Project name suffix:** Enter `_workshop`.
   * **Project Options -> SDK Debug Console:** Select **UART** (ensures `PRINTF` outputs to the MCU-Link Virtual COM port rather than semihosting).
   * **Project Options -> Copy sources:** Leave **Copy sources** checked.
   * **Examples list:** Expand `driver_examples -> lpi2c` and check **`lpi2c_polling_b2b_master`** (or expand `demo_apps` and check **`hello_world`**).
4. Click **Finish**.
5. If you imported `hello_world` or `gpio_led_output`, click **Manage SDK Components** in the top toolbar (or right-click the project -> **SDK Management -> Manage SDK Components**), expand **Driver**, check **`lpi2c`** (and **`pwm`** / **`ctimer`** for Module 2), and click **OK** so the SDK driver sources are added to your project's `drivers/` folder.

### Step 4: Add the Workshop Files from GitHub
1. In your operating system file manager, open the extracted `NXP-MCXA15x-One-Day-Workshop` folder.
2. Select both the **`common`** and **`board-support`** folders, copy them (`Ctrl+C`), right-click your project root in MCUXpresso IDE's **Project Explorer**, and select **Paste** (`Ctrl+V`).
3. Expand your project's `source/` folder in **Project Explorer** and **delete** the default SDK example source file (such as `lpi2c_polling_b2b_master.c` or `hello_world.c`) so there is only one `main()` function in the project.
4. Copy `module-01-gpio/starter/main.c` from the GitHub repository folder and **Paste** it into your project's `source/` folder.

### Step 5: Configure the `Debug` Build Configuration
1. Select your project in **Project Explorer** and click **Edit project settings** under *Miscellaneous* in the **Quickstart Panel** (or press `Alt+Enter` / right-click -> **Properties**).
2. Navigate to **C/C++ Build -> Settings** and confirm that **Configuration:** at the top is set to **`Debug [Active]`** (`-O0` optimization).
3. **Add the Common Include Path:**
   * Go to **Tool Settings -> MCU C Compiler -> Includes**.
   * Next to **Include paths (-I)**, click the **Add...** (`+`) icon, click **Workspace...**, select your project's **`common/include`** folder, and click **OK**.
4. **Enable Floating-Point `printf` / `snprintf` (`%.2f`):**
   * In the **Quickstart Panel**, click **Quick Settings >> -> Set library/header type** and ensure floating-point `printf` is enabled, or go to **C/C++ Build -> Settings -> MCU Linker -> Managed Linker Script** (or **Miscellaneous**) and enable **`-u _printf_float`** (also enforced automatically in `common/source/workshop_platform.c`).
5. Click **Apply and Close**.

### Step 6: Build, Flash, and Debug
1. Connect the **FRDM-MCXA156** board to your PC via the **MCU-Link USB Type-C** connector (`J21`).
2. Click **Build** in the **Quickstart Panel** and verify `0 errors` in the **Console** view.
3. Click **Debug** in the **Quickstart Panel**, select the **MCU-Link CMSIS-DAP** probe, click **OK**, and use **Resume (`F8`)** once the debugger halts at `main()`.

For the full, module-by-module classroom walkthrough, open **[`docs/NXP_MCXA15x_Workshop_Project_Article.md`](docs/NXP_MCXA15x_Workshop_Project_Article.md)**.

---

## Classroom Schedule (8 Hours 30 Minutes)

| Time | Activity | Duration |
| :--- | :--- | :--- |
| 8:30 – 8:50 | Welcome, board check, and repository access | $20\text{ min}$ |
| 8:50 – 10:00 | Module 1: GPIO and first debug session | $70\text{ min}$ |
| 10:00 – 10:15 | Break | $15\text{ min}$ |
| 10:15 – 11:15 | Module 2: PWM and RGB LED | $60\text{ min}$ |
| 11:15 – 12:30 | Module 3: Temperature and CSV telemetry (`LPI2C0`) | $75\text{ min}$ |
| 12:30 – 1:15 | Lunch | $45\text{ min}$ |
| 1:15 – 2:25 | Module 4: Interrupts and button events | $70\text{ min}$ |
| 2:25 – 2:40 | Break | $15\text{ min}$ |
| 2:40 – 4:20 | Module 5: Nonblocking environmental monitor capstone | $100\text{ min}$ |
| 4:20 – 5:00 | Validation, troubleshooting, and review | $40\text{ min}$ |

---

## License
This workshop repository is licensed under the **MIT License**. See [`LICENSE`](LICENSE) for details.
