#include "workshop_platform.h"
#include "board.h"
#include "clock_config.h"
#include "pin_mux.h"
#include "fsl_common.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
#include "fsl_debug_console.h"

/* Automatically enable floating-point printf/snprintf in MCUXpresso Newlib-nano */
#if defined(__GNUC__)
__asm(".global _printf_float");
#endif

/* Optional SDK peripheral drivers enabled per module */
#if defined(FSL_FEATURE_SOC_PWM_COUNT) && (FSL_FEATURE_SOC_PWM_COUNT > 0) && defined(ENABLE_WORKSHOP_PWM)
#include "fsl_pwm.h"
#elif defined(FSL_FEATURE_SOC_CTIMER_COUNT) && (FSL_FEATURE_SOC_CTIMER_COUNT > 0) && defined(ENABLE_WORKSHOP_CTIMER_PWM)
#include "fsl_ctimer.h"
#endif

#if defined(FSL_FEATURE_SOC_I3C_COUNT) && (FSL_FEATURE_SOC_I3C_COUNT > 0) && defined(BOARD_SENSOR_USE_I3C_I2C_MODE)
#include "fsl_i3c.h"
#elif defined(FSL_FEATURE_SOC_LPI2C_COUNT) && (FSL_FEATURE_SOC_LPI2C_COUNT > 0)
#include "fsl_lpi2c.h"
#endif

/* P3T1755DP Constants (Board Adapter Contract) */
#define P3T1755_I2C_ADDR_7BIT   (0x48U)
#define P3T1755_REG_TEMP        (0x00U)
#define P3T1755_LSB_CELSIUS     (0.0625F)

/* Fallback board macro mappings for FRDM-MCXA156 / FRDM-MCXA153 */
#ifndef BOARD_SW_GPIO
  #if defined(BOARD_SW3_GPIO)
    #define BOARD_SW_GPIO       BOARD_SW3_GPIO
    #define BOARD_SW_GPIO_PIN   BOARD_SW3_GPIO_PIN
    #define BOARD_SW_IRQ        BOARD_SW3_IRQ
    #define BOARD_SW_IRQ_HANDLER BOARD_SW3_IRQ_HANDLER
  #elif defined(BOARD_SW2_GPIO)
    #define BOARD_SW_GPIO       BOARD_SW2_GPIO
    #define BOARD_SW_GPIO_PIN   BOARD_SW2_GPIO_PIN
    #define BOARD_SW_IRQ        BOARD_SW2_IRQ
    #define BOARD_SW_IRQ_HANDLER BOARD_SW2_IRQ_HANDLER
  #else
    /* Default FRDM-MCXA156 SW3 (P0_6) */
    #define BOARD_SW_GPIO       GPIO0
    #define BOARD_SW_GPIO_PIN   (6U)
    #define BOARD_SW_IRQ        GPIO0_IRQn
    #define BOARD_SW_IRQ_HANDLER GPIO0_IRQHandler
  #endif
#endif

#ifndef BOARD_LED_RED_GPIO
  /* Default FRDM-MCXA156 RGB LED D12: Red=P3_12, Green=P3_13, Blue=P3_0 */
  #define BOARD_LED_RED_GPIO        GPIO3
  #define BOARD_LED_RED_GPIO_PIN    (12U)
  #define BOARD_LED_GREEN_GPIO      GPIO3
  #define BOARD_LED_GREEN_GPIO_PIN  (13U)
  #define BOARD_LED_BLUE_GPIO       GPIO3
  #define BOARD_LED_BLUE_GPIO_PIN   (0U)
#endif

static volatile uint32_t g_systick_ms = 0U;
static volatile bool g_button_edge_pending = false;

/* 1 ms System Tick ISR */
void SysTick_Handler(void) {
    g_systick_ms++;
}

/* Nonblocking GPIO Button Edge ISR (Module 4 Validation Requirement) */
void BOARD_SW_IRQ_HANDLER(void) {
    uint32_t flags = GPIO_GpioGetInterruptFlags(BOARD_SW_GPIO);
    GPIO_GpioClearInterruptFlags(BOARD_SW_GPIO, flags);
    g_button_edge_pending = true;
    SDK_ISR_EXIT_BARRIER;
}

static void write_led_pins(bool red_on, bool green_on, bool blue_on) {
    /* Onboard RGB LED is active-low: 0U turns channel ON, 1U turns channel OFF */
    GPIO_PinWrite(BOARD_LED_RED_GPIO, BOARD_LED_RED_GPIO_PIN, red_on ? 0U : 1U);
    GPIO_PinWrite(BOARD_LED_GREEN_GPIO, BOARD_LED_GREEN_GPIO_PIN, green_on ? 0U : 1U);
    GPIO_PinWrite(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PIN, blue_on ? 0U : 1U);
}

void platform_init(void) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Initialize active-low RGB LED outputs to OFF (logical high = 1U) */
    gpio_pin_config_t led_cfg = { kGPIO_DigitalOutput, 1U };
    GPIO_PinInit(BOARD_LED_RED_GPIO, BOARD_LED_RED_GPIO_PIN, &led_cfg);
    GPIO_PinInit(BOARD_LED_GREEN_GPIO, BOARD_LED_GREEN_GPIO_PIN, &led_cfg);
    GPIO_PinInit(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PIN, &led_cfg);
    platform_led_set(LED_OFF);

    /* Initialize button input and enable both-edge interrupt for Module 4 */
    gpio_pin_config_t sw_cfg = { kGPIO_DigitalInput, 0U };
    GPIO_PinInit(BOARD_SW_GPIO, BOARD_SW_GPIO_PIN, &sw_cfg);
    GPIO_SetPinInterruptConfig(BOARD_SW_GPIO, BOARD_SW_GPIO_PIN, kGPIO_InterruptEitherEdge);
    EnableIRQ(BOARD_SW_IRQ);

    /* Configure 1 ms SysTick timebase */
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
    /* Clamp duty cycle percentages to [0, 100] per Validation Checklist */
    uint8_t r = (red > 100U)   ? 100U : red;
    uint8_t g = (green > 100U) ? 100U : green;
    uint8_t b = (blue > 100U)  ? 100U : blue;

#if defined(ENABLE_WORKSHOP_PWM) && defined(BOARD_PWM_BASEADDR)
    /* Active-low polarity inversion: 0% brightness = 100% high level, 100% brightness = 0% high level */
    PWM_UpdatePwmDutycycle(BOARD_PWM_BASEADDR, kPWM_Module_0, kPWM_PwmA, kPWM_SignedCenterAligned, (uint8_t)(100U - r));
    PWM_UpdatePwmDutycycle(BOARD_PWM_BASEADDR, kPWM_Module_1, kPWM_PwmA, kPWM_SignedCenterAligned, (uint8_t)(100U - g));
    PWM_UpdatePwmDutycycle(BOARD_PWM_BASEADDR, kPWM_Module_2, kPWM_PwmA, kPWM_SignedCenterAligned, (uint8_t)(100U - b));
    PWM_SetPwmLdok(BOARD_PWM_BASEADDR, kPWM_Control_Module_0 | kPWM_Control_Module_1 | kPWM_Control_Module_2, true);
#elif defined(ENABLE_WORKSHOP_CTIMER_PWM) && defined(CTIMER0)
    CTIMER_UpdatePwmDutycycle(CTIMER0, kCTIMER_Match_0, kCTIMER_Match_1, (uint8_t)(100U - g));
    CTIMER_UpdatePwmDutycycle(CTIMER0, kCTIMER_Match_0, kCTIMER_Match_2, (uint8_t)(100U - b));
#else
    /* Fallback threshold behaviour if hardware PWM peripheral is not yet routed in Pins tool */
    write_led_pins(r >= 50U, g >= 50U, b >= 50U);
#endif
}

bool platform_temperature_read_c(float *temperature_c) {
    if (temperature_c == NULL) {
        return false;
    }

    uint8_t rx_buf[2] = {0U, 0U};
    status_t status = kStatus_Fail;

#if defined(FSL_FEATURE_SOC_I3C_COUNT) && (FSL_FEATURE_SOC_I3C_COUNT > 0) && defined(BOARD_SENSOR_USE_I3C_I2C_MODE)
    /* Unmodified FRDM-MCXA156 routes P3T1755DP to I3C0; use legacy I2C-compatible transfer at 0x48 */
    i3c_master_transfer_t xfer;
    memset(&xfer, 0, sizeof(xfer));
    xfer.flags          = kI3C_TransferDefaultFlag;
    xfer.slaveAddress   = P3T1755_I2C_ADDR_7BIT;
    xfer.direction      = kI3C_Read;
    xfer.subaddress     = P3T1755_REG_TEMP;
    xfer.subaddressSize = 1U;
    xfer.data           = rx_buf;
    xfer.dataSize       = sizeof(rx_buf);
    xfer.busType        = kI3C_TypeI2C;
    status = I3C_MasterTransferBlocking(I3C0, &xfer);
#elif defined(FSL_FEATURE_SOC_LPI2C_COUNT) && (FSL_FEATURE_SOC_LPI2C_COUNT > 0) && defined(BOARD_SENSOR_LPI2C_BASEADDR)
    lpi2c_master_transfer_t xfer;
    memset(&xfer, 0, sizeof(xfer));
    xfer.flags          = kLPI2C_TransferDefaultFlag;
    xfer.slaveAddress   = P3T1755_I2C_ADDR_7BIT;
    xfer.direction      = kLPI2C_Read;
    xfer.subaddress     = P3T1755_REG_TEMP;
    xfer.subaddressSize = 1U;
    xfer.data           = rx_buf;
    xfer.dataSize       = sizeof(rx_buf);
    status = LPI2C_MasterTransferBlocking(BOARD_SENSOR_LPI2C_BASEADDR, &xfer);
#endif

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
