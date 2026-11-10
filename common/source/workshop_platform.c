#include "workshop_platform.h"
#include "board.h"
#include "clock_config.h"
#include "pin_mux.h"
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
    PORT_SetPinMux(PORT3, 12U, kPORT_MuxAlt0);
    PORT_SetPinMux(PORT3, 13U, kPORT_MuxAlt0);
    PORT_SetPinMux(PORT3, 0U,  kPORT_MuxAlt0);
    PORT_SetPinMux(PORT0, 6U,  kPORT_MuxAlt0);

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

void platform_init(void) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    init_sensor_lpi2c0_pins_and_bus();

    /* Initialize active-low RGB LED outputs to OFF (1U) */
    gpio_pin_config_t led_cfg = { kGPIO_DigitalOutput, 1U };
    GPIO_PinInit(BOARD_LED_RED_GPIO, BOARD_LED_RED_GPIO_PIN, &led_cfg);
    GPIO_PinInit(BOARD_LED_GREEN_GPIO, BOARD_LED_GREEN_GPIO_PIN, &led_cfg);
    GPIO_PinInit(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PIN, &led_cfg);
    platform_led_set(LED_OFF);

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
    switch (color) {
        case LED_RED:     write_led_pins(true,  false, false); break;
        case LED_GREEN:   write_led_pins(false, true,  false); break;
        case LED_BLUE:    write_led_pins(false, false, true);  break;
        case LED_YELLOW:  write_led_pins(true,  true,  false); break;
        case LED_CYAN:    write_led_pins(false, true,  true);  break;
        case LED_MAGENTA: write_led_pins(true,  false, true);  break;
        case LED_WHITE:   write_led_pins(true,  true,  true);  break;
        case LED_OFF:
        default:          write_led_pins(false, false, false); break;
    }
}

void platform_led_set_rgb_percent(uint8_t red, uint8_t green, uint8_t blue) {
    /* Clamp duty cycle percentages to [0, 100] */
    uint8_t r = (red > 100U)   ? 100U : red;
    uint8_t g = (green > 100U) ? 100U : green;
    uint8_t b = (blue > 100U)  ? 100U : blue;

#if defined(ENABLE_WORKSHOP_PWM) && defined(BOARD_PWM_BASEADDR)
    PWM_UpdatePwmDutycycle(BOARD_PWM_BASEADDR, kPWM_Module_0, kPWM_PwmA, kPWM_SignedCenterAligned, (uint8_t)(100U - r));
    PWM_UpdatePwmDutycycle(BOARD_PWM_BASEADDR, kPWM_Module_1, kPWM_PwmA, kPWM_SignedCenterAligned, (uint8_t)(100U - g));
    PWM_UpdatePwmDutycycle(BOARD_PWM_BASEADDR, kPWM_Module_2, kPWM_PwmA, kPWM_SignedCenterAligned, (uint8_t)(100U - b));
    PWM_SetPwmLdok(BOARD_PWM_BASEADDR, kPWM_Control_Module_0 | kPWM_Control_Module_1 | kPWM_Control_Module_2, true);
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
