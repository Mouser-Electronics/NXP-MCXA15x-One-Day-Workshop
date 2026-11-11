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
