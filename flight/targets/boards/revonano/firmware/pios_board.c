/**
 ******************************************************************************
 * @file       pios_board.c
 * @author     The LibrePilot Project, http://www.librepilot.org Copyright (C) 2015.
 *             The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 *             PhoenixPilot, http://github.com/PhoenixPilot, Copyright (C) 2012
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotCore OpenPilot Core
 * @{
 * @brief Defines board specific static initializers for hardware for the revomini board.
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "inc/openpilot.h"
#include <pios_board_info.h>
#include <uavobjectsinit.h>
#include <hwsettings.h>
#include <taskinfo.h>
#include <pios_ws2811.h>

#include "sanitycheck.h"
#include "actuatorsettings.h"

#ifdef PIOS_INCLUDE_INSTRUMENTATION
#include <pios_instrumentation.h>
#endif

#include <pios_board_io.h>
#include <pios_board_sensors.h>

/*
 * Pull in the board-specific static HW definitions.
 * Including .c files is a bit ugly but this allows all of
 * the HW definitions to be const and static to limit their
 * scope.
 *
 * NOTE: THIS IS THE ONLY PLACE THAT SHOULD EVER INCLUDE THIS FILE
 */
#include "../board_hw_defs.c"

uintptr_t pios_uavo_settings_fs_id;
uintptr_t pios_user_fs_id;

#ifdef PIOS_INCLUDE_WS2811
uint32_t pios_ws2811_id;
#endif

static SystemAlarmsExtendedAlarmStatusOptions RevoNanoConfigHook();
static void ActuatorSettingsUpdatedCb(UAVObjEvent *ev);

/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */

#include <pios_board_info.h>

static const PIOS_BOARD_IO_UART_Function flexi_function_map[] = {
    [HWSETTINGS_RM_FLEXIPORT_TELEMETRY]      = PIOS_BOARD_IO_UART_TELEMETRY,
    [HWSETTINGS_RM_FLEXIPORT_GPS] = PIOS_BOARD_IO_UART_GPS,
    [HWSETTINGS_RM_FLEXIPORT_DSM] = PIOS_BOARD_IO_UART_DSM_FLEXI,
    [HWSETTINGS_RM_FLEXIPORT_EXBUS]          = PIOS_BOARD_IO_UART_EXBUS,
    [HWSETTINGS_RM_FLEXIPORT_HOTTSUMD]       = PIOS_BOARD_IO_UART_HOTT_SUMD,
    [HWSETTINGS_RM_FLEXIPORT_HOTTSUMH]       = PIOS_BOARD_IO_UART_HOTT_SUMH,
    [HWSETTINGS_RM_FLEXIPORT_SRXL] = PIOS_BOARD_IO_UART_SRXL,
    [HWSETTINGS_RM_FLEXIPORT_IBUS] = PIOS_BOARD_IO_UART_IBUS,
    [HWSETTINGS_RM_FLEXIPORT_DEBUGCONSOLE]   = PIOS_BOARD_IO_UART_DEBUGCONSOLE,
    [HWSETTINGS_RM_FLEXIPORT_COMBRIDGE]      = PIOS_BOARD_IO_UART_COMBRIDGE,
    [HWSETTINGS_RM_FLEXIPORT_OSDHK]          = PIOS_BOARD_IO_UART_OSDHK,
    [HWSETTINGS_RM_FLEXIPORT_MSP] = PIOS_BOARD_IO_UART_MSP,
    [HWSETTINGS_RM_FLEXIPORT_MAVLINK]        = PIOS_BOARD_IO_UART_MAVLINK,
    [HWSETTINGS_RM_FLEXIPORT_FRSKYSENSORHUB] = PIOS_BOARD_IO_UART_FRSKY_SENSORHUB,
};

static const PIOS_BOARD_IO_UART_Function main_function_map[] = {
    [HWSETTINGS_RM_MAINPORT_TELEMETRY] = PIOS_BOARD_IO_UART_TELEMETRY,
    [HWSETTINGS_RM_MAINPORT_GPS]            = PIOS_BOARD_IO_UART_GPS,
    [HWSETTINGS_RM_MAINPORT_SBUS]           = PIOS_BOARD_IO_UART_SBUS,
    [HWSETTINGS_RM_MAINPORT_DSM]            = PIOS_BOARD_IO_UART_DSM_MAIN,
    [HWSETTINGS_RM_MAINPORT_DEBUGCONSOLE]   = PIOS_BOARD_IO_UART_DEBUGCONSOLE,
    [HWSETTINGS_RM_MAINPORT_COMBRIDGE]      = PIOS_BOARD_IO_UART_COMBRIDGE,
    [HWSETTINGS_RM_MAINPORT_OSDHK]          = PIOS_BOARD_IO_UART_OSDHK,
    [HWSETTINGS_RM_MAINPORT_MSP]            = PIOS_BOARD_IO_UART_MSP,
    [HWSETTINGS_RM_MAINPORT_FRSKYSENSORHUB] = PIOS_BOARD_IO_UART_FRSKY_SENSORHUB,
};

void PIOS_Board_Init(void)
{
    const struct pios_board_info *bdinfo = &pios_board_info_blob;

#if defined(PIOS_INCLUDE_LED)
    const struct pios_gpio_cfg *led_cfg  = PIOS_BOARD_HW_DEFS_GetLedCfg(bdinfo->board_rev);
    PIOS_Assert(led_cfg);
    PIOS_LED_Init(led_cfg);
#endif /* PIOS_INCLUDE_LED */


#ifdef PIOS_INCLUDE_INSTRUMENTATION
    PIOS_Instrumentation_Init(PIOS_INSTRUMENTATION_MAX_COUNTERS);
#endif

    /* Set up the SPI interface to the gyro/acelerometer */
    if (PIOS_SPI_Init(&pios_spi_gyro_adapter_id, &pios_spi_gyro_cfg)) {
        PIOS_DEBUG_Assert(0);
    }

#ifdef PIOS_INCLUDE_I2C
    if (PIOS_I2C_Init(&pios_i2c_eeprom_pressure_adapter_id, &pios_i2c_eeprom_pressure_adapter_cfg)) {
        PIOS_DEBUG_Assert(0);
    }
#endif
#if defined(PIOS_INCLUDE_FLASH)
    /* Connect flash to the appropriate interface and configure it */

    uintptr_t flash_id = 0;

    // Initialize the external USER flash
    if (PIOS_Flash_EEPROM_Init(&flash_id, &flash_main_chip_cfg, pios_i2c_eeprom_pressure_adapter_id, 0x50)) {
        PIOS_DEBUG_Assert(0);
    }

    if (PIOS_FLASHFS_Init(&pios_uavo_settings_fs_id, &flash_main_fs_cfg, &pios_EEPROM_flash_driver, flash_id)) {
        PIOS_DEBUG_Assert(0);
    }

#endif /* if defined(PIOS_INCLUDE_FLASH) */

#if defined(PIOS_INCLUDE_RTC)
    PIOS_RTC_Init(&pios_rtc_main_cfg);
#endif
    /* IAP System Setup */
    PIOS_IAP_Init();
    // check for safe mode commands from gcs
    if (PIOS_IAP_ReadBootCmd(0) == PIOS_IAP_CLEAR_FLASH_CMD_0 &&
        PIOS_IAP_ReadBootCmd(1) == PIOS_IAP_CLEAR_FLASH_CMD_1 &&
        PIOS_IAP_ReadBootCmd(2) == PIOS_IAP_CLEAR_FLASH_CMD_2) {
        PIOS_FLASHFS_Format(pios_uavo_settings_fs_id);
        PIOS_IAP_WriteBootCmd(0, 0);
        PIOS_IAP_WriteBootCmd(1, 0);
        PIOS_IAP_WriteBootCmd(2, 0);
    }

#ifdef PIOS_INCLUDE_WDG
    PIOS_WDG_Init();
#endif

    /* Initialize the task monitor */
    if (PIOS_TASK_MONITOR_Initialize(TASKINFO_RUNNING_NUMELEM)) {
        PIOS_Assert(0);
    }

    /* Initialize the delayed callback library */
    PIOS_CALLBACKSCHEDULER_Initialize();

    /* Initialize UAVObject libraries */
    EventDispatcherInitialize();
    UAVObjInitialize();
    HwSettingsInitialize();

    /* Initialize the alarms library */
    AlarmsInitialize();

    /* Set up pulse timers */
    PIOS_TIM_InitClock(&tim_1_cfg);
    PIOS_TIM_InitClock(&tim_2_cfg);
    PIOS_TIM_InitClock(&tim_3_cfg);
    PIOS_TIM_InitClock(&tim_5_cfg);
    PIOS_TIM_InitClock(&tim_9_cfg);
    PIOS_TIM_InitClock(&tim_10_cfg);
    PIOS_TIM_InitClock(&tim_11_cfg);

    uint16_t boot_count = PIOS_IAP_ReadBootCount();
    if (boot_count < 3) {
        PIOS_IAP_WriteBootCount(++boot_count);
        AlarmsClear(SYSTEMALARMS_ALARM_BOOTFAULT);
    } else {
        /* Too many failed boot attempts, force hwsettings to defaults */
        HwSettingsSetDefaults(HwSettingsHandle(), 0);
        AlarmsSet(SYSTEMALARMS_ALARM_BOOTFAULT, SYSTEMALARMS_ALARM_CRITICAL);
    }

#if defined(PIOS_INCLUDE_USB)
    PIOS_BOARD_IO_Configure_USB();
#endif

    /* Configure FlexiPort */
    uint8_t hwsettings_flexiport;
    HwSettingsRM_FlexiPortGet(&hwsettings_flexiport);

    if (hwsettings_flexiport < NELEMENTS(flexi_function_map)) {
        PIOS_BOARD_IO_Configure_UART(&pios_usart_flexi_cfg, flexi_function_map[hwsettings_flexiport]);
    }

    uint8_t hwsettings_mainport;
    HwSettingsRM_MainPortGet(&hwsettings_mainport);

    if (hwsettings_mainport < NELEMENTS(main_function_map)) {
        PIOS_BOARD_IO_Configure_UART(&pios_usart_main_cfg, main_function_map[hwsettings_mainport]);
    }

#if defined(PIOS_INCLUDE_PWM) || defined(PIOS_INCLUDE_PPM)

    const struct pios_servo_cfg *pios_servo_cfg;
    // default to servo outputs only
    pios_servo_cfg = &pios_servo_cfg_out;
#endif

    /* Configure the receiver port*/
    uint8_t hwsettings_rcvrport;
    HwSettingsRM_RcvrPortGet(&hwsettings_rcvrport);
    //
    switch (hwsettings_rcvrport) {
    case HWSETTINGS_RM_RCVRPORT_DISABLED:
        break;
    case HWSETTINGS_RM_RCVRPORT_PWM:
#if defined(PIOS_INCLUDE_PWM)
        /* Set up the receiver port.  Later this should be optional */
        PIOS_BOARD_IO_Configure_PWM_RCVR(&pios_pwm_cfg);
#endif /* PIOS_INCLUDE_PWM */
        break;
    case HWSETTINGS_RM_RCVRPORT_PPM:
    case HWSETTINGS_RM_RCVRPORT_PPMOUTPUTS:
    case HWSETTINGS_RM_RCVRPORT_PPMPWM:
#if defined(PIOS_INCLUDE_PPM)
        if (hwsettings_rcvrport == HWSETTINGS_RM_RCVRPORT_PPMOUTPUTS) {
            // configure servo outputs and the remaining 5 inputs as outputs
            pios_servo_cfg = &pios_servo_cfg_out_in_ppm;
        }

        PIOS_BOARD_IO_Configure_PPM_RCVR(&pios_ppm_cfg);

        break;
#endif /* PIOS_INCLUDE_PPM */
    case HWSETTINGS_RM_RCVRPORT_OUTPUTS:
        // configure only the servo outputs
        pios_servo_cfg = &pios_servo_cfg_out_in;
        break;
    }


#ifdef PIOS_INCLUDE_GCSRCVR
    PIOS_BOARD_IO_Configure_GCS_RCVR();
#endif

#ifdef PIOS_INCLUDE_WS2811
#include <pios_ws2811.h>
    HwSettingsWS2811LED_OutOptions ws2811_pin_settings;
    HwSettingsWS2811LED_OutGet(&ws2811_pin_settings);

    // No other choices but servo pin 1 on nano
    if (ws2811_pin_settings != HWSETTINGS_WS2811LED_OUT_DISABLED) {
        pios_tim_servoport_all_pins[0] = dummmy_timer; // free timer 1
        PIOS_WS2811_Init(&pios_ws2811_id, &pios_ws2811_cfg, &pios_ws2811_pin_cfg[0]);
    }
#endif // PIOS_INCLUDE_WS2811


#ifndef PIOS_ENABLE_DEBUG_PINS
    // pios_servo_cfg points to the correct configuration based on input port settings
    PIOS_Servo_Init(pios_servo_cfg);
#else
    PIOS_DEBUG_Init(pios_tim_servoport_all_pins, NELEMENTS(pios_tim_servoport_all_pins));
#endif

    PIOS_DELAY_WaitmS(50);

    PIOS_BOARD_Sensors_Configure();

    // Attach the board config check hook
    SANITYCHECK_AttachHook(&RevoNanoConfigHook);
    // trigger a config check if actuatorsettings are updated
    ActuatorSettingsInitialize();
    ActuatorSettingsConnectCallback(ActuatorSettingsUpdatedCb);
}

SystemAlarmsExtendedAlarmStatusOptions RevoNanoConfigHook()
{
    // inhibit usage of oneshot for non supported RECEIVER port modes
    uint8_t recmode;

    HwSettingsRM_RcvrPortGet(&recmode);
    uint8_t modes[ACTUATORSETTINGS_BANKMODE_NUMELEM];
    ActuatorSettingsBankModeGet(modes);

    switch ((HwSettingsRM_RcvrPortOptions)recmode) {
    // Those modes allows oneshot usage
    case HWSETTINGS_RM_RCVRPORT_DISABLED:
    case HWSETTINGS_RM_RCVRPORT_PPM:
    case HWSETTINGS_RM_RCVRPORT_PPMOUTPUTS:
    case HWSETTINGS_RM_RCVRPORT_OUTPUTS:
        return SYSTEMALARMS_EXTENDEDALARMSTATUS_NONE;

    // inhibit oneshot for the following modes
    case HWSETTINGS_RM_RCVRPORT_PWM:
        for (uint8_t i = 0; i < ACTUATORSETTINGS_BANKMODE_NUMELEM; i++) {
            if (modes[i] == ACTUATORSETTINGS_BANKMODE_PWMSYNC ||
                modes[i] == ACTUATORSETTINGS_BANKMODE_ONESHOT125 ||
                modes[i] == ACTUATORSETTINGS_BANKMODE_ONESHOT42 ||
                modes[i] == ACTUATORSETTINGS_BANKMODE_MULTISHOT) {
                return SYSTEMALARMS_EXTENDEDALARMSTATUS_UNSUPPORTEDCONFIG_ONESHOT;;
            }
        }

        return SYSTEMALARMS_EXTENDEDALARMSTATUS_NONE;

    default:
        break;
    }

    return SYSTEMALARMS_EXTENDEDALARMSTATUS_UNSUPPORTEDCONFIG_ONESHOT;;
}

// trigger a configuration check if ActuatorSettings are changed.
void ActuatorSettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    configuration_check();
}
/**
 * @}
 * @}
 */
