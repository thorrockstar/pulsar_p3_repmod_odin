/**
 *  Copyright (c) 2020 Roy Schneider
 *
 *  main.h
 *
 *  Decllaration for the Pulsar P3 internal watch logic.
 *
 *  Project:            Pulsar Replacement module 'Odin'.
 *
 *  Programmer:         Roy Schneider
 *  Last Change:        19.12.2020
 *
 *  Language:           C
 *  Toolchain:          GCC/GNU-Make
 *
 * Software License Agreement
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 * The Free Software Foundation, Inc.
 * 59 Temple Place - Suite 330
 * Boston, MA  02111-1307, USA.
 *
 * As a special exception, if other files instantiate templates or
 * use macros or inline functions from this file, or you compile
 * this file and link it with other works to produce a work based
 * on this file, this file does not by itself cause the resulting
 * work to be covered by the GNU General Public License. However
 * the source code for this file must still be made available in
 * accordance with section (3) of the GNU General Public License.
 *
 * This exception does not invalidate any other reasons why a work
 * based on this file might be covered by the GNU General Public
 * License.
 */
 
/**
 * Define the possible watch types, supported by this WatchApp.
 */

// Original Litronix display using common cathodes.
#define APP_PULSAR_WRIST_WATCH_12H_NON_AUTO        0 // Pulsar P3 Odin module.
// USSR made L104G display using common anodes.
#define APP_PULSAR_WRIST_WATCH_24H_NON_AUTO        1 // Pulsar P3 Loki module.
// Using Litronix/Siemens bubble display.
#define APP_PROTOTYPE_BREAD_BOARD                  2 // Just my breadboard.
// Using VFD tubes.
#define APP_TABLE_WATCH                            3 // VFD watch project.

/**
 * Choose watch module type, that shall be build.
 */
 
#define APP_WATCH_TYPE_BUILD    APP_PULSAR_WRIST_WATCH_12H_NON_AUTO

/**
 * Define if the LED display has common cathode or anode. Basically
 * it is only the Loki module, were we have to manually set this as
 * we are using USSR made replacement displays with common cathode
 * or anode. The original Litronix displays were all common cathode.
 */

#define APP_WATCH_COMMON_CATHODE    0
#define APP_WATCH_COMMON_ANODE      1

#if (APP_WATCH_TYPE_BUILD==APP_PULSAR_WRIST_WATCH_12H_NON_AUTO)

    #define APP_WATCH_COMMON_PIN_USING  APP_WATCH_COMMON_CATHODE
    #define APP_WATCH_ANY_PULSAR_MODEL  1

#elif (APP_WATCH_TYPE_BUILD==APP_PULSAR_WRIST_WATCH_24H_NON_AUTO)

    #define APP_WATCH_COMMON_PIN_USING  APP_WATCH_COMMON_CATHODE
    #define APP_WATCH_ANY_PULSAR_MODEL  1

#elif (APP_WATCH_TYPE_BUILD==APP_PROTOTYPE_BREAD_BOARD)

    #define APP_WATCH_COMMON_PIN_USING  APP_WATCH_COMMON_CATHODE
    #define APP_WATCH_ANY_PULSAR_MODEL  0

#elif (APP_WATCH_TYPE_BUILD==APP_TABLE_WATCH))

    #define APP_WATCH_COMMON_PIN_USING  APP_WATCH_COMMON_CATHODE
    #define APP_WATCH_ANY_PULSAR_MODEL  0

#else

    #define APP_WATCH_COMMON_PIN_USING  APP_WATCH_COMMON_CATHODE
    #define APP_WATCH_ANY_PULSAR_MODEL  0

#endif

/**
 * Define to turn on display dimming via a LDR and the alarm feature.
 */

#if (APP_WATCH_TYPE_BUILD == APP_PULSAR_WRIST_WATCH_12H_NON_AUTO)

  #define APP_LIGHT_SENSOR_USAGE                     1
  #define APP_LIGHT_SENSOR_USAGE_DEBUG_SHOW_VALUE    1
  #define APP_BUZZER_ALARM_USAGE                     0
  #define APP_DATE_SPECIAL_DOT_USAGE                 0

#elif (APP_WATCH_TYPE_BUILD == APP_PULSAR_WRIST_WATCH_24H_NON_AUTO)

  #define APP_LIGHT_SENSOR_USAGE                     0
  #define APP_LIGHT_SENSOR_USAGE_DEBUG_SHOW_VALUE    0
  #define APP_BUZZER_ALARM_USAGE                     1
  #define APP_DATE_SPECIAL_DOT_USAGE                 1

#elif (APP_WATCH_TYPE_BUILD == APP_TABLE_WATCH)

  #define APP_LIGHT_SENSOR_USAGE                     1
  #define APP_LIGHT_SENSOR_USAGE_DEBUG_SHOW_VALUE    1
  #define APP_BUZZER_ALARM_USAGE                     0
  #define APP_DATE_SPECIAL_DOT_USAGE                 1

#else

  #define APP_LIGHT_SENSOR_USAGE                     1
  #define APP_LIGHT_SENSOR_USAGE_DEBUG_SHOW_VALUE    1
  #define APP_BUZZER_ALARM_USAGE                     0
  #define APP_DATE_SPECIAL_DOT_USAGE                 0

#endif

/**
* Defining the prototype of a handler called
* when a button has been pressed or hold pressed. */

typedef void(*ButtonHandlerType)(void);

/* Macro used to define the 7-segment table for driving the display. */

#define MAKE_7SEG(a,b,c,d,e,f,g) ((a)|((b)<<1)|((c)<<2)|((d)<<3)|((e)<<4)|((f)<<5)|((g)<<6))

/* The last four words of Flash program memory,
 * known as the Flash Configuration Words (FCW), are
 * used to store the configuration data. */
 
// PIC18F24J11 Configuration Bit Settings

// CONFIG1L
#pragma config WDTEN = OFF      // Watchdog Timer (Disabled - Controlled by SWDTEN bit)
#pragma config STVREN = OFF     // Stack Overflow/Underflow Reset (Disabled)
#pragma config XINST = OFF      // Extended Instruction Set (Disabled)

// CONFIG1H
#pragma config CP0 = OFF        // Code Protect (Program memory is not code-protected)

// CONFIG2L
#pragma config OSC = INTOSC     // Oscillator (INTOSC)
#pragma config T1DIG = ON       // T1OSCEN Enforcement (Secondary Oscillator clock source may be selected)
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator (High-power operation)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor (Enabled)
#pragma config IESO = ON        // Internal External Oscillator Switch Over Mode (Enabled)

// CONFIG2H
#pragma config WDTPS = 32768    // Watchdog Postscaler (1:32768)

// CONFIG3L
#pragma config DSWDTOSC = INTOSCREF// DSWDT Clock Select (DSWDT uses INTRC)
#pragma config RTCOSC = T1OSCREF// RTCC Clock Select (RTCC uses T1OSC/T1CKI)
#pragma config DSBOREN = ON     // Deep Sleep BOR (Enabled)
#pragma config DSWDTEN = ON     // Deep Sleep Watchdog Timer (Enabled)
#pragma config DSWDTPS = G2     // Deep Sleep Watchdog Postscaler (1:2,147,483,648 (25.7 days))

// CONFIG3H
#pragma config IOL1WAY = OFF    // IOLOCK One-Way Set Enable bit (The IOLOCK bit (PPSCON<0>) can be set once)
#pragma config MSSP7B_EN = MSK7 // MSSP address masking (7 Bit address masking mode)

// CONFIG4L
#pragma config WPFP = PAGE_15   // Write/Erase Protect Page Start/End Location (Write Protect Program Flash Page 15)
#pragma config WPEND = PAGE_WPFP// Write/Erase Protect Region Select (valid when WPDIS = 0) (Page WPFP<5:0> through Configuration Words erase/write protected)
#pragma config WPCFG = OFF      // Write/Erase Protect Configuration Region (Configuration Words page not erase/write-protected)

// CONFIG4H
#pragma config WPDIS = OFF      // Write Protect Disable bit (WPFP<5:0>/WPEND region ignored)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>

/**
* Button port definitions */

#if (APP_WATCH_TYPE_BUILD==APP_PULSAR_WRIST_WATCH_12H_NON_AUTO)

// Pulsar P2/3 with original display

    // TIME
    #define PB0_PORT_BITS   PORTAbits
    #define PB0_PIN         RA5
    //DATE
    #define PB1_PORT_BITS   PORTAbits
    #define PB1_PIN         RA1
    //HOUR
    #define PB2_PORT_BITS   PORTBbits
    #define PB2_PIN         RB0
    //MIN
    #define PB3_PORT_BITS   PORTAbits
    #define PB3_PIN         RA0

#elif (APP_WATCH_TYPE_BUILD==APP_PULSAR_WRIST_WATCH_24H_NON_AUTO)

// Pulsar P2/3 with replacement display

    // TIME
    #define PB0_PORT_BITS   PORTAbits
    #define PB0_PIN         RA5
    //DATE
    #define PB1_PORT_BITS   PORTAbits
    #define PB1_PIN         RA1
    //HOUR
    #define PB2_PORT_BITS   PORTBbits
    #define PB2_PIN         RB0
    //MIN
    #define PB3_PORT_BITS   PORTAbits
    #define PB3_PIN         RA0

#elif APP_WATCH_TYPE_BUILD==APP_TABLE_WATCH

// VFD table watch

    // TIME
    #define PB0_PORT_BITS   PORTAbits
    #define PB0_PIN         RA0
    //DATE
    #define PB1_PORT_BITS   PORTBbits
    #define PB1_PIN         RB0
    //HOUR
    #define PB2_PORT_BITS   PORTAbits
    #define PB2_PIN         RA1
    //MIN
    #define PB3_PORT_BITS   PORTAbits
    #define PB3_PIN         RA5

#elif APP_WATCH_TYPE_BUILD==APP_PROTOTYPE_BREAD_BOARD

// Prototype PCB

    // TIME
    #define PB0_PORT_BITS   PORTAbits
    #define PB0_PIN         RA0
    //DATE
    #define PB1_PORT_BITS   PORTBbits
    #define PB1_PIN         RB0
    //HOUR
    #define PB2_PORT_BITS   PORTAbits
    #define PB2_PIN         RA1
    //MIN
    #define PB3_PORT_BITS   PORTAbits
    #define PB3_PIN         RA2

#endif

// TIME
#define PB0             PB0_PORT_BITS.PB0_PIN
//DATE
#define PB1             PB1_PORT_BITS.PB1_PIN
//HOUR
#define PB2             PB2_PORT_BITS.PB2_PIN
//MIN
#define PB3             PB3_PORT_BITS.PB3_PIN

/**
* Display port definitions */

// Anodes/Segments
#define LED_AA_B        PORTCbits.RC4
#define LED_AB_TD       PORTCbits.RC5
#define LED_AC_LD       PORTCbits.RC6
#define LED_AD_C        PORTBbits.RB3
#define LED_AE          PORTCbits.RC7
#define LED_AF          PORTBbits.RB5
#define LED_AG          PORTBbits.RB2

// Cathods/Multiplexing
#define LED_1M          PORTCbits.RC3
#define LED_10M         PORTBbits.RB4
#define LED_1H          PORTBbits.RB1
#define LED_10H         PORTBbits.RB6

// Light sensor power
#define PWR_LGTH_SENSOR PORTAbits.RA6

// Optional date dot for the 24h Loki mod. Conflicts with the light sensor.
#define LED_DATE_DOT    PORTAbits.RA6

/* Debounce and hold times for the buttons. */

#define T0_DEBOUNCE     0x0400
#define T0_HOLD         0x45FF
#define T0_REPEAT_SLOW  0x3000
#define T0_REPEAT_QUICK 0x2000

/**
 * Type definitions for mapping out bits of
 * deep sleep persistent memory byte 0 and 1. */

typedef unsigned char ButtonStateType;
typedef unsigned char DisplayStateType;

/**
 * Button states */

typedef enum ButtonStateEnum
{
    PB_STATE_IDLE = 0,
    PB_STATE_DEBOUNCING = 1,
    PB_STATE_SHORT_PRESS = 2,
    PB_STATE_LONG_PRESS = 3,
    PB_STATE_RELEASED = 4

} ButtonStateEnum;

/**
 * Display states */

typedef enum DisplayStateEnum
{
    // Readout
    DISP_STATE_BLANK = 0,
    DISP_STATE_TIME = 1,
    DISP_STATE_SECONDS = 2,
    DISP_STATE_DATE = 3,
    DISP_STATE_YEAR = 4,
    DISP_STATE_WEEKDAY = 5,
    DISP_STATE_ALARM = 6,
    // Settings
    DISP_STATE_SET_HOURS = 7,
    DISP_STATE_SET_MINUTES = 8,
    DISP_STATE_SET_MONTH = 9,
    DISP_STATE_SET_DAY = 10,
    DISP_STATE_SET_YEAR = 11,
    DISP_STATE_SET_WEEKDAY = 12,
    DISP_STATE_SET_SECONDS = 13,
    // Alarm on/off
    DISP_STATE_TOGGLE_ALARM = 14,
    // Wait for starting the seconds again.
    DISP_STATE_SECONDS_STALLED = 15,
    // Debug
    DISP_STATE_LIGHT_SENSOR = 16

} DisplayStateEnum;

/**
 * Function prototypes */

void PressPB0(void);
void HoldPB0(void);
void ReleasePB0(void);

void PressPB1(void);
void HoldPB1(void);
void ReleasePB1(void);

void PressPB2(void);
void HoldPB2(void);
void ReleasePB2(void);

void PressPB3(void);
void HoldPB3(void);
void ReleasePB3(void);

/* EoF */
