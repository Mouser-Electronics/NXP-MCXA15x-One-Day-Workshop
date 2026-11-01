# NXP MCXA15x One-Day Hands-On Workshop

This repository supports a one-day, onboard-hardware-only embedded systems workshop for the **NXP FRDM-MCXA156** (primary reference target) and **NXP FRDM-MCXA153** (supported target) development boards. Using **MCUXpresso IDE v25.6 [Build 136]** and **MCUXpresso SDK 26.09.00**, participants build five progressively integrated bare-metal C applications that culminate in a nonblocking environmental telemetry and alarm monitor.

---

## Workshop Learning Path

| Module | Topic | Key Embedded Concepts |
| :--- | :--- | :--- |
| **Module 1** | **GPIO & First Debug Session** | Active-low RGB LED control, push-button polling, hardware abstraction, breakpoints, and variable inspection via MCU-Link. |
| **Module 2** | **PWM & Breathing RGB LED** | Hardware pulse-width modulation, duty-cycle clamping ($0\%$ to $100\%$), active-low polarity inversion, and nonblocking $20\text{ ms}$ updates. |
| **Module 3** | **P3T1755DP Temperature & CSV** | $\text{I}^2\text{C}$-compatible register reads ($7\text{-bit}$ address `0x48`, register `0x00`), signed $12\text{-bit}$ two's-complement conversion ($0.0625^\circ\text{C/LSB}$), and $3\text{-field}$ UART CSV telemetry. |
| **Module 4** | **Interrupts, Timers & Button Events** | Dual-edge GPIO Interrupt Service Routines (ISRs), `volatile` event flags, nonblocking $30\text{ ms}$ state-machine debouncing, and short-press ($< 1500\text{ ms}$) vs. long-press ($\ge 1500\text{ ms}$) classification. |
| **Module 5** | **Capstone Environmental Monitor** | Cooperative multi-rate scheduling ($1000\text{ ms}$ sensor sampling, $1000\text{ ms}$ $7\text{-field}$ CSV reporting, $250\text{ ms}$ alarm/error LED flashing, and asynchronous button counter control) with zero blocking delays. |

---

## Hardware & Software Requirements

### Hardware
* **Development Board:** One **NXP FRDM-MCXA156** (or **FRDM-MCXA153**) board.
* **USB Cable:** USB Type-C cable connected to the onboard **MCU-Link** debug probe port (`J21` on FRDM-MCXA156).
* **Host Computer:** Windows 10/11 or Linux PC with an available USB port.
* *Note:* **No external components, breadboards, jumper wires, or resistor modifications are required.** All exercises use factory-populated onboard hardware (RGB LED `D12`, user button `SW3`/`SW2`, and onboard `P3T1755DP` temperature sensor).

### Software
* **MCUXpresso IDE:** Version **25.06 [Build 136]** (or newer) — [Download from NXP](https://www.nxp.com/mcuxpresso/ide)
* **MCUXpresso SDK:** **SDK_2.x_FRDM-MCXA156 (v26.09.00)** (or **FRDM-MCXA153**) built specifically for the **MCUXpresso IDE** toolchain — install directly via the IDE *Installed SDKs* view or download from [MCUXpresso SDK Builder](https://mcuxpresso.nxp.com/).
* **Serial Terminal:** [Tera Term](https://teratermproject.github.io/), PuTTY, or the built-in MCUXpresso IDE **Terminal** view configured for **$115200\text{ baud}$, $8$ data bits, no parity, $1$ stop bit ($8\text{-N-}1$), and no flow control**.
* **Git (Optional):** [Download Git](https://git-scm.com/downloads) to clone this repository, or download the repository as a `.zip` archive.

---

## Repository Directory Structure

The workshop separates portable application logic from board-specific SDK drivers so the teaching code remains clean and readable across modules:

```text
NXP-MCXA15x-One-Day-Workshop/
├── README.md                          # Repository overview and quick-start guide
├── LICENSE                            # MIT License
├── common/
│   ├── include/
│   │   ├── workshop_platform.h        # Hardware abstraction layer (HAL) interface
│   │   ├── workshop_config.h          # Timing intervals and temperature thresholds
│   │   ├── workshop_events.h          # Nonblocking button debounce & event types
│   │   └── workshop_temperature.h     # Temperature unit conversion interface
│   └── source/
│       ├── workshop_events.c          # 30 ms debounce filter & hold-duration classifier
│       └── workshop_temperature.c     # Celsius to Fahrenheit conversion
├── board-support/
│   ├── README.md                      # Board Adapter Contract specification
│   └── workshop_platform.c            # FRDM-MCXA156 / FRDM-MCXA153 SDK 26.09.00 implementation
├── module-01-gpio/
│   ├── starter/main.c                 # Module 1 student starter template
│   └── solution/main.c                # Module 1 reference solution
├── module-02-pwm/
│   ├── starter/main.c                 # Module 2 student starter template
│   └── solution/main.c                # Module 2 reference solution
├── module-03-temperature/
│   ├── starter/main.c                 # Module 3 student starter template
│   └── solution/main.c                # Module 3 reference solution
├── module-04-events/
│   ├── starter/main.c                 # Module 4 student starter template
│   └── solution/main.c                # Module 4 reference solution
├── module-05-capstone/
│   ├── starter/main.c                 # Module 5 student starter template
│   └── solution/main.c                # Module 5 reference solution
└── docs/
    ├── NXP_MCXA15x_Workshop_Project_Article.md # Complete student step-by-step walkthrough
    ├── workshop-schedule.md           # 1-day instructor and classroom schedule
    ├── software-setup.md              # Preflight checklist and installation notes
    ├── csv-format.md                  # Telemetry CSV schema for Modules 3, 4, and 5
    ├── validation.md                  # Module-by-module verification checklist
    └── troubleshooting.md             # Common hardware, build, and serial fixes
```

---

## Quick-Start: Setting Up Your MCUXpresso IDE v25.6 Project

### 1. Clone or Download This Repository
Open a terminal or command prompt and clone the repository to a known folder on your computer (or download and extract the ZIP archive from GitHub):
```bash
git clone https://github.com/Mouser-Electronics/NXP-MCXA15x-One-Day-Workshop.git
```

### 2. Install the FRDM-MCXA156 SDK (v26.09.00)
1. Launch **MCUXpresso IDE v25.6** and select a writable workspace directory (with no spaces or special characters in the path).
2. Locate the **Installed SDKs** view in the bottom panel of the IDE.
3. Drag and drop your downloaded `SDK_2_x_FRDM-MCXA156.zip` archive directly into the **Installed SDKs** view (or click the **Import archive...** icon in the top-right corner of the **Installed SDKs** view) and confirm installation.

### 3. Import the Base SDK Example Project
To ensure all pin multiplexers, clocks, debug console UART settings, and sensor bus drivers (`I3C0`/`LPI2C` and `GPIO`) are pre-configured by the NXP SDK:
1. In the **Quickstart Panel** (bottom-left of the IDE), click **Import SDK example(s)...**.
2. Click the **frdmmcxa156** board image and click **Next >**.
3. In the **SDK Import Wizard**:
   * **Project name suffix:** Enter `_workshop` (creates a uniquely named project).
   * **Project Options -> SDK Debug Console:** Select **UART** (routes `PRINTF` to the MCU-Link Virtual COM port).
   * **Project Options -> Copy sources:** Ensure **Copy sources** and **Import other files** are **checked**.
   * **Examples filter:** Type `p3t1755` and check `driver_examples -> i3c -> master_read_sensor_p3t1755` (recommended because it includes both GPIO and P3T1755DP sensor bus initialization out of the box) *or* type `hello_world` and check `demo_apps -> hello_world`.
4. Click **Finish**.

### 4. Add the Workshop Files from GitHub to Your Project
1. Open your file manager to your cloned `NXP-MCXA15x-One-Day-Workshop` folder.
2. Copy the **`common`** folder and **`board-support`** folder from the GitHub repository, right-click the top-level project name in MCUXpresso IDE's **Project Explorer**, and click **Paste** (or drag and drop them onto the project and select **Copy files and folders**).
3. Expand the `source/` folder inside your MCUXpresso project in **Project Explorer** and **delete** the default SDK example application file (for example, `i3c_master_read_sensor_p3t1755.c` or `hello_world.c`) so it does not conflict with the workshop's `main()`.
4. Copy `module-01-gpio/starter/main.c` from the GitHub repository and **Paste** it into the project's `source/` folder.

### 5. Configure the `Debug` Build Configuration
1. Select your project in **Project Explorer** and click **Edit project settings** under *Miscellaneous* in the **Quickstart Panel** (or right-click the project and select **Properties**).
2. In the left pane, expand **C/C++ Build** and click **Settings**. Confirm at the top that **Configuration:** is set to **`Debug [Active]`** (which uses `-O0` optimization for accurate source-level debugging).
3. **Add the Include Path:**
   * Under the **Tool Settings** tab, select **MCU C Compiler -> Includes**.
   * In the **Include paths (-I)** box, click the **Add...** (`+`) icon, click **Workspace...**, select the `common/include` folder inside your project, and click **OK**.
4. **Add Preprocessor Symbols:**
   * Select **MCU C Compiler -> Preprocessor**.
   * In the **Defined symbols (-D)** box, click **Add...** (`+`) and add:
     ```text
     BOARD_SENSOR_USE_I3C_I2C_MODE=1
     ```
5. **Verify Floating-Point `printf`/`snprintf` Support:**
   * Select **MCU Linker -> Managed Linker Script** (or click **Quick Settings >> -> Set library/header type** in the Quickstart Panel) and ensure **Enable printf float** (`-u _printf_float`) is enabled (this is also automatically enforced in `board-support/workshop_platform.c`).
6. Click **Apply and Close**.

### 6. Build, Flash, and Debug
1. Connect the **FRDM-MCXA156** board to your computer using the **MCU-Link USB Type-C** connector (`J21`).
2. In the **Quickstart Panel**, click **Build** (hammer icon) and verify `0 errors, 0 warnings` in the **Console** view.
3. Click **Debug** (blue bug icon) in the **Quickstart Panel**. Select the discovered **MCU-Link CMSIS-DAP** probe, click **OK**, and click **Resume** (`F8`) once execution halts at `main()`.

For complete, step-by-step instructions for every module, open **[`docs/NXP_MCXA15x_Workshop_Project_Article.md`](docs/NXP_MCXA15x_Workshop_Project_Article.md)**.

---

## Classroom Schedule (8 Hours 30 Minutes)

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
| 2:40 – 4:20 | Module 5: Nonblocking environmental monitor capstone | $100\text{ min}$ |
| 4:20 – 5:00 | Validation, troubleshooting, and review | $40\text{ min}$ |

---

## License
This workshop repository is licensed under the **MIT License**. See [`LICENSE`](LICENSE) for details.