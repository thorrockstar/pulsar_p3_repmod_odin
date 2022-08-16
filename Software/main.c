/**
 *  Copyright (c) 2020-22 Roy Schneider
 *
 *  main.c
 *
 *  Implementation of the Pulsar P3/P4 internal watch logic.
 *
 *  Project:            Pulsar P3 (3013) Replacement modules 'Odin' & 'Loki'.
 *                      Pulsar P4 (403) Replacement modules 'Sif' & 'Hel'.
 *                      The 'Odin' & 'Sif' modules are featuring the original
 *                      Litronix made (dotty) display, while the 'Loki' & 'Hel'
 *                      modules are featuring an USSR made ALS314A or AL304G
 *                      display.
 *                      The 'Loki' and 'Hel' modules are used in the case the
 *                      original display is corroded beyond repair.
 *
 *  Programmer:         Roy Schneider
 *  Last Change:        02.01.2022
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
 *
 * Input and Output Assignment
 *
 * Reed switches:
 * RA5 - Time readout button on the right/lower side of the watch.
 * RA1 - Date readout button on the left/upper side of the watch.
 * RB0 - Hour set button (Magnet set)
 * RA0 - Minute set button (Magnet set)
 * RC2 - Wrist flick, which can not be used together with the light sensor
 *       or the buzzer support. The wrist flick feature is only supported for
 *       watches, featuring Auto-Set and not for those that feature Magnet-Set.
 *
 * Segments:
 * There is a macro in the header file, that let you select these outputs
 * to be anodes or cathodes.
 * RC4 'A' (but 'B' for the left most 10h digit of the Litronix display)
 * RC5 'B' (but 'top dot' in the middle of the Litronix display)
 * RC6 'C' (but 'lower dot' in the middle of the Litronix display)
 * RC7 'E'
 * RB2 'G'
 * RB3 'D' (but 'C' for the left most 10h digit of the Litronix display)
 * RB5 'F'
 * RA6 for the date and alarm dots of the replacement ALS314A or AL304G display.
 *
 * Common:
 * There is a macro in the header file, that let you select these outputs
 * to be anodes or cathodes.
 * RC3 for '1' minute
 * RB4 for '10' minute
 * RB1 for '1' hour
 * RB6 for '10' hour and the dots of the 12h Litronix display
 *
 * Light sensor option
 * RA6 provides power to the light sensor, when measuring
 * RC2 used as AN11 analogue input to measure the light indirectly
 *
 * Optional buzzer option (mutual exlude with the light sensor option)
 * RA6 used for the alarm dot
 * RC2 used as PWM to drive the piezo
 * 
 * RC0 RTCC Xtal OSO
 * RC1 RTCC Xtal OSI
 */

/* Include the right header file for the PIC microcontroller used. */

#include <p18cxxx.h>

/* Include project main header file. */

#include "main.h"

/**
 * 24 hour to 12 hour system conversion for the original Litronix display.
 */

#if (APP_WATCH_TYPE_BUILD==APP_PULSAR_P3_WRIST_WATCH_12H_ODIN_MOD) || \
    (APP_WATCH_TYPE_BUILD==APP_PULSAR_P4_WRIST_WATCH_12H_SIF_MOD)

const unsigned char g_24_to_12_hours[] = { /* AM */12, 1, 2, 3, 4, 5, 6, \
                                                    7, 8, 9, 10, 11, \
                                           /* PM */12, 1, 2, 3, 4, 5, 6, \
                                                    7, 8, 9, 10, 11 };

/**
 * 24 hour to AM/PM dot system conversion for the original Litronix display.
 */

const unsigned char g_24_to_AMPM[] = { /* AM */ 1, 1, 1, 1, 1, 1, 1, \
                                                1, 1, 1, 1, 1, \
                                       /* PM */ 2, 2, 2, 2, 2, 2, 2, \
                                                2, 2, 2, 2, 2 };

#endif // Odin and Sif modules

/**
 * 7-segment numerical encoding table
 */

const unsigned char g_numerical_7segment[16] =
{
    //        a b c d e f g
    MAKE_7SEG(1,1,1,1,1,1,0),   // 0
    MAKE_7SEG(0,1,1,0,0,0,0),   // 1
    MAKE_7SEG(1,1,0,1,1,0,1),   // 2
    MAKE_7SEG(1,1,1,1,0,0,1),   // 3
    MAKE_7SEG(0,1,1,0,0,1,1),   // 4
    MAKE_7SEG(1,0,1,1,0,1,1),   // 5
    MAKE_7SEG(1,0,1,1,1,1,1),   // 6
    MAKE_7SEG(1,1,1,0,0,0,0),   // 7
    MAKE_7SEG(1,1,1,1,1,1,1),   // 8
    MAKE_7SEG(1,1,1,1,0,1,1),   // 9
    MAKE_7SEG(0,0,0,0,0,0,1),   // 10 (Minus))
    MAKE_7SEG(0,0,0,0,0,0,1),   // 11
    MAKE_7SEG(0,0,0,0,0,0,1),   // 12
    MAKE_7SEG(0,0,0,0,0,0,1),   // 13
    MAKE_7SEG(0,0,0,0,0,0,1),   // 14
    MAKE_7SEG(0,0,0,0,0,0,1)    // 15
};

/**
 * 7-segment weekday encoding table
 */

const unsigned char g_weekday_7segment[16] =
{
    //        a b c d e f g
    MAKE_7SEG(1,0,1,1,0,1,1),   // S
    MAKE_7SEG(0,1,1,1,1,1,0),   // U
    \
    MAKE_7SEG(1,1,1,0,1,1,0),   // M
    MAKE_7SEG(1,1,1,1,1,1,0),   // O
    \
    MAKE_7SEG(1,0,0,0,1,1,0),   // T
    MAKE_7SEG(0,1,1,1,1,1,0),   // U
    \
    MAKE_7SEG(0,1,1,1,1,1,0),   // W
    MAKE_7SEG(1,0,0,1,1,1,1),   // E
    \
    MAKE_7SEG(1,0,0,0,1,1,0),   // T
    MAKE_7SEG(0,1,1,0,1,1,1),   // H
    \
    MAKE_7SEG(1,0,0,0,1,1,1),   // F
    MAKE_7SEG(1,1,1,0,1,1,1),   // R
    \
    MAKE_7SEG(1,0,1,1,0,1,1),   // S
    MAKE_7SEG(1,1,1,0,1,1,1),   // A
    \
    MAKE_7SEG(0,0,0,0,0,0,0),   // [blank]
    MAKE_7SEG(0,0,0,0,0,0,1),   // -
};

/**
 * BCD to decimal decoding table.
 * 
 * This table returns 0 if a BCD counter is
 * on a not used tetrade.
 */

const unsigned char g_bcd_decimal[160] =
{
    // 0..9 [0x00..0x09 - 0x0A..0x0F]
    0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,
    // 10..19 [0x10..0x19 - 0x1A..0x1F]
    10,11,12,13,14,15,16,17,18,19,0,0,0,0,0,0,
    // 20..29 [0x20..0x29 - 0x2A..0x2F]
    20,21,22,23,24,25,26,27,28,29,0,0,0,0,0,0,
    // 30..39 [0x30..0x39 - 0x3A..0x3F]
    30,31,32,33,34,35,36,37,38,39,0,0,0,0,0,0,
    // 40..49 [0x40..0x49 - 0x4A..0x4F]
    40,41,42,43,44,45,46,47,48,49,0,0,0,0,0,0,
    // 50..59 [0x50..0x59 - 0x5A..0x5F]
    50,51,52,53,54,55,56,57,58,59,0,0,0,0,0,0,
    // 60..69 [0x60..0x69 - 0x6A..0x6F]
    60,61,62,63,64,65,66,67,68,69,0,0,0,0,0,0,
    // 70..79 [0x70..0x79 - 0x7A..0x7F]
    70,71,72,73,74,75,76,77,78,79,0,0,0,0,0,0,
    // 80..89 [0x80..0x89 - 0x8A..0x8F]
    80,81,82,83,84,85,86,87,88,89,0,0,0,0,0,0,
    // 90..99 [0x90..0x99 - 0x9A..0x9F]
    90,91,92,93,94,95,96,97,98,99,0,0,0,0,0,0,
};

/**
 * BCD to decimal decoding and testing table.
 * 
 * This table returns 127 if a BCD counter is
 * on a not used tetrade.
 */

const unsigned char g_bcd_decimal_testing[160] =
{
    // 0..9 [0x00..0x09 - 0x0A..0x0F]
    0,1,2,3,4,5,6,7,8,9,127,127,127,127,127,127,
    // 10..19 [0x10..0x19 - 0x1A..0x1F]
    10,11,12,13,14,15,16,17,18,19,127,127,127,127,127,127,
    // 20..29 [0x20..0x29 - 0x2A..0x2F]
    20,21,22,23,24,25,26,27,28,29,127,127,127,127,127,127,
    // 30..39 [0x30..0x39 - 0x3A..0x3F]
    30,31,32,33,34,35,36,37,38,39,127,127,127,127,127,127,
    // 40..49 [0x40..0x49 - 0x4A..0x4F]
    40,41,42,43,44,45,46,47,48,49,127,127,127,127,127,127,
    // 50..59 [0x50..0x59 - 0x5A..0x5F]
    50,51,52,53,54,55,56,57,58,59,127,127,127,127,127,127,
    // 60..69 [0x60..0x69 - 0x6A..0x6F]
    60,61,62,63,64,65,66,67,68,69,127,127,127,127,127,127,
    // 70..79 [0x70..0x79 - 0x7A..0x7F]
    70,71,72,73,74,75,76,77,78,79,127,127,127,127,127,127,
    // 80..89 [0x80..0x89 - 0x8A..0x8F]
    80,81,82,83,84,85,86,87,88,89,127,127,127,127,127,127,
    // 90..99 [0x90..0x99 - 0x9A..0x9F]
    90,91,92,93,94,95,96,97,98,99,127,127,127,127,127,127,
};

/**
 * Decimal to BCD encoding table.
 */

const unsigned char g_decimal_bcd[100] =
{
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,
    0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,
    0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,
    0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99
};

/**
 * Modulo 10 table for the 1x digits
 */

const unsigned char g_mod10[100] =
{
    0,1,2,3,4,5,6,7,8,9,    //  0..9
    0,1,2,3,4,5,6,7,8,9,    // 10..19
    0,1,2,3,4,5,6,7,8,9,    // 20..29
    0,1,2,3,4,5,6,7,8,9,    // 30..39
    0,1,2,3,4,5,6,7,8,9,    // 40..49
    0,1,2,3,4,5,6,7,8,9,    // 50..59
    0,1,2,3,4,5,6,7,8,9,    // 60..69
    0,1,2,3,4,5,6,7,8,9,    // 70..79
    0,1,2,3,4,5,6,7,8,9,    // 80..89
    0,1,2,3,4,5,6,7,8,9,    // 90..99
};

/**
 * Div 10 table for the 10x digits
 */

const unsigned char g_div10[100] =
{
    0,0,0,0,0,0,0,0,0,0,    //  0..9
    1,1,1,1,1,1,1,1,1,1,    // 10..19
    2,2,2,2,2,2,2,2,2,2,    // 20..29
    3,3,3,3,3,3,3,3,3,3,    // 30..39
    4,4,4,4,4,4,4,4,4,4,    // 40..49
    5,5,5,5,5,5,5,5,5,5,    // 50..59
    6,6,6,6,6,6,6,6,6,6,    // 60..69
    7,7,7,7,7,7,7,7,7,7,    // 70..79
    8,8,8,8,8,8,8,8,8,8,    // 80..89
    9,9,9,9,9,9,9,9,9,9,    // 90..99
};

/**
 * Table for animating the dot, when
 * the alarm buzzer has been turned on.
 */

#if APP_ALARM_SPECIAL_DOT_ANIMATION==1

const unsigned char g_dot_banner[60] =
{
    4,4,4,4,4,4,4,4,4,4,
    1,1,1,1,1,1,1,1,1,1,
    8,8,8,8,8,8,8,8,8,8,
    2,2,2,2,2,2,2,2,2,2,
    8,8,8,8,8,8,8,8,8,8,
    1,1,1,1,1,1,1,1,1,1,
};

unsigned char g_dot_banner_index = 0;

#endif // #if APP_ALARM_SPECIAL_DOT_ANIMATION==1

/**
 * Global indication for the watch to stay awake. */

unsigned char g_ucStayAwake;

/**
 * Global counter for the timer used to keep the display lit. */

unsigned short g_ucRollOver;

/**
 * Global button state variables. */

ButtonStateType g_ucPB0TIMEState = PB_STATE_IDLE; // TIME
ButtonStateType g_ucPB1DATEState = PB_STATE_IDLE; // DATE
ButtonStateType g_ucPB2HOURState = PB_STATE_IDLE; // HOUR
ButtonStateType g_ucPB3MINTState = PB_STATE_IDLE; // MIN

/**
 * Global button debounce timer value variables.
 * As all buttons share the same timer for debouncing,
 * we have to keep track of the timer value for each button. */

short g_sPB0Timer = 0;
short g_sPB1Timer = 0;
short g_sPB2Timer = 0;
short g_sPB3Timer = 0;

/**
 Wrist Flick support. */

#if APP_WRIST_FLICK_USAGE==1

ButtonStateType g_ucPB4FLICKState = PB_STATE_IDLE; // WRIST FLICK

short g_sPB4Timer = 0;
short g_FlickTimeSpan = 0;
unsigned char g_WristFlick = 0;

#endif // #if APP_WRIST_FLICK_USAGE==1

/**
 * Global variable indicating which buttons
 * are using the timer 0 for debouncing. */

unsigned char g_ucTimer0Usage = 0;

/**
 * Global variable indicating if timer 2
 * is in use for showing the display. */

unsigned char g_ucTimer2Usage = 0;

/**
 * Multiplexer variable for the digits,
 * reflecting which digit is currently powered. */

unsigned char g_ucMplexDigits = 0;

/**
 * Brightness variable for the digits, reflecting
 * the readout from the AN11 analogue input. */

#if APP_LIGHT_SENSOR_USAGE==1

unsigned char  g_ucDimmingCnt = 0;
unsigned char  g_ucDimmingRef = 0;
unsigned char  g_ucDimming = 0;
unsigned short g_ucLightSensor = 0;

#endif // #if APP_LIGHT_SENSOR_USAGE==1

/**
 * Current shown values for the left two and right two
 * digits, usually the hours and minutes or month and day. */

unsigned char g_ucLeftVal = 255;
unsigned char g_ucRightVal = 255;

/**
 * Alarm piezo base period value. */

const unsigned char g_ualarm_period = 20 + 64;
const unsigned char g_ualarm_duty_cycle = 10 + 32;

/**
 * Table to the 7-segment digits, can be set to numerical or character table. */

const unsigned char *g_pDigits;

/**
 * Variable, reflecting the state of the Dot-LED's on the display,
 * in between month and day or hour and minute. */

#if APP_WATCH_TYPE_BUILD!=APP_PROTOTYPE_BREAD_BOARD

unsigned char g_ucDots = 0;

#endif

/**
 * If using the Pulsar Autoset button mode, there
 * are two button press counters for the TIME and DATE
 * buttons, that are reset, when the display is turned off.
 */

#if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET

unsigned char g_ucTimePressCnt = 0;
unsigned char g_ucDatePressCnt = 0;

#endif // #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET

/**
 * In order not to change the display mode while multiplexing
 * store a copy of the display state, when the multiplexing
 * cycle had started. */

DisplayStateType g_uDispState;
DisplayStateType g_uDispStateBackup;

/**
 * Alarm counter, cancelled by expiring or pressing a button. */

#if APP_BUZZER_ALARM_USAGE==1

unsigned short g_ucAlarm;

#endif

/**
 * Unlock the RTC. This is time critical, so we use
 * assembly language here to get it right. */

inline void Unlock_RTCC(void)
{
    /* Write Magic */

    EECON2 = 0x55;
    EECON2 = 0xAA;

    /* Set write enable bit. */

    RTCCFGbits.RTCWREN = 1; // RTCC Value Registers Write Enable bit
}

/**
 * Lock the RTC and protect it from further write attempts. */

inline void Lock_RTCC(void)
{
    /* Ensure the RTC is enabled. */

    RTCCFGbits.RTCEN = 1;

    /* Clear write enable bit. */

    RTCCFGbits.RTCWREN = 0; // RTCC Value Registers Write Enable bit
}

/**
 * Initialize the ports of the general purpose inputs and outputs. */

inline void Init_Inputs_Outputs_Ports(void)
{
    /* Initialize LATA/B/C to clear output data latches. */

    LATA = 0;
    LATB = 0;
    LATC = 0;

    /* Free AN5 from being tight to the Voltage monitoring. */

    HLVDCON = 8;

    /* Turn off SSDMA (SPI DMA Module) */

    DMACON1 = 0;
}

/**
 * Setting up the general purpose inputs and outputs
 * as well as the ananlogue input for the light sensor
 * or the PWM output for the alarm feature, if used. */

inline void Configure_Inputs_Outputs(void)
{
    /* Disable the unused Charge Time Measurement Unit (CTMU)
     * used for touch screens and capacitive sensors. */

    CTMUCONH = 0x00; // Make sure CTMU is disabled.
    CTMUCONL = 0x90; // Edge 1/2 programmed for a positive edge response
    CTMUICON = 0x01; // 0.55uA, Nominal - No Adjustment

  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

    /* Set RA1/5 to input, RA0/2/3/4/6/7 as output. */

    TRISA = 0x22;

    /* Set RB1/4/6 to input, keep RB0/2/3/5/7 as output. */

    TRISB = 0x52;
    
  #else

    /* Set RA0/1/2/5 to input, RA3/4/6/7 as output. */

    TRISA = 0x27;

    /* Set RB0/1/4/6 to input, keep RB2/3/5/7 as output. */

    TRISB = 0x53;
    
  #endif

#if APP_BUZZER_ALARM_USAGE==1

    /* Set RC0/1/2/4/5..7 to output and RC3 as input. */

    TRISC = 0x08;

#else

    /* Set RC0/1/4/5..7 to output and RC3 and RC2(AN11) as input. */

    TRISC = 0x0C;

#endif

    /* Set unused port pins to drive low. */

  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

    PORTAbits.RA0 = 0;
    PORTBbits.RB0 = 0;
    
  #endif

    PORTAbits.RA3 = 0;
    PORTAbits.RA7 = 0;

#if APP_LIGHT_SENSOR_USAGE==1

    /* ANCON0 - A/D PORT CONFIGURATION REGISTER
     * Analog Port Configuration bits (AN<7:0>)
     * 1 = Pin configured as a digital port
     * 0 = Pin configured as an analog channel - digital input disabled
     * and reads zero */

    ANCON0 = 0xFF;          // AN0...7

    /* ANCON1 - A/D PORT CONFIGURATION REGISTER
     * Analog Port Configuration bits (AN<7:0>)
     * 1 = Pin configured as a digital port
     * 0 = Pin configured as an analog channel - digital input disabled
     * and reads zero */

    ANCON1 = 0x17;          // AN8..12 Activate AN11 as being an analogue input.

    // ADCON1
    ADCON1bits.ADFM = 1;    // Result format 1= Right justified
    ADCON1bits.ADCAL = 0;   // Normal A/D conversion operation
    ADCON1bits.ACQT = 1;    // Acquisition time 7 = 20TAD 2 = 4TAD 1=2TAD
    ADCON1bits.ADCS = 2;    // Clock conversion bits 6= FOSC/64 2=FOSC/32
    
    // ANCON1
    ANCON1bits.VBGEN = 1;   // Turn on the Bandgap

    // ADCON0
    ADCON0bits.VCFG0 = 0;   // Vref+ = AVdd 3V battery
    ADCON0bits.VCFG1 = 0;   // Vref- = AVss 0V battery
    ADCON0bits.CHS = 11;    // Select ADC channel -> AN11
    ADCON0bits.ADON = 1;    // Turn on ADC

#else // Light sensor not used. Anyway free the analogue inputs for digital use.

    /* ANCON0 - A/D PORT CONFIGURATION REGISTER
     * Analog Port Configuration bits (AN<7:0>)
     * 1 = Pin configured as a digital port
     * 0 = Pin configured as an analog channel - digital input disabled
     * and reads zero */

    ANCON0 = 0xFF;          // AN0...7

    /* ANCON1 - A/D PORT CONFIGURATION REGISTER
     * Analog Port Configuration bits (AN<7:0>)
     * 1 = Pin configured as a digital port
     * 0 = Pin configured as an analog channel - digital input disabled
     * and reads zero */

    ANCON1 = 0x1F;          // AN8..12

    /* ADCON0 */

    ADCON0bits.ADON = 0;    // Turn off ADC

#endif

  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

    /* Set RA1/5 to input, RA0/2/3/4/6/7 as output. */

    TRISA = 0x22;
    
  #else

    /* Set RA0/1/2/5 to input, RA3/4/6/7 as output. */

    TRISA = 0x27;
    
  #endif

    /* Configure the output as PWM output channel A. */

#if APP_BUZZER_ALARM_USAGE==1

    /* Write Magic */

    EECON2 = 0x55;
    EECON2 = 0xAA;

    /* Set write enable bit for the I/O mapping. */

    IOLOCK = 0;

    /* Map RP13 to CCP1/P1A - PWM Channel A */

    RPOR13 = 14;

    /* Clear write enable bit for the I/O mapping. */

    IOLOCK = 1;

#endif
}

/**
 * Initialize the button states. */

inline void Init_Button_States(void)
{
    g_ucPB0TIMEState = PB_STATE_IDLE; // TIME
    g_ucPB1DATEState = PB_STATE_IDLE; // DATE
    g_ucPB2HOURState = PB_STATE_IDLE; // HOUR
    g_ucPB3MINTState = PB_STATE_IDLE; // MIN

    g_sPB0Timer = 0;
    g_sPB1Timer = 0;
    g_sPB2Timer = 0;
    g_sPB3Timer = 0;
    
  #if APP_WRIST_FLICK_USAGE==1

    g_ucPB4FLICKState = PB_STATE_IDLE; // WRIST FLICK
    g_sPB4Timer = 0;

  #endif // #if APP_WRIST_FLICK_USAGE==1
}

/**
 * Setting up the onboard RTC. */

inline void Configure_Real_Time_Clock(void)
{
    unsigned char ucRTCInval;
    unsigned char ucTemp;
    unsigned char ucValue;

    while(RTCCFGbits.RTCSYNC);

    /* Unlock write operations to the RTC. */

    Unlock_RTCC();

    /* Set write enable bit. */

    RTCCFGbits.RTCWREN = 1; // RTCC Value Registers Write Enable bit

    /* Disable the RTCC */

    RTCCFGbits.RTCEN = 0;   // RTCC module is disabled

    /* Poll RTCSYNC until it has cleared.
     *
     * The RTCSYNC bit indicates a time window during
     * which the RTCC Clock Domain registers can be safely
     * read and written without concern about a rollover.
     * When RTCSYNC = 0, the registers can be safely accessed. */

    while(RTCCFGbits.RTCSYNC);

    /* Check if the RTC has invalid values. */

    ucRTCInval = 0;

    RTCCFG |= 3;        // Highest RTC address

    ucTemp = RTCVALL; // Year
    ucValue = g_bcd_decimal_testing[ucTemp];
    if (ucValue > 99)
    {
        ucRTCInval = 1;
    }

    ucTemp = RTCVALH; // reserved, dummy for decrement

    ucTemp = RTCVALL; // Day
    ucValue = g_bcd_decimal_testing[ucTemp];
    if ((ucValue < 1) || (ucValue > 31))
    {
        ucRTCInval = 1;
    }

    ucTemp = RTCVALH; // Month
    ucValue = g_bcd_decimal_testing[ucTemp];
    if ((ucValue < 1) || (ucValue > 12))
    {
        ucRTCInval = 1;
    }

    ucTemp = RTCVALL; // Hour
    ucValue = g_bcd_decimal_testing[ucTemp];
    if (ucValue > 23)
    {
        ucRTCInval = 1;
    }

    ucTemp = RTCVALH; // Weekday
    ucValue = g_bcd_decimal_testing[ucTemp];
    if (ucValue > 6)
    {
        ucRTCInval = 1;
    }

    ucTemp = RTCVALL; // Second
    ucValue = g_bcd_decimal_testing[ucTemp];
    if (ucValue > 59)
    {
        ucRTCInval = 1;
    }

    ucTemp = RTCVALH; // Minute
    ucValue = g_bcd_decimal_testing[ucTemp];
    if (ucValue > 59)
    {
        ucRTCInval = 1;
    }

    /* Reset RTC if invalid. */

    if (ucRTCInval)
    {
        while(RTCCFGbits.RTCSYNC);

        RTCCFG |= 3;

        RTCVALL = g_decimal_bcd[22]; // Year 2022

        ucRTCInval = RTCVALH; // Dummy for decrement

        RTCVALL = 1; // Day
        RTCVALH = 1; // Month

#if (APP_WATCH_TYPE_BUILD==APP_PULSAR_P3_WRIST_WATCH_12H_ODIN_MOD) || \
    (APP_WATCH_TYPE_BUILD==APP_PULSAR_P4_WRIST_WATCH_12H_SIF_MOD)
        
        RTCVALL = 0; // Hour (midnight))

#else

        RTCVALL = g_decimal_bcd[12]; // Hour (noon)
        
#endif

        RTCVALH = 3; // Weekday (1.1.2020 was a Wednesday)

        RTCVALL = 0; // Seconds
        RTCVALH = 0; // Minutes
        
        /* Initialize all alarm registers. */

        ALRMCFGbits.ALRMPTR0 = 0;
        ALRMCFGbits.ALRMPTR1 = 1;

        ALRMVALL = 1; // Day
        ALRMVALH = 1; // Month

        ALRMVALL = 0; // Hour
        ALRMVALH = 1; // Weekday

        ALRMVALL = 0; // Seconds
        ALRMVALH = 0; // Minutes
    }
    else // RTC time/date was valid
    {
        /* Check alarm time only, if clock time/date was valid. */
        
        ucRTCInval = 0;

        ALRMCFGbits.ALRMPTR0 = 1;
        ALRMCFGbits.ALRMPTR1 = 0;
        
        ucTemp = ALRMVALL; // Hour
        ucValue = g_bcd_decimal_testing[ucTemp];
        if (ucValue > 23)
        {
            ucRTCInval = 1;
        }

        ucTemp = ALRMVALH; // Dummy to decrement address.

        ucTemp = ALRMVALH; // Minute
        ucValue = g_bcd_decimal_testing[ucTemp];
        if (ucValue > 59)
        {
            ucRTCInval = 1;
        }

        /* Initialize alarm registers. */

        ALRMCFGbits.ALRMPTR0 = 0;
        ALRMCFGbits.ALRMPTR1 = 1;

        if (ucRTCInval)
        {
            /* Initialize all alarm registers. */

            ALRMVALL = 1; // Day
            ALRMVALH = 1; // Month

            ALRMVALL = 0; // Hour
            ALRMVALH = 1; // Weekday

            ALRMVALL = 0; // Seconds
            ALRMVALH = 0; // Minutes
        }
        else // RTC alarm time was valid
        {
            /* Initialize only not used alarm registers. */
            
            ALRMVALL = 1; // Day
            ALRMVALH = 1; // Month, auto-decrement!
            ALRMVALH = 1; // Weekday, auto-decrement!
            ALRMVALL = 0; // Seconds
        }
    }
    
    while(RTCCFGbits.RTCSYNC);

    /* Enable setting pointers to read MIN and SEC. */

    RTCCFGbits.RTCPTR0 = 0; // Points to the RTCC Value registers when reading.
    RTCCFGbits.RTCPTR1 = 0;

    /* Set optional RTC alarm to 1 Hz, but keep it off. */

    ALRMCFGbits.CHIME = 0;  /* If chime is enabled, ensure not to clear ALRMEN
                             * bit. ALRMRPT<7:0> bits are allowed to roll over
                             * from 00h to FFh */

    /* Configure an optional alarm every second. */

    ALRMCFGbits.AMASK = 0x06; // Alarm repeats every day.

    /* Disable the alarm for now. */

    ALRMCFGbits.ALRMEN = 0;

    /* Set Alarm repeat to zero. */

    ALRMRPT = 0;

    /* Disable the Alarm interrupt by default. */

    PIE3bits.RTCCIE = 0;

    /* Enable the RTCC */

    RTCCFGbits.RTCEN = 1;   // RTCC module is enabled

    while(RTCCFGbits.RTCSYNC);

    /* Lock writing to the RTCC. */

    Lock_RTCC();
}

/**
 * Configure the timer 0, featuring the internal oscillator as source
 * and 8-bit counting mode. The prescaler is set to 1:16.
 * Timer 0 is used to debounce the buttons. */

inline void Configure_Timer_0(void)
{
    /* Set the clock source to the internal oscillator. */

    T0CONbits.T0CS = 0;    // Clock source internal
    T0CONbits.T08BIT = 0;  // Keep default 16 bit mode

    /* Set the prescaler to 1:64.
     *
     * 1MHz * prescaler = 64 us -> 0xFFFF -> 4 s */

    T0CONbits.T0PS = 0x05;
    T0CONbits.PSA = 0;

    /* Set the timer to zero. */

    TMR0H = 0;
    TMR0L = 0;

    /* Disable the timer interrupt. */

    INTCONbits.TMR0IE = 0;  // Disables the TMR0 overflow interrupt
    INTCONbits.TMR0IF = 0;  // Reset state bit TMR0 register did not overflow

    /* Turn the timer 0 off. */

    T0CONbits.TMR0ON = 0;

    /* Timer usage */

    g_ucTimer0Usage = 0;
}

/**
 * Configure the timer 1, featuring the external 32.768 crystal as source
 * and 16-bit counting mode. No prescaler is used.
 * This timer is not used yet. */

inline void Configure_Timer_1(void)
{
    /* Set the clock source to the external oscillator. */

    T1CONbits.TMR1CS = 2; // Timer1 clock source is T1OSC or T1CKI pin

    /* Set the prescaler to 0.
     *
     * (1/32.768KHz) * prescaler = 30.5 us -> 0xFFFF -> 2 s */

    T1CONbits.T1CKPS = 0;

    /* Enable T1 external oscillator. */

    T1CONbits.T1OSCEN = 1;  // Timer1 oscillator circuit enabled

    /* Set timer to 16 bit width. */

    T1CONbits.RD16 = 1;

    /* Use the TIMER1 GATE CONTROL REGISTER
     * to drop the Timer1 Gate Enable bit to ensure
     * that Timer1 counts regardless of Timer1 gate
     * function. */

    T1GCONbits.TMR1GE = 0;

     /* Clear the Overflow Interrupt Flag bit
      * and disable the Overflow Interrupt Enable bit. */

    PIR1bits.TMR1IF = 0;
    PIE1bits.TMR1IE = 0;

    /* Turn the timer 1 off. */

    T1CONbits.TMR1ON = 0;
}

/**
 * Configure the timer 2, featuring the internal oscillator as source
 * The prescaler is set to 1:16. This is an 8-bit counter.
 * This timer is used to define the time the display is on for readout. */

inline void Configure_Timer_2(void)
{
    /* Set the clock prescaler to 1:16. */

    T2CONbits.T2CKPS = 2; // Prescaler is 16

    /* Set the timer to zero. */

    TMR2 = 0;

    /* Turn timer 2 off. */

    T2CONbits.TMR2ON = 0;

    /* Timer usage */

    g_ucTimer2Usage = 0;
}

/**
 * Configure the timer 3, featuring the internal oscillator as source
 * and 16-bit counting mode. The prescaler is set to maximum.
 * This timer is not used yet. */

inline void Configure_Timer_3(void)
{
    /* Set the clock source to the internal oscillator. */

    T3CONbits.TMR3CS = 0;

    /* Set the prescaler to 1:8. */

    T3CONbits.T3CKPS = 3;

    /* Set the timer to 16-bit mode. */

    T3CONbits.RD16 = 1;

    /* Set the timer to zero. */

    TMR3H = 0;
    TMR3L = 0;

    /* Turn timer 3 off. */

    T3CONbits.TMR3ON = 0;
}

/**
 * Configure the timer 4.
 * This timer is used for the buzzer, driven via PWM. */

inline void Configure_Timer_4(void)
{
    /* Set the clock prescaler to 1:16. */

    T4CONbits.T4CKPS = 2; // Prescaler is 16

    /* Set the timer to zero. */

    TMR4 = 0;

    /* Turn timer 4 off. */

    T4CONbits.TMR4ON = 0;

    /* ECCP1 and ECCP2 both use Timer3 (capture/compare) and Timer4 (PWM) */

  #if APP_BUZZER_ALARM_USAGE==1

    TCLKCONbits.T3CCP1 = 0;
    TCLKCONbits.T3CCP2 = 1;

  #endif
}

/**
 * Enter deep sleep mode. This function will not return as the controller
 * will have entered sleep mode, before returning.
 *
 * Once the system wakes up, it will restart the program an the main entry point.
 */

#if APP_WATCH_TYPE_BUILD!=APP_TABLE_WATCH

inline void enterSleep(void)
{
  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

    /* Set RA1/5 to input, RA0/2/3/4/6/7 as output. */

    TRISA = 0x22;
    
  #else

    /* Set RA0/1/2/5 to input, RA3/4/6/7 as output. */

    TRISA = 0x27;
    
  #endif

  #if APP_WATCH_TYPE_BUILD==APP_PROTOTYPE_BREAD_BOARD

    /* Map INT1 to RP0/RA0 (TIME - BREADBOARD) input for wake-up. */

    RPINR1 = 0; // RP0/RA0

  #else // #if APP_WATCH_TYPE_BUILD==APP_PROTOTYPE_BREAD_BOARD

  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

   #if APP_WRIST_FLICK_USAGE==1

    /* Map INT1 to RP13/RC2 (WRIST FLICK) input for wake-up. */

    RPINR1 = 13; // RP13/RC2

   #else // #if APP_WRIST_FLICK_USAGE==1

    /* Map INT1 to RP0/RA0 input for wake-up. */

    RPINR1 = 0; // RP0/RA0 (unused)

   #endif // #else #if APP_WRIST_FLICK_USAGE==1

  #else // #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

    /* Map INT1 to RP0/RA0 (MIN) input for wake-up. */

    RPINR1 = 0; // RP0/RA0
    
  #endif // #else #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

    /* Map INT2 to RP0/RA1 (DATE) input for wake-up. */

    RPINR2 = 1; // RP1/RA1

    /* Map INT3 to RP2/RA5 (TIME) input for wake-up. */

    RPINR3 = 2; // RP2/RA5

  #endif // #else #if APP_WATCH_TYPE_BUILD==APP_PROTOTYPE_BREAD_BOARD

    /* Disable Ultra Low-Power Wake-up (ULPWU) usage. */

    WDTCONbits.REGSLP  = 1; /* On-chip regulator enters low-power operation
                             * when device enters Sleep mode. */
    DSCONHbits.RTCWDIS = 0; // Wake-up from RTCC is enabled.
    DSCONHbits.DSULPEN = 0; // ULPWU module is disabled in Deep Sleep
    DSCONLbits.ULPWDIS = 1; // ULPWU wake-up source is disabled.

    /* Applications needing to save a small amount of data throughout a
     * Deep Sleep cycle can save the data to the general purpose DSGPR0
     * and DSGPR1 registers. */

    //DSGPR0
    //DSGPR1

    /* Wake up on a rising edge of the HOUR button. */

    INTCON2bits.INTEDG0 = 1;    // External Interrupt 0 Edge Select bit

    /* Wake up on a rising edge of the DATE button. */

    INTCON2bits.INTEDG1 = 1;    // External Interrupt 1 Edge Select bit

    /* Wake up on a rising edge of the MIN button or WRIST FLICK button. */

    INTCON2bits.INTEDG2 = 1;    // External Interrupt 2 Edge Select bit

    /* Wake up on a rising edge of the TIME. */

    INTCON2bits.INTEDG3 = 1;    // External Interrupt 3 Edge Select bit

    /* Clear all interuppts. */

    // HOURS
    INTCONbits.INT0IF = 0;   // Clear INT0 Flag
  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET
    INTCONbits.INT0IE = 0;   // Disable INT0
  #else
    INTCONbits.INT0IE = 1;   // Enable INT0
  #endif // #if APP_WATCH_ANY_PULSAR_MODEL!=APP_WATCH_PULSAR_AUTO_SET

    // MIN / WRIST FLICK
    INTCON3bits.INT1IF = 0;  // Clear INT1 Flag
  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET
   #if APP_WRIST_FLICK_USAGE==1
    INTCON3bits.INT1IE = 1;  // Enable INT1
   #else
    INTCON3bits.INT1IE = 0;  // Disable INT1
   #endif
  #else
    INTCON3bits.INT1IE = 1;  // Enable INT1
  #endif

    // DATE
    INTCON3bits.INT2IF = 0;  // Clear INT2 Flag
    INTCON3bits.INT2IE = 1;  // Enable INT2

    // TIME
    INTCON3bits.INT3IF = 0;  // Clear INT3 Flag
    INTCON3bits.INT3IE = 1;  // Enable INT3

#if APP_BUZZER_ALARM_USAGE==1

    /* PERIPHERAL INTERRUPT REQUEST (FLAG) REGISTER 3 */

    PIR3bits.RTCCIF = 0;    // No RTCC interrupt occurred.

    /* Enable the Alarm interrupt, if the alarm had been enabled. */

    PIE3bits.RTCCIE = ALRMCFGbits.ALRMEN ? 1 : 0;

#endif // #if APP_BUZZER_ALARM_USAGE==1

    /* INTERRUPT CONTROL REGISTER */

    // Global Interrupt Enable bit
    INTCONbits.GIE = 1;      // Enables all unmasked interrupts
    // Peripheral Interrupt Enable bit
    INTCONbits.PEIE = 1;     // Enables all unmasked peripheral interrupts

    /* Double check the TIME & DATE buttons to be low. Otherwise we
     * may fail to detect a rising edge. */

#if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

    if ((!PB0 /*TIME*/) && (!PB1 /*DATE*/))

#else

    if ((!PB0 /*TIME*/) && (!PB1 /*DATE*/) && (!PB2 /*HOUR*/) && (!PB3 /*MIN*/))

#endif        

    {
        /* Sleep vs. deep sleep mode. */

        OSCCONbits.IDLEN = 0;     // Disable deep sleep.
        DSCONHbits.DSEN = 0;      // Must be set to 1 DS just before Sleep();

        Sleep();
    }
}

#endif

/**
 * This function will turn the alarm buzzer on again.
 *
 * @param duration  Duration in ticks.
 */

#if APP_BUZZER_ALARM_USAGE==1

void Turn_Buzzer_On(unsigned short duration)
{
    /* Alarm counter used to keep the buzzer on. */

    g_ucAlarm = duration;

    /* Set the display state to time reading. */

    if (g_uDispState == DISP_STATE_BLANK)
    {
        g_uDispState = DISP_STATE_TIME;
    }

    /* Turn the 'stay awake' timer on, if activating the buzzer. */

    g_ucTimer2Usage = 1;    // Indicate using the timer.
    TMR2 = 0;               // Zero the timer.
    T2CONbits.TMR2ON = 1;   // Turn timer 2 on.

    /* Single output: PxA, PxB, PxC and PxD controlled by steering. */

    CCP1CONbits.P1M1 = 0;
    CCP1CONbits.P1M0 = 0;

    /* Assuming 40Mhz/4 as FOSC makes 0,0000001s as TOSC.
     * Having a timer 4 pre-scaler of 16 and using this equation
     * pwm period = (PR4+1)*4*TOSC*(TMR4 Prescaler) and turning
     * that around to PR4 = ((1/1700)/4/16/0,0000001)-1 */

    PR4bits.PR4 = g_ualarm_period; // We are using timer 4 for the PWM!

    /* Set duty cycle.
     * Assuming 40Mhz/4 as FOSC makes 0,0000001s as TOSC.
     * Having a timer 4 pre-scaler of 16 and using this equation
     * duty cycle = (CCPRXL:CCPXCON<5:4>))*TOSC*(TMR4 Prescaler)
     * and turning that around to DC = (1/1700/2)/16/0,0000001 */

    CCP1CONbits.DC1B = 0;    // Lower 2 bit
    CCPR1Lbits.CCPR1L = g_ualarm_duty_cycle;  // Upper 8 Bit

    /* Turn timer 4 as PWM source. */

    T4CONbits.TMR4ON = 1;

    /* Activate PWM mode PxA and PxC active-high; PxB and PxD active-high */

    CCP1CONbits.CCP1M = 0xC;
}

/**
 * This function will turn the alarm buzzer off again.
 */

inline void Turn_Buzzer_Off(void)
{
    /* Zero the Alarm counter used to keep the buzzer on. */

    g_ucAlarm = 0;

    /* Deactivate PWM mode PxA and PxC active-high; PxB and PxD active-high */

    CCP1CONbits.CCP1M = 0;

    /* Turn timer 4 off again. */

    T4CONbits.TMR4ON = 0;
}

/**
 * This function will turn the alarm buzzer off again.
 *
 * @param ufancy  Alter the sound (0-off, 1..3 frequency variant)
 */

inline void Turn_Buzzer_Fancy(unsigned char ufancy)
{
    /* Deactivate PWM mode PxA and PxC active-high; PxB and PxD active-high */

    CCP1CONbits.CCP1M = 0;

    /* Turn timer 4 as PWM source off. */

    T4CONbits.TMR4ON = 0;

    /* Assuming 40Mhz/4 as FOSC makes 0,0000001s as TOSC.
     * Having a timer 4 pre-scaler of 16 and using this equation
     * pwm period = (PR4+1)*4*TOSC*(TMR4 Prescaler) and turning
     * that around to PR4 = ((1/1000)/4/16/0,0000001)-1 */

    PR4bits.PR4 = (unsigned char) (g_ualarm_period - (ufancy << 3)); // We are using timer 4 for the PWM!

    /* Set duty cycle.
     * Assuming 40Mhz/4 as FOSC makes 0,0000001s as TOSC.
     * Having a timer 4 pre-scaler of 16 and using this equation
     * duty cycle = (CCPRXL:CCPXCON<5:4>))*TOSC*(TMR4 Prescaler)
     * and turning that around to DC = (1/1000/2)/16/0,0000001 */

    CCPR1Lbits.CCPR1L = (unsigned char) (g_ualarm_duty_cycle - (ufancy << 2));  // Upper 8 Bit

    /* Turn timer 4 as PWM source. */

    T4CONbits.TMR4ON = 1;

    /* Activate PWM mode PxA and PxC active-high; PxB and PxD active-high */

    CCP1CONbits.CCP1M = 0xC;
}

#endif

/**
 * This function will read and debounce the push buttons.
 *
 * @return  Return zero, if the watch can enter sleep, non-zero otherwise.
 */

unsigned char DebounceButtons(void)
{
    short *ptimer;
    unsigned char *pstate;
    unsigned char ubutton;
    short itimer;

    /* Button handler pointer. */

    ButtonHandlerType ppressed;
    ButtonHandlerType phold;
    ButtonHandlerType preleased;

    /* Check for a wrist flick event, if that feature has been enabled. */

#if APP_WRIST_FLICK_USAGE==1
    
    static const unsigned char imaxb = 5; // Buttons and wrist flick.

#else
    
    static const unsigned char imaxb = 4; // Buttons only.
    
#endif // #if APP_WRIST_FLICK_USAGE==1

    /* Init */

    unsigned char ibtns = 0;
    unsigned char istayawake = 0;
    unsigned char *pusage = &g_ucTimer0Usage;

    do // while(++ibtns < 4);
    {
        /* Read the button status and state machine value. */

        switch(ibtns)
        {
            // PB0 - TIME
            case DEBOUNCE_INDEX_BUTTON_TIME: // 0
                pstate = &g_ucPB0TIMEState;
                ptimer = &g_sPB0Timer;
                ubutton = PB0;
                ppressed = &PressPB0;
                phold = &HoldPB0;
                preleased = &ReleasePB0;
            break;

            // PB1 - DATE
            case DEBOUNCE_INDEX_BUTTON_DATE: // 1
                pstate = &g_ucPB1DATEState;
                ptimer = &g_sPB1Timer;
                ubutton = PB1;
                ppressed = &PressPB1;
                phold = &HoldPB1;
                preleased = &ReleasePB1;
            break;

#if APP_WATCH_ANY_PULSAR_MODEL!=APP_WATCH_PULSAR_AUTO_SET

            // PB2 - HOUR
            case DEBOUNCE_INDEX_BUTTON_HOUR: // 2
                pstate = &g_ucPB2HOURState;
                ptimer = &g_sPB2Timer;
                ubutton = PB2;
                ppressed = &PressPB2;
                phold = &HoldPB2;
                preleased = &ReleasePB2;
            break;

            // PB3 - MIN
            case DEBOUNCE_INDEX_BUTTON_MIN: // 3
                pstate = &g_ucPB3MINTState;
                ptimer = &g_sPB3Timer;
                ubutton = PB3;
                ppressed = &PressPB3;
                phold = &HoldPB3;
                preleased = &ReleasePB3;
            break;

#endif // #if APP_WATCH_ANY_PULSAR_MODEL!=APP_WATCH_PULSAR_AUTO_SET

#if APP_WRIST_FLICK_USAGE==1
            
            // PB4 - WRIST FLICK
            case DEBOUNCE_INDEX_BUTTON_FLICK: // 4
                pstate = &g_ucPB4FLICKState;
                ptimer = &g_sPB4Timer;
                ubutton = PB4;
                ppressed = &PressPB4;
                phold = NULL;
                preleased = &ReleasePB4;
            break;
            
#endif // #if APP_WRIST_FLICK_USAGE==1

            default:
                pstate = NULL;
            break;
        }

        if (pstate)
        {
            /* Check if the button has been pressed. */

            if (ubutton) // pressed
            {
                switch(*pstate)
                {
                    case PB_STATE_IDLE:

                    /* Ignore the Wrist Flick if the display
                     * is not in blank mode anymore or any
                     * other event is keeping the watch awake. */

                  #if APP_WRIST_FLICK_USAGE==1

                    if (ibtns == DEBOUNCE_INDEX_BUTTON_FLICK)
                    {
                        /* Already in a display mode, ignore the wrist flick. */
                        
                        if (g_uDispState)
                        {
                            break;
                        }
                        
                        /* The watch is already awake, so it was not the wrist
                         * flick waking it up, ignore the wrist flick. */
                        
                        if (g_ucStayAwake)
                        {
                            break;
                        }
                    }

                  #endif // #if APP_WRIST_FLICK_USAGE==1

                    /* If the button has been pressed, start debouncing it. */

                    *pstate = PB_STATE_DEBOUNCING;

                    /* Start the timer, if not started yet by another
                     * button before. */

                    if (!(*pusage))
                    {
                        /* Zero timer */

                        TMR0H = 0;
                        TMR0L = 0;

                        /* Turn timer 0 on. */

                        T0CONbits.TMR0ON = 1;

                        /* Set the start timer value for this button. */

                        *ptimer = 0;
                    }
                    else // Timer already in use and running.
                    {
                        /* Read out the start timer value for this button,
                         * if the timer is already running, triggered by
                         * another button already using it.
                         *
                         * TMR0H is not the actual high byte of Timer0 in 16-bit
                         * mode. It is actually a buffered version of the real high
                         * byte of Timer0, which is not directly readable nor
                         * writable. TMR0H is updated with the contents of the high
                         * byte of Timer0 during a read of TMR0L.
                         * This provides the ability to read all 16 bits of
                         * Timer0 without having to verify that the read of the high
                         * and low byte were valid, due to a rollover between
                         * successive reads of the high and low byte. */

                        /* First read the low byte of the timer, which will buffer
                         * the high byte. */

                        const unsigned char ulow = TMR0L;

                        /* Read now the buffered high byte of the timer, that
                         * had been stored, when the low byte had been read. */

                        const unsigned char uhigh = TMR0H;

                        *ptimer = ulow | (uhigh << 8);
                    }

                    /* Indicate that this button is using the timer. */

                    *pusage |= 1 << ibtns;

                    /* Return none-zero to indicate not to enter
                     * deep sleep mode. */

                    istayawake = 1;

                    /* Continue checking the next button. */

                    break;

                    case PB_STATE_DEBOUNCING:
                    case PB_STATE_SHORT_PRESS:

                    /* If the button is still pressed, read
                     * out the current timer value. */

                    {
                        /* TMR0H is not the actual high byte of Timer0 in 16-bit
                         * mode. It is actually a buffered version of the real high
                         * byte of Timer0, which is not directly readable nor
                         * writable. TMR0H is updated with the contents of the high
                         * byte of Timer0 during a read of TMR0L.
                         * This provides the ability to read all 16 bits of
                         * Timer0 without having to verify that the read of the high
                         * and low byte were valid, due to a rollover between
                         * successive reads of the high and low byte. */

                        /* First read the low byte of the timer, which will buffer
                         * the high byte. */

                        const unsigned char ulow = TMR0L;

                        /* Read now the buffered high byte of the timer, that
                         * had been stored, when the low byte had been read. */

                        const unsigned char uhigh = TMR0H;

                        itimer = ulow | (uhigh << 8);
                    }

                    /* Check if the long (hold) debounce timer has been expired.
                     * Use signed values to take mathimatical a rollover in account.
                     * This will work as long as the time span is lower than the
                     * half of the timer's range. */

                    if ((itimer - (*ptimer + T0_HOLD)) >= 0)
                    {
                        /* Check if the button is using a 'hold' handler. */
                        
                        if (phold)
                        {
                            /* Set the button to 'hold' state. */

                            *pstate = PB_STATE_LONG_PRESS;

                            /* Store timer value as new start point. */

                            *ptimer = itimer;

                            /* Call the hold handler for this button. */

                            (*phold)();

                            /* Turn the 'stay awake' timer on. */

                            g_ucTimer2Usage = 1;    // Indicate using the timer.
                            TMR2 = 0;               // Zero the timer.
                            T2CONbits.TMR2ON = 1;   // Turn timer 2 on.
                        }
                        else // if (phold)
                        {
                            /* If the button is not featuring a 'hold' handler drop the state. */
                            
                            *pstate = PB_STATE_IDLE;

                            /* Indicate that this button is not using the timer anymore. */

                            *pusage &= ~(1 << ibtns);

                            /* If this was the last button using the timer, stop the timer. */

                            if (!(*pusage))
                            {
                                /* Stop the debouncing timer used. */

                                T0CONbits.TMR0ON = 0;
                            }
                            else
                            {
                                /* As long as another button is still
                                 * using the timer, do not enter
                                 * deep sleep mode. */

                                istayawake = 1;
                            }
                        }
                    }
                    else if ((itimer - (*ptimer + T0_DEBOUNCE)) >= 0)
                    {
                        /* If the short debounce timer has been expired. */

                        if (*pstate != PB_STATE_SHORT_PRESS)
                        {
                            /* Set the button to 'pressed' state. */

                            *pstate = PB_STATE_SHORT_PRESS;

                            /* Call the 'press' handler. */

                            if (ppressed)
                            {
                                (*ppressed)();
                            }

                            /* Trigger 'stay awake' timer. */

                            g_ucTimer2Usage = 1;    // Indicate using the timer.
                            TMR2 = 0;               // Zero the timer.
                            T2CONbits.TMR2ON = 1;   // Turn timer 2 on.

                            /* Keep the timer going as we have to detect
                             * the 'hold' state as well.*/
                        }
                    }

                    /* Do not enter deep sleep mode. */

                    istayawake = 1;

                    /* Continue checking the next button. */

                    break;

                    case PB_STATE_LONG_PRESS:

                    /* If the button is still pressed, read
                     * out the current timer value. */

                    {
                        /* TMR0H is not the actual high byte of Timer0 in 16-bit
                         * mode. It is actually a buffered version of the real high
                         * byte of Timer0, which is not directly readable nor
                         * writable. TMR0H is updated with the contents of the high
                         * byte of Timer0 during a read of TMR0L.
                         * This provides the ability to read all 16 bits of
                         * Timer0 without having to verify that the read of the high
                         * and low byte were valid, due to a rollover between
                         * successive reads of the high and low byte. */

                        /* First read the low byte of the timer, which will buffer
                         * the high byte. */

                        const unsigned char ulow = TMR0L;

                        /* Read now the buffered high byte of the timer, that
                         * had been stored, when the low byte had been read. */

                        const unsigned char uhigh = TMR0H;

                        itimer = ulow | (uhigh << 8);
                    }

                    /* Check if the long (hold) debounce timer has been expired.
                     * Use signed values to take mathimatical a rollover in account.
                     * This will work as long as the time span is lower than the
                     * half of the timer's range. */

                    short ltime = T0_REPEAT_SLOW;

                    switch(ibtns)
                    {
                        case 0: // TIME
                     #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET
                            if (g_uDispState >= DISP_STATE_AUTOSET_TIME)
                            {
                                ltime = T0_REPEAT_QUICK;
                            }
                     #endif
                        break;

                        case 1: // DATE
                     #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET
                            if (g_uDispState >= DISP_STATE_AUTOSET_TIME)
                            {
                                ltime = T0_REPEAT_QUICK;
                            }
                            else
                            {
                                ltime = T0_HOLD;
                            }
                     #else
                            ltime = T0_HOLD;
                     #endif
                        break;

                        case 2: // HOUR
                        case 3: // MINUTE
                            ltime = T0_REPEAT_QUICK;
                        break;

                        default:
                        break;
                    }

                    /* Check if the long-press time has expired.
                     * If yes, recharge the timer again. */
                    
                    if ((itimer - (*ptimer + (ltime))) >= 0)
                    {
                        *ptimer = itimer;

                        /* Call the hold handler for this button. */

                        if (phold)
                        {
                            (*phold)();
                        }
                    }

                    /* As long as another button is still
                     * using the timer, do not enter
                     * deep sleep mode. */

                    istayawake = 1;

                    /* Continue checking the next button. */

                    break;

                    case PB_STATE_RELEASED:

                    /* If the button was held and then was dropped
                     * but actually was turned on again within the
                     * debounce time for dropping, go back to the
                     * held state again. */

                    *pstate = PB_STATE_LONG_PRESS;

                    /* As long as another button is still
                     * pressed, do not enter
                     * deep sleep mode. */

                    istayawake = 1;

                    /* Continue checking the next button. */

                    break;

                    default:
                    break;
                }
            }
            else // if (ubutton)
            {
                /* If the button has been released or is not pressed at all
                 * return zero to indicate, that it would be safe to enter deep
                 * sleep again. */

                switch(*pstate)
                {
                    /* The button has not been pressed and the state machine
                     * is idle. */

                    case PB_STATE_IDLE:

                    /* Continue checking the next button. */

                    break;

                    /* If the button was on debouncing for a peak up
                     * and the signal peaked down again, dismiss the
                     * debouncing process. */

                    case PB_STATE_DEBOUNCING:

                    *pstate = PB_STATE_IDLE;

                    /* Indicate that this button is not using the timer anymore. */

                    *pusage &= ~(1 << ibtns);

                    /* If this was the last button using the timer, stop the timer. */

                    if (!(*pusage))
                    {
                        /* Stop the debouncing timer used. */

                        T0CONbits.TMR0ON = 0;
                    }
                    else
                    {
                        /* As long as another button is still
                         * using the timer, do not enter
                         * deep sleep mode. */

                        istayawake = 1;
                    }

                    /* Continue checking the next button. */

                    break;

                    case PB_STATE_SHORT_PRESS:

                    *pstate = PB_STATE_IDLE;

                    /* Indicate that this button is not using the timer anymore. */

                    *pusage &= ~(1 << ibtns);

                    /* If this was the last button using the timer,
                     * stop the timer. */

                    if (!(*pusage))
                    {
                        /* Stop the debouncing timer used. */

                        T0CONbits.TMR0ON = 0;
                    }
                    else
                    {
                        /* As long as another button is still
                         * using the timer, do not enter
                         * deep sleep mode. */

                        istayawake = 1;
                    }

                    /* Call the hold handler for this button. */

                    if (preleased)
                    {
                        (*preleased)();
                    }

                    /* Continue checking the next button. */

                    break;

                    /* If the button is not pressed anymore, but was hold
                     * down before for a while, debounce the dropping as well. */

                    case PB_STATE_LONG_PRESS:

                    *pstate = PB_STATE_RELEASED;

                    /* Start the timer, if not started yet by any other button. */

                    if (!(*pusage))
                    {
                        /* Zero timer */

                        TMR0H = 0;
                        TMR0L = 0;

                        /* Turn tmer 0 on. */

                        T0CONbits.TMR0ON = 1;

                        /* Set the start timer value for this button. */

                        *ptimer = 0;
                    }
                    else
                    {
                        /* Read out the start timer value for this button,
                         * if the timer is already running, triggered by
                         * another button already using it.
                         *
                         * TMR0H is not the actual high byte of Timer0 in 16-bit
                         * mode. It is actually a buffered version of the real high
                         * byte of Timer0, which is not directly readable nor
                         * writable. TMR0H is updated with the contents of the high
                         * byte of Timer0 during a read of TMR0L.
                         * This provides the ability to read all 16 bits of
                         * Timer0 without having to verify that the read of the high
                         * and low byte were valid, due to a rollover between
                         * successive reads of the high and low byte. */

                        /* First read the low byte of the timer, which will buffer
                         * the high byte. */

                        const unsigned char ulow = TMR0L;

                        /* Read now the buffered high byte of the timer, that
                         * had been stored, when the low byte had been read. */

                        const unsigned char uhigh = TMR0H;

                        *ptimer = ulow | (uhigh << 8);
                    }

                    /* Indicate that this button is using the timer. */

                    *pusage |= 1 << ibtns;

                    /* Return none-zero to indicate not to enter
                     * deep sleep mode. */

                    istayawake = 1;

                    /* Continue checking the next button. */

                    break;

                    /* If the button has been dropped and was hold, before,
                     * debounce the dropping of the button as well. */

                    case PB_STATE_RELEASED:

                    /* Read out the timer value. */

                    {
                        /* TMR0H is not the actual high byte of Timer0 in 16-bit
                         * mode. It is actually a buffered version of the real high
                         * byte of Timer0, which is not directly readable nor
                         * writable. TMR0H is updated with the contents of the high
                         * byte of Timer0 during a read of TMR0L.
                         * This provides the ability to read all 16 bits of
                         * Timer0 without having to verify that the read of the high
                         * and low byte were valid, due to a rollover between
                         * successive reads of the high and low byte. */

                        /* First read the low byte of the timer, which will buffer
                         * the high byte. */

                        const unsigned char ulow = TMR0L;

                        /* Read now the buffered high byte of the timer, that
                         * had been stored, when the low byte had been read. */

                        const unsigned char uhigh = TMR0H;

                        itimer = ulow | (uhigh << 8);
                    }

                    /* Check if the long (hold) debounce timer for dropping
                     * the button has been expired.
                     * Use signed values to take mathimatical a rollover in account.
                     * This will work as long as the time span is lower than the
                     * half of the timer's range. */

                    if ((itimer - (*ptimer + T0_DEBOUNCE)) >= 0)
                    {
                        /* Indicate that this button is using the timer
                         * not anymore. */

                        *pusage &= ~(1 << ibtns);

                        /* Check if any other button is not using the
                         * timer anymore. Then it is safe to turn it off. */

                        if (!(*pusage))
                        {
                            /* Stop the debouncing timer used. */

                            T0CONbits.TMR0ON = 0;
                        }
                        else
                        {
                            /* As long as another button is still
                             * using the timer, do not enter
                             * deep sleep mode. */

                            istayawake = 1;
                        }

                        /* Set the state machine for the button back to 'idle. */

                        *pstate = PB_STATE_IDLE;

                        /* Call the hold handler for this button. */

                        if (preleased)
                        {
                            (*preleased)();
                        }

                        /* Continue checking the next button. */
                    }
                    else // if ((itimer - (*ptimer + T0_DEBOUNCE)) >= 0)
                    {
                        /* As long as another button is still
                         * using the timer, do not enter
                         * deep sleep mode. */

                        istayawake = 1;
                    }
                    break;

                    default:
                    break;
                }
            }
        } // if (pstate)
    }
    while(++ibtns < imaxb);

    /* Return none-zero to indicate not to enter
     * deep sleep mode. */

    return (istayawake);
}

/**
 * Called when button 0 TIME has been pressed.
 */

void PressPB0(void)
{
  #if APP_BUZZER_ALARM_USAGE==1

    /* Check if the alarm buzzer is still active. */

    if (g_ucAlarm)
    {
        Turn_Buzzer_Off();
    }

  #endif

    const DisplayStateType istate = g_uDispState;

    /* If using the Pulsar Autoset button mode, there
     * are two button press counters for the TIME and DATE
     * buttons, that are reset, when the display is turned off. */

  #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET

    if (istate == DISP_STATE_AUTOSET_DATE)
    {
        /* Restart the time, the display will stay lit up. */

        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        unsigned char ucExtra;
        unsigned char ucTemp;
        unsigned char ucValue;

     #if (APP_WATCH_TYPE_BUILD==APP_PULSAR_P3_WRIST_WATCH_12H_ODIN_MOD) || \
         (APP_WATCH_TYPE_BUILD==APP_PULSAR_P4_WRIST_WATCH_12H_SIF_MOD)

        ucExtra = 1;

        /* On every second cycle change betwen AM and PM and otherwise
         * forward the day. */

        RTCCFGbits.RTCPTR0 = 1;
        RTCCFGbits.RTCPTR1 = 0;

        ucTemp = RTCVALL;
        ucValue = g_bcd_decimal[ucTemp];

        if (ucValue < 12)
        {
            /* AM -> PM */

            ucValue += 12;

            ucTemp = g_decimal_bcd[ucValue];
            RTCVALL = ucTemp;

            /* Do not forward the day. */

            ucExtra = 0;
        }
        else
        {
            /* PM -> AM */

            ucValue -= 12;

            ucTemp = g_decimal_bcd[ucValue];
            RTCVALL = ucTemp;
        }

        if (ucExtra)

      #endif

        {
            /* Forward the day of month. */

            RTCCFGbits.RTCPTR0 = 0;
            RTCCFGbits.RTCPTR1 = 1;
            //while(RTCCFGbits.RTCSYNC);

            ucTemp = RTCVALL; // day
            ucValue = 1 + g_bcd_decimal[ucTemp];

            ucTemp = RTCVALH; // month
            ucExtra = g_bcd_decimal[ucTemp];

            if (ucExtra == 2) // February, always assume it to be a leap year.
            {
                /* Assume february to have 29 days to support leap years. */

                if (ucValue >= 30) // 29 days (leap year)
                {
                    ucValue = 1;
                }
            }
            else if ((ucExtra == 1) || (ucExtra == 3) || (ucExtra == 5) || \
                     (ucExtra == 7) || (ucExtra == 8) || (ucExtra == 10) || \
                     (ucExtra == 12))
            {
                if (ucValue >= 32) // 31 days
                {
                    ucValue = 1;
                }
            }
            else
            {
                if (ucValue >= 31) // 30 days
                {
                    ucValue = 1;
                }
            }

            RTCCFGbits.RTCPTR0 = 0;
            RTCCFGbits.RTCPTR1 = 1;

            ucTemp = g_decimal_bcd[ucValue];
            RTCVALL = ucTemp;
        }

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (istate == DISP_STATE_AUTOSET_TIME)
    {
        /* Restart the time, the display will stay lit up. */
        
        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enabling writing to the RTC. */

        RTCCFGbits.RTCWREN = 1;

        /* Stop the RTC */

        RTCCFGbits.RTCEN = 0;

        /* Set the RTC register to read/write. */

        RTCCFG &= ~3;

        /* Forward the minutes. */

        unsigned char ucTemp = RTCVALH;
        unsigned char ucValue = 1 + g_bcd_decimal[ucTemp];

        if (ucValue >= 60)
        {
            ucValue = 0;
        }

        /* Set the RTC register to read/write. */

        ucTemp = g_decimal_bcd[ucValue];
        RTCVALH = ucTemp;

        /* Set seconds to zero. */

        RTCVALL = 0; // Zero the second when forwarding the minute.

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
        
        /* Indicate to zero the seconds and stall the watch. */
        
        g_ucTimePressCnt = HINT_AUTOSET_MINUTES_SET;
    }
    else if (istate == DISP_STATE_AUTOSET_WEEKDAY)
    {
        /* Restart the time, the display will stay lit up. */
        
        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enabling writing to the RTC. */

        RTCCFGbits.RTCWREN = 1;

        /* Stop the RTC */

        RTCCFGbits.RTCEN = 0;

        /* Set the RTC register to read/write. */

        RTCCFGbits.RTCPTR0 = 1;
        RTCCFGbits.RTCPTR1 = 0;
        //while(RTCCFGbits.RTCSYNC);

        /* Backward the weekday. */

        unsigned char ucTemp = RTCVALH;
        unsigned char ucValue = (ucTemp & 7);

        if (!ucValue)
        {
            ucValue = 6;
        }
        else
        {
            ucValue--;
        }

        /* As accessing the hi-byte will automatically decrement
         * the address pointer, we have to set the address again, before
         * writing the weekday. */

        RTCCFGbits.RTCPTR0 = 1;

        RTCVALH = ucValue;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (istate == DISP_STATE_AUTOSET_YEAR)
    {
        /* Restart the time, the display will stay lit up. */
        
        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        /* Set the RTC register to read/write. */

        RTCCFG |= 3;
        //while(RTCCFGbits.RTCSYNC);

        /* Backward the year. */

        unsigned char ucTemp = RTCVALL;
        unsigned char ucValue = g_bcd_decimal[ucTemp];

        if (!ucValue)
        {
            ucValue = 99;
        }
        else
        {
            ucValue--;
        }

        ucTemp = g_decimal_bcd[ucValue];
        RTCVALL = ucTemp;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (istate == DISP_STATE_AUTOSET_CALIBRA)
    {
        /* Restart the time, the display will stay lit up. */
        
        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        /* Backward the calibration register and turn around on max. */

        unsigned char ucTemp = RTCCAL;
        
        /* Check if passing the maximum absolute value. */
        
        if (ucTemp & 0x80)
        {
            /* Negative calibration for clocks running too fast. */
            
            if (ucTemp <= 0x81)
            {
                /* Turn around to zero value. */

                ucTemp = 0x7E;
            }
            else
            {
                ucTemp -= 2;
            }
        }
        else // if (ucTemp & 0x80)
        {
            /* Positive calibration for clocks running too slow. */

            if (ucTemp <= 0x01)
            {
                /* Turn around to maximum negative value. */

                ucTemp = 0xFE;
            }
            else
            {
                ucTemp -= 2;
            }
        }
        
        RTCCAL = ucTemp;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else // No autoset mode in charge.
    {
        g_ucDatePressCnt = 0;

        if (g_ucTimePressCnt < 3)
        {
            if (++g_ucTimePressCnt == 3)
            {
                if (istate == DISP_STATE_SET_CALIBRA)
                {
                    g_uDispState = DISP_STATE_AUTOSET_CALIBRA;
                }
                else
                {
                    g_uDispState = DISP_STATE_AUTOSET_TIME;
                }
            }
        }
    }

  #endif // #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET

    /* When having set the time, the RTC will be disabled until
     * you press the very first time the 'TIME' button.
     * This will make the watch start counting time at zero seconds. */

    if (istate == DISP_STATE_SECONDS_STALLED)
    {
        g_uDispState = DISP_STATE_TIME;

        /* Unlock via magic. */

        Unlock_RTCC();

        /* Enable write to RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Enable RTC operation. */

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }

    /* Revoke the 'blanked' state in order to turn the display on. */

    if (istate == DISP_STATE_BLANK)
    {
        g_uDispState = DISP_STATE_TIME;
    }
    else
    {
        /* Pressing the TIME button will usually show the time.
         * But reagrding how Pulsar P2/P3 and early P4 works,
         * the time button can be pressed together with the DATE button,
         * when setting the day of month. */

      #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_MAGNET_SET

        if ((istate == DISP_STATE_DATE) || (istate == DISP_STATE_SET_MONTH))
        {
            g_uDispState = DISP_STATE_SET_DAY;
        }
        else if (istate == DISP_STATE_SET_WEEKDAY)
        {
            g_uDispState = DISP_STATE_SET_YEAR                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   ;
        }

      #endif
    }
}

/**
 * Called when button 0 TIME has been held pressed.
 */

void HoldPB0(void)
{
    DisplayStateType istate = g_uDispState;

    /* If using the Pulsar Autoset button mode, there
     * are two button press counters for the TIME and DATE
     * buttons, that are reset, when the display is turned off. */

  #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET

    if (istate == DISP_STATE_AUTOSET_DATE)
    {
        /* Restart the time, the display will stay lit up. */
        
        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        unsigned char ucExtra;
        unsigned char ucTemp;
        unsigned char ucValue;

     #if (APP_WATCH_TYPE_BUILD==APP_PULSAR_P3_WRIST_WATCH_12H_ODIN_MOD) || \
         (APP_WATCH_TYPE_BUILD==APP_PULSAR_P4_WRIST_WATCH_12H_SIF_MOD)

        ucExtra = 1;

        /* On every second cycle change betwen AM and PM and otherwise
         * forward the day. */

        RTCCFGbits.RTCPTR0 = 1;
        RTCCFGbits.RTCPTR1 = 0;

        ucTemp = RTCVALL;
        ucValue = g_bcd_decimal[ucTemp];

        if (ucValue < 12)
        {
            /* AM -> PM */

            ucValue += 12;

            ucTemp = g_decimal_bcd[ucValue];
            RTCVALL = ucTemp;

            /* Do not forward the day. */

            ucExtra = 0;
        }
        else
        {
            /* PM -> AM */

            ucValue -= 12;

            ucTemp = g_decimal_bcd[ucValue];
            RTCVALL = ucTemp;
        }

        if (ucExtra)

      #endif

        {
            /* Forward the day of month. */

            RTCCFGbits.RTCPTR0 = 0;
            RTCCFGbits.RTCPTR1 = 1;
            //while(RTCCFGbits.RTCSYNC);

            ucTemp = RTCVALL; // day
            ucValue = 1 + g_bcd_decimal[ucTemp];

            ucTemp = RTCVALH; // month
            ucExtra = g_bcd_decimal[ucTemp];

            if (ucExtra == 2) // February, always assume it to be a leap year.
            {
                /* Assume february to have 29 days to support leap years. */

                if (ucValue >= 30) // 29 days (leap year)
                {
                    ucValue = 1;
                }
            }
            else if ((ucExtra == 1) || (ucExtra == 3) || (ucExtra == 5) || \
                     (ucExtra == 7) || (ucExtra == 8) || (ucExtra == 10) || \
                     (ucExtra == 12))
            {
                if (ucValue >= 32) // 31 days
                {
                    ucValue = 1;
                }
            }
            else
            {
                if (ucValue >= 31) // 30 days
                {
                    ucValue = 1;
                }
            }

            RTCCFGbits.RTCPTR0 = 0;
            RTCCFGbits.RTCPTR1 = 1;

            ucTemp = g_decimal_bcd[ucValue];
            RTCVALL = ucTemp;
        }

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (istate == DISP_STATE_AUTOSET_TIME)
    {
        /* Restart the time, the display will stay lit up. */
        
        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enabling writing to the RTC. */

        RTCCFGbits.RTCWREN = 1;

        /* Stop the RTC */

        RTCCFGbits.RTCEN = 0;

        /* Set the RTC register to read/write. */

        RTCCFG &= ~3;

        /* Forward the minutes. */

        unsigned char ucTemp = RTCVALH;
        unsigned char ucValue = 1 + g_bcd_decimal[ucTemp];

        if (ucValue >= 60)
        {
            ucValue = 0;
        }

        /* Set the RTC register to read/write. */

        ucTemp = g_decimal_bcd[ucValue];
        RTCVALH = ucTemp;

        /* Set seconds to zero. */

        RTCVALL = 0; // Zero the second when forwarding the minute.

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();

        /* Indicate to zero the seconds and stall the watch. */
        
        g_ucTimePressCnt = HINT_AUTOSET_MINUTES_SET;
    }
    else if (istate == DISP_STATE_AUTOSET_WEEKDAY)
    {
        /* Restart the time, the display will stay lit up. */
        
        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enabling writing to the RTC. */

        RTCCFGbits.RTCWREN = 1;

        /* Stop the RTC */

        RTCCFGbits.RTCEN = 0;

        /* Set the RTC register to read/write. */

        RTCCFGbits.RTCPTR0 = 1;
        RTCCFGbits.RTCPTR1 = 0;
        //while(RTCCFGbits.RTCSYNC);

        /* Backward the weekday. */

        unsigned char ucTemp = RTCVALH;
        unsigned char ucValue = (ucTemp & 7);

        if (!ucValue)
        {
            ucValue = 6;
        }
        else
        {
            ucValue--;
        }

        /* As accessing the hi-byte will automatically decrement
         * the address pointer, we have to set the address again, before
         * writing the weekday. */

        RTCCFGbits.RTCPTR0 = 1;

        RTCVALH = ucValue;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (istate == DISP_STATE_AUTOSET_YEAR)
    {
        /* Restart the time, the display will stay lit up. */
        
        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        /* Set the RTC register to read/write. */

        RTCCFG |= 3;
        //while(RTCCFGbits.RTCSYNC);

        /* Backward the year. */

        unsigned char ucTemp = RTCVALL;
        unsigned char ucValue = g_bcd_decimal[ucTemp];

        if (!ucValue)
        {
            ucValue = 99;
        }
        else
        {
            ucValue--;
        }

        ucTemp = g_decimal_bcd[ucValue];
        RTCVALL = ucTemp;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (istate == DISP_STATE_AUTOSET_CALIBRA)
    {
        /* Restart the time, the display will stay lit up. */
        
        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        /* Forward the calibration register and turn around on max. */

        unsigned char ucTemp = RTCCAL;
        
        /* Check if passing the maximum absolute value. */
        
        if (ucTemp & 0x80)
        {
            /* Negative calibration for clocks running too fast. */
            
            if (ucTemp <= 0x81)
            {
                /* Turn around to zero value. */

                ucTemp = 0x7E;
            }
            else
            {
                ucTemp -= 2;
            }
        }
        else // if (ucTemp & 0x80)
        {
            /* Positive calibration for clocks running too slow. */

            if (ucTemp <= 0x01)
            {
                /* Turn around to maximum negative value. */

                ucTemp = 0xFE;
            }
            else
            {
                ucTemp -= 2;
            }
        }
        
        RTCCAL = ucTemp;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }

  #endif // #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET

    /* Holding the TIME button pressed will usually show the seconds.
     * But reagrding how Pulsar P2/P3 and eraly P4 works, the
     * time button can be hold pressed after the DATE button,
     * when setting the day of month. */

    if ((g_ucPB2HOURState == PB_STATE_IDLE) && \
        (g_ucPB3MINTState == PB_STATE_IDLE))
    {
        if (istate == DISP_STATE_TIME)
        {
            g_uDispState = DISP_STATE_SECONDS;
        }
        else if ((istate == DISP_STATE_YEAR) || \
                 (istate == DISP_STATE_LIGHT_SENSOR))
        {
            g_uDispState = DISP_STATE_SET_CALIBRA;
        }
    }
}

/**
 * Called when button 0 TIME has been released.
 */

void ReleasePB0(void)
{
    const DisplayStateType istate = g_uDispState;

  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_MAGNET_SET

    if (istate == DISP_STATE_SET_DAY)
    {
        g_uDispState = DISP_STATE_SET_MONTH;
    }
    else if (istate == DISP_STATE_SET_YEAR)
    {
        g_uDispState = DISP_STATE_SET_WEEKDAY;
    }
    else

  #endif

    {
        if (istate == DISP_STATE_SECONDS)
        {
            /* Turn timer 2 off. */

            T2CONbits.TMR2ON = 0;

            /* Timer usage */

            g_ucTimer2Usage = 0;
        }
    }
}

/**
 * Called when button 1 DATE has been pressed.
 */

void PressPB1(void)
{
  #if APP_BUZZER_ALARM_USAGE==1

    /* Check if the alarm buzzer is still active. */

    if (g_ucAlarm)
    {
        Turn_Buzzer_Off();
    }

  #endif

    DisplayStateType *pb = &g_uDispState;

    /* If using the Pulsar Autoset button mode, there
     * are two button press counters for the TIME and DATE
     * buttons, that are reset, when the display is turned off. */

  #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET

    if (*pb == DISP_STATE_AUTOSET_DATE)
    {
        /* Restart the time, the display will stay lit up. */
        
        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        /* Forward the month. */

        RTCCFGbits.RTCPTR0 = 0;
        RTCCFGbits.RTCPTR1 = 1;
        //while(RTCCFGbits.RTCSYNC);

        unsigned char ucTemp = RTCVALH;
        unsigned char ucValue = 1 + g_bcd_decimal[ucTemp];

        if (ucValue >= 13)
        {
            ucValue = 1;
        }

        RTCCFGbits.RTCPTR0 = 0;
        RTCCFGbits.RTCPTR1 = 1;

        ucTemp = g_decimal_bcd[ucValue];
        RTCVALH = ucTemp;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (*pb == DISP_STATE_AUTOSET_TIME)
    {
        /* Restart the time, the display will stay lit up. */
        
        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        /* Forward the hour. */

        RTCCFGbits.RTCPTR0 = 1;
        RTCCFGbits.RTCPTR1 = 0;
        //while(RTCCFGbits.RTCSYNC);

        unsigned char ucTemp = RTCVALL;
        unsigned char ucValue = 1 + g_bcd_decimal[ucTemp];

        if (ucValue >= 24)
        {
            ucValue = 0;
        }

        ucTemp = g_decimal_bcd[ucValue];
        RTCVALL = ucTemp;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (*pb == DISP_STATE_AUTOSET_WEEKDAY)
    {
        /* Restart the time, the display will stay lit up. */
        
        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enabling writing to the RTC. */

        RTCCFGbits.RTCWREN = 1;

        /* Stop the RTC */

        RTCCFGbits.RTCEN = 0;

        /* Set the RTC register to read/write. */

        RTCCFGbits.RTCPTR0 = 1;
        RTCCFGbits.RTCPTR1 = 0;
        //while(RTCCFGbits.RTCSYNC);

        /* Forward the weekday. */

        unsigned char ucTemp = RTCVALH;
        unsigned char ucValue = 1 + (ucTemp & 7);

        if (ucValue >= 7)
        {
            ucValue = 0;
        }

        /* As accessing the hi-byte will automatically decrement
         * the address pointer, we have to set the address again, before
         * writing the weekday. */

        RTCCFGbits.RTCPTR0 = 1;

        RTCVALH = ucValue;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (*pb == DISP_STATE_AUTOSET_YEAR)
    {
        /* Restart the time, the display will stay lit up. */
        
        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        /* Set the RTC register to read/write. */

        RTCCFG |= 3;
        //while(RTCCFGbits.RTCSYNC);

        /* Forward the year. */

        unsigned char ucTemp = RTCVALL;
        unsigned char ucValue = 1 + g_bcd_decimal[ucTemp];

        if (ucValue >= 100)
        {
            ucValue = 0;
        }

        ucTemp = g_decimal_bcd[ucValue];
        RTCVALL = ucTemp;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (*pb == DISP_STATE_AUTOSET_CALIBRA)
    {
        /* Restart the time, the display will stay lit up. */
        
        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        /* Forward the calibration register and turn around on max. */

        unsigned char ucTemp = RTCCAL;
        
        /* Check if passing the maximum absolute value. */
        
        if (ucTemp & 0x80)
        {
            /* Negative calibration for clocks running too fast. */
            
            if (ucTemp >= 0xFE)
            {
                /* Turn around to zero value. */

                ucTemp = 0;
            }
            else
            {
                ucTemp += 2;
            }
        }
        else // if (ucTemp & 0x80)
        {
            /* Positive calibration for clocks running too slow. */

            if (ucTemp >= 0x7E)
            {
                /* Turn around to maximum negative value. */

                ucTemp = 0x82;
            }
            else
            {
                ucTemp += 2;
            }
        }
        
        RTCCAL = ucTemp;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else
    {
        g_ucTimePressCnt = 0;

        if (g_ucDatePressCnt < 3)
        {
            if (++g_ucDatePressCnt == 3)
            {
                if (*pb == DISP_STATE_WEEKDAY)
                {
                    *pb = DISP_STATE_AUTOSET_WEEKDAY;
                }
                else if (*pb == DISP_STATE_YEAR)
                {
                    *pb = DISP_STATE_AUTOSET_YEAR;
                }
                else
                {
                    *pb = DISP_STATE_AUTOSET_DATE;
                }
            }
        }
    }

  #endif // #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET

    /* Revoke the 'blanked' state in order to turn the display on. */

    if (*pb == DISP_STATE_BLANK)
    {
        *pb = DISP_STATE_DATE;
    }

  #if (APP_WATCH_TYPE_BUILD==APP_PULSAR_P3_WRIST_WATCH_12H_ODIN_MOD) || \
      (APP_WATCH_TYPE_BUILD==APP_PULSAR_P4_WRIST_WATCH_12H_SIF_MOD)

    else if (*pb == DISP_STATE_TIME)
    {
        *pb = DISP_STATE_DATE;
    }

  #else

   #if APP_BUZZER_ALARM_USAGE==1

    else if ((*pb == DISP_STATE_TIME) || \
             (*pb == DISP_STATE_SECONDS))
    {
        /* Show Alarm Time */

        *pb = DISP_STATE_ALARM;
    }
    else if (*pb == DISP_STATE_TOGGLE_ALARM)
    {
        if (((g_ucPB0TIMEState == PB_STATE_SHORT_PRESS) || \
             (g_ucPB0TIMEState == PB_STATE_LONG_PRESS)) \
             \
             && \
             \
            (g_ucPB2HOURState == PB_STATE_IDLE) && \
            (g_ucPB3MINTState == PB_STATE_IDLE))
        {
            /* Unlock write access to the RTC and disable the clock. */

            Unlock_RTCC();

            /* Enable writing to the RTC */

            RTCCFGbits.RTCWREN = 1;

            /* Stop RTC operation. */

            RTCCFGbits.RTCEN = 0;

            /* Toggle the alarm bit. */

            ALRMCFGbits.AMASK = 0x06; // Alarm repeats every day.

            int ival = ALRMCFGbits.ALRMEN ? 0 : 1;

            ALRMCFGbits.CHIME = ival ? 1 : 0;  // Chime enable;

            /* Set Alarm repeat */

            ALRMRPT = ival ? 255 : 0;

            /* Enable the Alarm interrupt, if the alarm had been enabled. */

            PIE3bits.RTCCIE = ival ? 1 : 0;

            /* PERIPHERAL INTERRUPT REQUEST (FLAG) REGISTER 3 */

            PIR3bits.RTCCIF = 0;    // No RTCC interrupt occurred.

            /* Enable/diable alarm */
            
            ALRMCFGbits.ALRMEN = ival ? 1 : 0;  // Alarm enable

            /* Enable the RTC operation again.*/

            RTCCFGbits.RTCEN = 1;

            /* Lock writing to the RTCC. */

            Lock_RTCC();
        }
    }
   #else

    else if (*pb == DISP_STATE_TIME)
    {
        *pb = DISP_STATE_DATE;
    }

   #endif

  #endif
}

/**
 * Called when button 1 DATE has been held pressed.
 */

void HoldPB1(void)
{
    DisplayStateType istate = g_uDispState;
    
    /* If using the Pulsar Autoset button mode, there
     * are two button press counters for the TIME and DATE
     * buttons, that are reset, when the display is turned off. */

  #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET

    if (istate == DISP_STATE_AUTOSET_DATE)
    {
        /* Restart the time, the display will stay lit up. */
        
        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        /* Forward the month. */

        RTCCFGbits.RTCPTR0 = 0;
        RTCCFGbits.RTCPTR1 = 1;
        //while(RTCCFGbits.RTCSYNC);

        unsigned char ucTemp = RTCVALH;
        unsigned char ucValue = 1 + g_bcd_decimal[ucTemp];

        if (ucValue >= 13)
        {
            ucValue = 1;
        }

        RTCCFGbits.RTCPTR0 = 0;
        RTCCFGbits.RTCPTR1 = 1;

        ucTemp = g_decimal_bcd[ucValue];
        RTCVALH = ucTemp;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (istate == DISP_STATE_AUTOSET_TIME)
    {
        /* Restart the time, the display will stay lit up. */
        
        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        /* Forward the hour. */

        RTCCFGbits.RTCPTR0 = 1;
        RTCCFGbits.RTCPTR1 = 0;
        //while(RTCCFGbits.RTCSYNC);

        unsigned char ucTemp = RTCVALL;
        unsigned char ucValue = 1 + g_bcd_decimal[ucTemp];

        if (ucValue >= 24)
        {
            ucValue = 0;
        }

        ucTemp = g_decimal_bcd[ucValue];
        RTCVALL = ucTemp;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (istate == DISP_STATE_AUTOSET_WEEKDAY)
    {
        /* Restart the time, the display will stay lit up. */
        
        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enabling writing to the RTC. */

        RTCCFGbits.RTCWREN = 1;

        /* Stop the RTC */

        RTCCFGbits.RTCEN = 0;

        /* Set the RTC register to read/write. */

        RTCCFGbits.RTCPTR0 = 1;
        RTCCFGbits.RTCPTR1 = 0;
        //while(RTCCFGbits.RTCSYNC);

        /* Forward the weekday. */

        unsigned char ucTemp = RTCVALH;
        unsigned char ucValue = 1 + (ucTemp & 7);

        if (ucValue >= 7)
        {
            ucValue = 0;
        }

        /* As accessing the hi-byte will automatically decrement
         * the address pointer, we have to set the address again, before
         * writing the weekday. */

        RTCCFGbits.RTCPTR0 = 1;

        RTCVALH = ucValue;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (istate == DISP_STATE_AUTOSET_YEAR)
    {
        /* Restart the time, the display will stay lit up. */
        
        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        /* Set the RTC register to read/write. */

        RTCCFG |= 3;
        //while(RTCCFGbits.RTCSYNC);

        /* Forward the year. */

        unsigned char ucTemp = RTCVALL;
        unsigned char ucValue = 1 + g_bcd_decimal[ucTemp];

        if (ucValue >= 100)
        {
            ucValue = 0;
        }

        ucTemp = g_decimal_bcd[ucValue];
        RTCVALL = ucTemp;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (istate == DISP_STATE_AUTOSET_CALIBRA)
    {
        /* Restart the time, the display will stay lit up. */
        
        g_ucRollOver = 1;

        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        /* Forward the calibration register and turn around on max. */

        unsigned char ucTemp = RTCCAL;
        
        /* Check if passing the maximum absolute value. */
        
        if (ucTemp & 0x80)
        {
            /* Negative calibration for clocks running too fast. */
            
            if (ucTemp >= 0xFE)
            {
                /* Turn around to zero value. */

                ucTemp = 0;
            }
            else
            {
                ucTemp += 2;
            }
        }
        else // if (ucTemp & 0x80)
        {
            /* Positive calibration for clocks running too slow. */

            if (ucTemp >= 0x7E)
            {
                /* Turn around to maximum negative value. */

                ucTemp = 0x82;
            }
            else
            {
                ucTemp += 2;
            }
        }
        
        RTCCAL = ucTemp;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }

  #endif //   #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET

    /* Holding the date button pressed, will forward to the weekday and year. */

    if ((g_ucPB0TIMEState == PB_STATE_IDLE) && \
        (g_ucPB2HOURState == PB_STATE_IDLE) && \
        (g_ucPB3MINTState == PB_STATE_IDLE))
    {
        if (istate == DISP_STATE_DATE)
        {
            g_uDispState = DISP_STATE_WEEKDAY;
        }
        else if (istate == DISP_STATE_WEEKDAY)
        {
            g_uDispState = DISP_STATE_YEAR;
        }

  #if APP_LIGHT_SENSOR_USAGE_DEBUG_SHOW_VALUE==1

        else if (istate == DISP_STATE_YEAR)
        {
            g_uDispState = DISP_STATE_LIGHT_SENSOR;
        }

  #endif
    }
}

/**
 * Called when button 1 DATE has been released.
 */

void ReleasePB1(void)
{
  #if APP_BUZZER_ALARM_USAGE==1

    DisplayStateType *pb = &g_uDispState;

    if (*pb == DISP_STATE_ALARM)
    {
        if (((g_ucPB0TIMEState == PB_STATE_SHORT_PRESS) || \
             (g_ucPB0TIMEState == PB_STATE_LONG_PRESS)) \
             \
             && \
             \
            (g_ucPB2HOURState == PB_STATE_IDLE) && \
            (g_ucPB3MINTState == PB_STATE_IDLE))
        {
            *pb = DISP_STATE_TOGGLE_ALARM;
        }
    }

  #endif
}

#if APP_WATCH_ANY_PULSAR_MODEL!=APP_WATCH_PULSAR_AUTO_SET

/**
 * Called when button 2 HOUR has been pressed.
 */

void PressPB2(void)
{
  #if APP_BUZZER_ALARM_USAGE==1

    /* Check if the alarm buzzer is still active. */

    if (g_ucAlarm)
    {
        Turn_Buzzer_Off();
    }

  #endif

    /* Revoke the 'blanked' state in order to turn the display on. */

    DisplayStateType *pb = &g_uDispState;

    if (*pb == DISP_STATE_BLANK)
    {
        *pb = DISP_STATE_SET_HOURS;
    }
    else
    {
        /* If the display is in DATE mode and the HOUR button is pressed,
         * enter setting the month mode. */

        const unsigned char ust = *pb;

        if (((ust == DISP_STATE_DATE) || \
             (ust == DISP_STATE_SET_DAY) || \
             (ust == DISP_STATE_SET_MONTH)))
        {
            /* If time and date button have been pressed and the HOUR
             * button is pressed as well, forward the day of month.*/

          #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_MAGNET_SET

            if (g_ucPB1DATEState >= PB_STATE_SHORT_PRESS)
            {
                if (g_ucPB0TIMEState >= PB_STATE_SHORT_PRESS)
                {
                    *pb = DISP_STATE_SET_DAY;
                }
                else
                {
                    *pb = DISP_STATE_SET_MONTH;
                }
            }

          #else

            *pb = DISP_STATE_SET_MONTH;

          #endif
        }
        else
        {
            /* If the display is not in DATE mode, go to set HOURS mode. */

            if ((ust == DISP_STATE_TIME) || (ust == DISP_STATE_SET_MINUTES) || \
                (ust == DISP_STATE_SET_SECONDS))
            {
                *pb = DISP_STATE_SET_HOURS;
            }

          #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_MAGNET_SET

            else
            {
                /* If the display is in YEAR mode, go to set YEAR mode. */

                if (ust == DISP_STATE_YEAR)
                {
                    *pb = DISP_STATE_SET_YEAR;
                }
            }

          #endif
        }
    }

    /* Quick forward the value */

    HoldPB2();
}

/**
 * Called when button 2 HOUR has been held pressed.
 */

void HoldPB2(void)
{
    unsigned char ucTemp;
    unsigned char ucValue;
    unsigned char ucExtra;

    /* Check if setting hours, minutes, month or day. */

    const DisplayStateType ust = g_uDispState;

    if (ust == DISP_STATE_SET_HOURS)
    {
        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        /* Forward the hour. */

        RTCCFGbits.RTCPTR0 = 1;
        RTCCFGbits.RTCPTR1 = 0;
        //while(RTCCFGbits.RTCSYNC);

        ucTemp = RTCVALL;
        ucValue = 1 + g_bcd_decimal[ucTemp];

        if (ucValue >= 24)
        {
            ucValue = 0;
        }

        ucTemp = g_decimal_bcd[ucValue];
        RTCVALL = ucTemp;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (ust == DISP_STATE_SET_MONTH)
    {
        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        /* Forward the month. */

        RTCCFGbits.RTCPTR0 = 0;
        RTCCFGbits.RTCPTR1 = 1;
        //while(RTCCFGbits.RTCSYNC);

        ucTemp = RTCVALH;
        ucValue = 1 + g_bcd_decimal[ucTemp];

        if (ucValue >= 13)
        {
            ucValue = 1;
        }

        RTCCFGbits.RTCPTR0 = 0;
        RTCCFGbits.RTCPTR1 = 1;

        ucTemp = g_decimal_bcd[ucValue];
        RTCVALH = ucTemp;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (ust == DISP_STATE_SET_DAY)
    {
        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

      #if (APP_WATCH_TYPE_BUILD==APP_PULSAR_P3_WRIST_WATCH_12H_ODIN_MOD) || \
          (APP_WATCH_TYPE_BUILD==APP_PULSAR_P4_WRIST_WATCH_12H_SIF_MOD)

        ucExtra = 1;

        /* On every second cycle change betwen AM and PM and otherwise
         * forward the day. */

        RTCCFGbits.RTCPTR0 = 1;
        RTCCFGbits.RTCPTR1 = 0;

        ucTemp = RTCVALL;
        ucValue = g_bcd_decimal[ucTemp];

        if (ucValue < 12)
        {
            /* AM -> PM */

            ucValue += 12;

            ucTemp = g_decimal_bcd[ucValue];
            RTCVALL = ucTemp;

            /* Do not forward the day. */

            ucExtra = 0;
        }
        else
        {
            /* PM -> AM */

            ucValue -= 12;

            ucTemp = g_decimal_bcd[ucValue];
            RTCVALL = ucTemp;
        }

        if (ucExtra)

      #endif

        {
            /* Forward the day of month. */

            RTCCFGbits.RTCPTR0 = 0;
            RTCCFGbits.RTCPTR1 = 1;
            //while(RTCCFGbits.RTCSYNC);

            ucTemp = RTCVALL; // day
            ucValue = 1 + g_bcd_decimal[ucTemp];

            ucTemp = RTCVALH; // month
            ucExtra = g_bcd_decimal[ucTemp];

            if (ucExtra == 2) // February, always assume it to be a leap year.
            {
                /* Assume february to have 29 days to support leap years. */

                if (ucValue >= 30) // 29 days (leap year)
                {
                    ucValue = 1;
                }
            }
            else if ((ucExtra == 1) || (ucExtra == 3) || (ucExtra == 5) || \
                     (ucExtra == 7) || (ucExtra == 8) || (ucExtra == 10) || \
                     (ucExtra == 12))
            {
                if (ucValue >= 32) // 31 days
                {
                    ucValue = 1;
                }
            }
            else
            {
                if (ucValue >= 31) // 30 days
                {
                    ucValue = 1;
                }
            }

            RTCCFGbits.RTCPTR0 = 0;
            RTCCFGbits.RTCPTR1 = 1;

            ucTemp = g_decimal_bcd[ucValue];
            RTCVALL = ucTemp;
        }

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (ust == DISP_STATE_SET_YEAR)
    {
        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        /* Set the RTC register to read/write. */

        RTCCFG |= 3;
        //while(RTCCFGbits.RTCSYNC);

        /* Forward the year. */

        ucTemp = RTCVALL;
        ucValue = 1 + g_bcd_decimal[ucTemp];

        if (ucValue >= 100)
        {
            ucValue = 0;
        }

        ucTemp = g_decimal_bcd[ucValue];
        RTCVALL = ucTemp;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (ust == DISP_STATE_SET_CALIBRA)
    {
        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        /* Forward the calibration register and turn around on max. */

        ucTemp = RTCCAL;
        
        /* Check if passing the maximum absolute value. */
        
        if (ucTemp & 0x80)
        {
            /* Negative calibration for clocks running too fast. */
            
            if (ucTemp <= 0x81)
            {
                /* Turn around to zero value. */

                ucTemp = 0x7E;
            }
            else
            {
                ucTemp -= 2;
            }
        }
        else // if (ucTemp & 0x80)
        {
            /* Positive calibration for clocks running too slow. */

            if (ucTemp <= 0x01)
            {
                /* Turn around to maximum negative value. */

                ucTemp = 0xFE;
            }
            else
            {
                ucTemp -= 2;
            }
        }
        
        RTCCAL = ucTemp;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }

  #if APP_BUZZER_ALARM_USAGE==1

    else if ((ust == DISP_STATE_ALARM) || \
             (ust == DISP_STATE_TOGGLE_ALARM))
    {
        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        /* Forward the alarm hour. */

        ALRMCFGbits.ALRMPTR0 = 1;
        ALRMCFGbits.ALRMPTR1 = 0;

        ucTemp = ALRMVALL;
        ucValue = 1 + g_bcd_decimal[ucTemp];

        if (ucValue >= 24)
        {
            ucValue = 0;
        }

        ucTemp = g_decimal_bcd[ucValue];
        ALRMVALL = ucTemp;

        /* Ensure the other register being valid. */

        ALRMCFGbits.ALRMPTR0 = 0;
        ALRMCFGbits.ALRMPTR1 = 1;

        ALRMVALL = 1; // Day
        ALRMVALH = 1; // Month, auto-decrement!
        ALRMVALH = 1; // Weekday

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }

  #endif
}

/**
 * Called when button 2 HOUR has been released.
 */

void ReleasePB2(void)
{
  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_MAGNET_SET

    /* Turn timer 2 off. */

    T2CONbits.TMR2ON = 0;

    /* Timer usage */

    g_ucTimer2Usage = 0;

  #endif
}

/**
 * Called when button 3 MIN has been pressed.
 */

void PressPB3(void)
{
  #if APP_BUZZER_ALARM_USAGE==1

    /* Check if the alarm buzzer is still active. */

    if (g_ucAlarm)
    {
        Turn_Buzzer_Off();
    }

  #endif

    /* Revoke the 'blanked' state in order to turn the display on. */

    DisplayStateType *pb = &g_uDispState;
    DisplayStateType ust = *pb;

    if (ust == DISP_STATE_BLANK)
    {
        *pb = DISP_STATE_SET_MINUTES;
    }
    else
    {
        /* If the time is pressed, enable to set the MINUTES. */

        if ((ust == DISP_STATE_TIME) || (ust == DISP_STATE_SET_HOURS))
        {
            *pb = DISP_STATE_SET_MINUTES;
        }
        else
        {
          #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_MAGNET_SET

            if (((ust == DISP_STATE_DATE) || (ust == DISP_STATE_YEAR) || \
                 (ust == DISP_STATE_WEEKDAY) || (ust == DISP_STATE_SET_DAY)))
            {
                if (g_ucPB1DATEState >= PB_STATE_SHORT_PRESS)
                {
                    if (g_ucPB0TIMEState >= PB_STATE_SHORT_PRESS)
                    {
                        *pb = DISP_STATE_SET_YEAR;
                    }
                    else
                    {
                        *pb = DISP_STATE_SET_WEEKDAY;
                    }
                }
            }

          #else

            if ((ust == DISP_STATE_DATE) || (ust == DISP_STATE_SET_MONTH))
            {
                g_uDispState = DISP_STATE_SET_DAY;
            }
            else if (ust == DISP_STATE_YEAR)
            {
                g_uDispState = DISP_STATE_SET_YEAR;
            }
            else if (ust == DISP_STATE_WEEKDAY)
            {
                g_uDispState = DISP_STATE_SET_WEEKDAY;
            }
            else if (ust == DISP_STATE_SECONDS)
            {
                g_uDispState = DISP_STATE_SET_SECONDS;
            }

          #endif
        }
    }

    /* Quick forward the value */

    HoldPB3();
}

/**
 * Called when button 3 MIN has been held pressed.
 */

void HoldPB3(void)
{
    unsigned char ucTemp;
    unsigned char ucValue;

    const DisplayStateType ust = g_uDispState;

    if (ust == DISP_STATE_SET_MINUTES)
    {
        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enabling writing to the RTC. */

        RTCCFGbits.RTCWREN = 1;

        /* Stop the RTC */

        RTCCFGbits.RTCEN = 0;

        /* Set the RTC register to read/write. */

        RTCCFG &= ~3;
        //while(RTCCFGbits.RTCSYNC);

        /* Forward the minutes. */

        ucTemp = RTCVALH;
        ucValue = 1 + g_bcd_decimal[ucTemp];

        if (ucValue >= 60)
        {
            ucValue = 0;
        }

        /* Set the RTC register to read/write. */

        //RTCCFG &= ~3;
        //while(RTCCFGbits.RTCSYNC);

        ucTemp = g_decimal_bcd[ucValue];
        RTCVALH = ucTemp;

        /* Set seconds to zero. */

        //RTCCFG &= ~3;

        RTCVALL = 0; // Zero the second when forwarding the minute.

  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_GENERIC_4_BUTTON

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();

  #endif

        /* Keep the RTC disabled and the seconds zero, until the user
         * pressed a readout button. */
    }
    else if (ust == DISP_STATE_SET_YEAR)
    {
        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enabling writing to the RTC. */

        RTCCFGbits.RTCWREN = 1;

        /* Stop the RTC */

        RTCCFGbits.RTCEN = 0;

        /* Set the RTC register to read/write. */

        RTCCFG |= 3;
        //while(RTCCFGbits.RTCSYNC);

        /* Forward the year. */

        ucTemp = RTCVALL;
        ucValue = 1 + g_bcd_decimal[ucTemp];

        if (ucValue >= 100)
        {
            ucValue = 0;
        }

        /* Set the RTC register to read/write. */

        ucTemp = g_decimal_bcd[ucValue];
        RTCVALL = ucTemp;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (ust == DISP_STATE_SET_WEEKDAY)
    {
        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enabling writing to the RTC. */

        RTCCFGbits.RTCWREN = 1;

        /* Stop the RTC */

        RTCCFGbits.RTCEN = 0;

        /* Set the RTC register to read/write. */

        RTCCFGbits.RTCPTR0 = 1;
        RTCCFGbits.RTCPTR1 = 0;
        //while(RTCCFGbits.RTCSYNC);

        /* Forward the weekday. */

        ucTemp = RTCVALH;
        ucValue = 1 + (ucTemp & 7);

        if (ucValue >= 7)
        {
            ucValue = 0;
        }

        /* As accessing the hi-byte will automatically decrement
         * the address pointer, we have to set the address again, before
         * writing the weekday. */

        RTCCFGbits.RTCPTR0 = 1;

        RTCVALH = ucValue;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (ust == DISP_STATE_SET_CALIBRA)
    {
        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        /* Forward the calibration register and turn around on max. */

        ucTemp = RTCCAL;
        
        /* Check if passing the maximum absolute value. */
        
        if (ucTemp & 0x80)
        {
            /* Negative calibration for clocks running too fast. */
            
            if (ucTemp >= 0xFE)
            {
                /* Turn around to zero value. */

                ucTemp = 0;
            }
            else
            {
                ucTemp += 2;
            }
        }
        else // if (ucTemp & 0x80)
        {
            /* Positive calibration for clocks running too slow. */

            if (ucTemp >= 0x7E)
            {
                /* Turn around to maximum negative value. */

                ucTemp = 0x82;
            }
            else
            {
                ucTemp += 2;
            }
        }
        
        RTCCAL = ucTemp;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }

  #if APP_BUZZER_ALARM_USAGE==1

    else if ((ust == DISP_STATE_ALARM) || \
             (ust == DISP_STATE_TOGGLE_ALARM))
    {
        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enable writing to the RTC */

        RTCCFGbits.RTCWREN = 1;

        /* Stop RTC operation. */

        RTCCFGbits.RTCEN = 0;

        /* Forward the alarm minute. */

        ALRMCFGbits.ALRMPTR0 = 0;
        ALRMCFGbits.ALRMPTR1 = 0;

        ucTemp = ALRMVALH;
        ucValue = 1 + g_bcd_decimal[ucTemp];

        if (ucValue >= 60)
        {
            ucValue = 0;
        }

        ucTemp = g_decimal_bcd[ucValue];
        ALRMVALH = ucTemp;

        /* Set Alarm seconds to zero. */
        
        ALRMVALL = 0;

        /* Ensure the other register being valid. */

        ALRMCFGbits.ALRMPTR0 = 0;
        ALRMCFGbits.ALRMPTR1 = 1;

        ALRMVALL = 1; // Day
        ALRMVALH = 1; // Month, auto-decrement!
        ALRMVALH = 1; // Weekday

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }

  #endif

  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_GENERIC_4_BUTTON

    else if (ust == DISP_STATE_SET_DAY)
    {
        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enabling writing to the RTC. */

        RTCCFGbits.RTCWREN = 1;

        /* Stop the RTC */

        RTCCFGbits.RTCEN = 0;

        /* Forward the day of month. */

        RTCCFGbits.RTCPTR0 = 0;
        RTCCFGbits.RTCPTR1 = 1;
        //while(RTCCFGbits.RTCSYNC);

        ucTemp = RTCVALL; // day
        ucValue = 1 + g_bcd_decimal[ucTemp];

        ucTemp = RTCVALH; // month
        unsigned char ucExtra = g_bcd_decimal[ucTemp];

        if (ucExtra == 2) // February, always assume it to be a leap year.
        {
            /* Assume february to have 29 days to support leap years. */

            if (ucValue >= 30) // 29 days (leap year)
            {
                ucValue = 1;
            }
        }
        else if ((ucExtra == 1) || (ucExtra == 3) || (ucExtra == 5) || \
                 (ucExtra == 7) || (ucExtra == 8) || (ucExtra == 10) || \
                 (ucExtra == 12))
        {
            if (ucValue >= 32) // 31 days
            {
                ucValue = 1;
            }
        }
        else
        {
            if (ucValue >= 31) // 30 days
            {
                ucValue = 1;
            }
        }

        RTCCFGbits.RTCPTR0 = 0;
        RTCCFGbits.RTCPTR1 = 1;

        ucTemp = g_decimal_bcd[ucValue];
        RTCVALL = ucTemp;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }
    else if (ust == DISP_STATE_SET_SECONDS)
    {
        /* Unlock write access to the RTC and disable the clock. */

        Unlock_RTCC();

        /* Enabling writing to the RTC. */

        RTCCFGbits.RTCWREN = 1;

        /* Stop the RTC */

        RTCCFGbits.RTCEN = 0;

        /* Set the RTC register to read/write. */

        RTCCFG &= ~3;
        //while(RTCCFGbits.RTCSYNC);

        /* Zero the seconds. */

        RTCVALL = 0;

        /* Enable the RTC operation again.*/

        RTCCFGbits.RTCEN = 1;

        /* Lock writing to the RTCC. */

        Lock_RTCC();
    }

  #endif
}

/**
 * Called when button 3 MIN has been released.
 */

void ReleasePB3(void)
{
  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_MAGNET_SET

    /* For Pulsar P2/P3/P4 compatibility, indicate to stop/stall the RTC,
     * until the time readout button has been pressed the first time. */

    const DisplayStateType ust = g_uDispState;

    if (ust == DISP_STATE_SET_MINUTES)
    {
        g_uDispState = DISP_STATE_SECONDS_STALLED;
    }
    else
    {
        /* Turn timer 2 off. */

        T2CONbits.TMR2ON = 0;

        /* Timer usage */

        g_ucTimer2Usage = 0;
    }

  #endif
}

#endif // #if APP_WATCH_ANY_PULSAR_MODEL!=APP_WATCH_PULSAR_AUTO_SET

#if APP_WRIST_FLICK_USAGE==1

/**
 * Called when the Wrist flick input peaks up and has been stable for
 * some milliseconds.
 */

void PressPB4(void)
{
    /* This handler is called, if the wrist flick input
     * peaks up and has been stable for some milliseconds. */
    
    g_FlickTimeSpan = g_sPB4Timer;
}

/**
 * Called when the Wrist flick input peaks down and has been stable for
 * some milliseconds.
 */

void ReleasePB4(void)
{
    /* This handler is called, if the wrist flick input
     * peaks down and has been stable for some milliseconds. */
    
    /* First read the low byte of the timer, which will buffer
     * the high byte. */

    const unsigned char ulow = TMR0L;

    /* Read now the buffered high byte of the timer, that
     * had been stored, when the low byte had been read. */

    const unsigned char uhigh = TMR0H;

    /* Calculate the tick count, indicating how long the wrist flick
     * input had been peaked up. */
    
    short itimer  = ulow | (uhigh << 8);
          itimer -= g_FlickTimeSpan;
    
    /* Check the wrist flick input for being turned on within a plausible
     * time span. To put it into a nutstell. The input shall be on not
     * too short and not too long. */
          
    if (itimer >= 0)
    {
        if (itimer <= T0_WRIST_FLICK)
        {
            /* Check if the display is still blank. */
            
            if (g_uDispState == DISP_STATE_BLANK)
            {
                if ((!g_ucPB0TIMEState) && \
                    (!g_ucPB1DATEState)

                  #if APP_WATCH_ANY_PULSAR_MODEL != APP_WATCH_PULSAR_AUTO_SET
                    && (!g_ucPB2HOURState) \
                    && (!g_ucPB3MINTState)
                  #endif
                   )
                {
                    /* Press virtually the readout button for the time. */

                    PressPB0();
                    
                    /* Indicate, that the display is lit up by a wrist flick,
                     * if the state has been altered to 'TIME'. */
                    
                    if (g_uDispState == DISP_STATE_TIME)
                    {
                        g_WristFlick = 1;
                    }
                }
            }
        }
    }
}

#endif // #if APP_WRIST_FLICK_USAGE==1

/**
 * Show the time or date.
 */

void Display_Digits(void)
{
    /* Read current digit to show
     * from the multiplexer. */

    unsigned char ucPlex = g_ucMplexDigits;

    /* Lower the brightness, by skipping the multiplexing. */

  #if APP_LIGHT_SENSOR_USAGE==1

    /* Measure ambient brightness via AN11. */

    if (!ucPlex)
    {
        /* Check if we shall feature the last measured value or
         * if we are in need to measure again. */

        if (g_ucDimmingCnt)
        {
            g_ucDimmingCnt--;

            /* Use the result of the last measurment. */

            const unsigned char uv = g_ucDimmingRef;

            g_ucDimming = uv;

            if (uv > 0)
            {
                /* Stop multiplexing, */

                ucPlex = 255;

                /* Turn all common pins off by setting the outputs to tri-state
                 * high impedance by making inputs out of them. */

              #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

                /* Set RB1/4/6 to input, keep RB0/2/3/5/7 as output. */

                TRISB = 0x52; // without RB0

              #else

                /* Set RB0/1/4/6 to input, keep RB2/3/5/7 as output. */

                TRISB = 0x53; // with RB0

              #endif

            #if APP_BUZZER_ALARM_USAGE==1

                /* Set RC0/1/2/4/5..7 to output and RC3 as input. */

                TRISC = 0x08; // RC2 is an output (piezo)

            #else

                /* Set RC0/1/4/5..7 to output and RC3 and RC2(AN11) as input. */

                TRISC = 0x0C; // RC2 is an input (light sensor))

            #endif

                /* Turn all segment outputs off. */

             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

                    PORTC |= 0xF0;
                    PORTB |= 0x2C;

               #if APP_DATE_SPECIAL_DOT_USAGE==1

                    PORTA |= 0x40;

               #endif

             #else
                    PORTC &= 0;
                    PORTB &= 1;

               #if APP_DATE_SPECIAL_DOT_USAGE==1

                    PORTA &= 0xBF;

               #endif

             #endif
            } // if (uv > 0)
        }
        else // if (g_ucDimmingCnt)
        {
            /* Start a new measurment in another 100 cycles. */

            g_ucDimmingCnt = 100;

            /* Turn all common pins off by setting the outputs to tri-state
             * high impedance by making inputs out of them. */

          #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

            /* Set RB1/4/6 to input, keep RB0/2/3/5/7 as output. */

            TRISB = 0x52;

          #else

            /* Set RB0/1/4/6 to input, keep RB2/3/5/7 as output. */

            TRISB = 0x53;

          #endif

        #if APP_BUZZER_ALARM_USAGE==1

            /* Set RC0/1/2/4/5..7 to output and RC3 as input. */

            TRISC = 0x08;

        #else

            /* Set RC0/1/4/5..7 to output and RC3 and RC2(AN11) as input. */

            TRISC = 0x0C;

        #endif

            /* Turn all segment outputs off. */

        #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

            PORTC |= 0xF0;
            PORTB |= 0x2C;

          #if APP_DATE_SPECIAL_DOT_USAGE==1

            PORTA |= 0x40;

          #endif

        #else
            PORTC &= 0;
            PORTB &= 1;

          #if APP_DATE_SPECIAL_DOT_USAGE==1

            PORTA &= 0xBF;

          #endif

        #endif

            /* Turn on RA6 to power up the light sensor. */

        #if APP_WATCH_TYPE_BUILD!=APP_TABLE_WATCH

            PWR_LGTH_SENSOR = 1;

        #endif

            /* Measure the voltage across the resistor. */

            CTMUCONHbits.CTMUEN = 1;    // Enable Charge Time Measurement Unit
            CTMUCONLbits.EDG1STAT = 0;  // Set Edge status bits to zero
            CTMUCONLbits.EDG2STAT = 0;

            CTMUCONHbits.IDISSEN = 1;   // Drain charge on the circuit
            for(int i=0;i<50;i++){};
            CTMUCONHbits.IDISSEN = 0;   // End drain of circuit

            CTMUCONLbits.EDG1STAT = 1;  // Begin charging the circuit
            for(int i=0;i<50;i++){};
            CTMUCONLbits.EDG1STAT = 0;  // Stop charging circuit

            PIR1bits.ADIF = 0;          // Make sure A/D Int not set

            ADCON0bits.GODONE = 1;      // and begin A/D conv.
            while(ADCON0bits.GODONE);   // Wait for A/D convert complete

            unsigned short uv = ADRES;  // Get the value from the A/D

            PIR1bits.ADIF = 0;          // Clear A/D Interrupt Flag

            /* Turn off RA6 to power down the light sensor. */

        #if APP_WATCH_TYPE_BUILD!=APP_TABLE_WATCH

            PWR_LGTH_SENSOR = 0;

        #endif

            /* Calculate the average value between this readout
             * and the very last. */

            g_ucLightSensor = uv;

            /* Calculate brightness factor from readout. */

            if (uv)  // If the resitor would be missing.
            {
              #if APP_WATCH_TYPE_BUILD==APP_PROTOTYPE_BREAD_BOARD

                /* Normal daylight */

                if (uv > 1000)
                {
                    uv = 0;     // Keep maximum brightness.
                }
                else
                {
                    uv = 15 - (uv >> 6);
                }

              #else

                /* Normal daylight */

                if (uv > 128)
                {
                    uv = 0;     // Keep maximum brightness.
                }
                else
                {
                    uv = 1;     // Used dimmed brightness.
                }

              #endif

                g_ucDimming = (unsigned char)uv;
                g_ucDimmingRef = (unsigned char)uv;

                if (uv > 0)
                {
                    /* Stop multiplexing, */

                    ucPlex = 255;
                }
            }
            else
            {
                g_ucDimming = 0;
                g_ucDimmingRef = 0;
            }
        }
    }
    else // if (!ucPlex)
    {
        /* Continue multiplexing after having skipped,
         * to lower the brightness. */

        if (ucPlex == 255)
        {
            ucPlex = 0;
        }
    }

    /* Multiplexing disabled? */

    if (ucPlex != 255)
    {

#endif // #if APP_LIGHT_SENSOR_USAGE==1

        unsigned char ucTemp;

        /* If starting a new multiplexer cycle,
         * read out the RTC registers. */

        if (!ucPlex) // New multiplexing cycle?
        {
            g_ucLeftVal = 255;
            g_ucRightVal = 255;

       #if APP_WATCH_TYPE_BUILD!=APP_PROTOTYPE_BREAD_BOARD

            g_ucDots = 0;

       #endif

            /* Check what shall be displayed. */

            const DisplayStateType ustate = g_uDispState;

            g_uDispStateBackup = ustate;

            switch(ustate)
            {
                /* If being the table watch, show the
                 * time instead of blanking the watch. */

              #if APP_WATCH_TYPE_BUILD==APP_TABLE_WATCH

                case DISP_STATE_BLANK:

                    /* Hours */
                    RTCCFGbits.RTCPTR0 = 1;
                    RTCCFGbits.RTCPTR1 = 0;
                    //while(RTCCFGbits.RTCSYNC);

                    ucTemp = RTCVALL;   // read minutes
                    g_ucLeftVal = g_bcd_decimal[ucTemp];

                    if (g_ucLeftVal > 23)
                    {
                        g_ucLeftVal = 0;
                    }

                    /* Minutes */

                    ucTemp = RTCVALH;   // dummy to decrement
                    ucTemp = RTCVALH;   // read hours
                    g_ucRightVal = g_bcd_decimal[ucTemp];

                    if (g_ucRightVal > 59)
                    {
                        g_ucRightVal = 0;
                    }

                    break;

              #else // #if APP_WATCH_TYPE_BUILD==APP_TABLE_WATCH

                case DISP_STATE_BLANK:
                break;

              #endif // #else #if APP_WATCH_TYPE_BUILD==APP_TABLE_WATCH

                case DISP_STATE_TIME:
                case DISP_STATE_SET_HOURS:
                case DISP_STATE_SET_MINUTES:

              #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET
                case DISP_STATE_AUTOSET_TIME:
              #endif

                    /* Hours */

                    RTCCFGbits.RTCPTR0 = 1;
                    RTCCFGbits.RTCPTR1 = 0;

                    ucTemp = RTCVALL;   // read hours
                    g_ucLeftVal = g_bcd_decimal[ucTemp];

                    if (g_ucLeftVal > 23)
                    {
                        g_ucLeftVal = 0;
                    }

                    /* 24h -> 12h system */

             #if (APP_WATCH_TYPE_BUILD==APP_PULSAR_P3_WRIST_WATCH_12H_ODIN_MOD) || \
                 (APP_WATCH_TYPE_BUILD==APP_PULSAR_P4_WRIST_WATCH_12H_SIF_MOD)

                    g_ucLeftVal = g_24_to_12_hours[g_ucLeftVal];

             #endif

                    /* Minutes */

                    ucTemp = RTCVALH;   // dummy to decrement
                    ucTemp = RTCVALH;   // read minutes
                    g_ucRightVal = g_bcd_decimal[ucTemp];

                    if (g_ucRightVal > 59)
                    {
                        g_ucRightVal = 0;
                    }

             #if (APP_WATCH_TYPE_BUILD==APP_PULSAR_P3_WRIST_WATCH_12H_ODIN_MOD) || \
                 (APP_WATCH_TYPE_BUILD==APP_PULSAR_P4_WRIST_WATCH_12H_SIF_MOD)

                    g_ucDots = 3; // Show both dots.

             #else

               #if APP_ALARM_SPECIAL_DOT_ANIMATION==1

                    if (g_ucAlarm)
                    {
                        g_ucDots = g_dot_banner[g_dot_banner_index >> 2];
                    }

               #endif

             #endif
                break;

                case DISP_STATE_SECONDS:
                case DISP_STATE_SET_SECONDS:

                    RTCCFG &= ~3;

                    g_ucLeftVal = 255; // Hidden
                    ucTemp = RTCVALL;
                    g_ucRightVal = g_bcd_decimal[ucTemp];

                    if (g_ucRightVal > 59)
                    {
                        g_ucRightVal = 0;
                    }
                break;

                case DISP_STATE_SET_CALIBRA:

              #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET
                case DISP_STATE_AUTOSET_CALIBRA:
              #endif

                    ucTemp = RTCCAL;

                    /* Check the value to be negative.
                     * If yes show a minus. */

                    if (ucTemp & 0x80) // MSB
                    {
                        g_ucLeftVal = 128; /*Minus*/

                        ucTemp ^= 0xFF; // 1-compliment
                        ucTemp++;       // 2-compliment
                    }
                    else
                    {
                        g_ucLeftVal = 255; /*Blank*/
                    }

                    /* Show the absolute value on the right digits. */

                    g_ucRightVal = ucTemp >> 1;
                break;

              #if APP_LIGHT_SENSOR_USAGE_DEBUG_SHOW_VALUE==1

                case DISP_STATE_LIGHT_SENSOR:
                    g_ucRightVal = (unsigned char)(g_ucLightSensor / 100);
                    g_ucLeftVal = 255;
                break;

              #endif // #if APP_LIGHT_SENSOR_USAGE_DEBUG_SHOW_VALUE==1

                case DISP_STATE_DATE:
                case DISP_STATE_SET_MONTH:
                case DISP_STATE_SET_DAY:

              #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET
                case DISP_STATE_AUTOSET_DATE:
              #endif

                    RTCCFGbits.RTCPTR0 = 0;
                    RTCCFGbits.RTCPTR1 = 1;

                    /* Day of month */
                    ucTemp = RTCVALL;
                    g_ucRightVal = g_bcd_decimal[ucTemp];

                    if (g_ucRightVal > 31)
                    {
                        g_ucRightVal = 1;
                    }

                    /* Month */
                    ucTemp = RTCVALH;
                    g_ucLeftVal = g_bcd_decimal[ucTemp];

                    if (g_ucLeftVal > 12)
                    {
                        g_ucLeftVal = 1;
                    }

                    /* 24h -> 12h system */

             #if (APP_WATCH_TYPE_BUILD==APP_PULSAR_P3_WRIST_WATCH_12H_ODIN_MOD) || \
                 (APP_WATCH_TYPE_BUILD==APP_PULSAR_P4_WRIST_WATCH_12H_SIF_MOD)

                    /* Hours to indicate AM/PM dot. */

                    RTCCFGbits.RTCPTR0 = 1;
                    RTCCFGbits.RTCPTR1 = 0;

                    ucTemp = RTCVALL;
                    ucTemp = g_bcd_decimal[ucTemp];

                    if (ucTemp > 23)
                    {
                        ucTemp = 0;
                    }

                    g_ucDots = g_24_to_AMPM[ucTemp]; // AM/PM dot

             #elif APP_DATE_SPECIAL_DOT_USAGE==1

                    g_ucDots = 1;

             #endif

                break;

                case DISP_STATE_YEAR:
                case DISP_STATE_SET_YEAR:

              #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET
                case DISP_STATE_AUTOSET_YEAR:
              #endif

                    RTCCFG |= 3;

                    /* Year */
                    ucTemp = RTCVALL;
                    g_ucRightVal = g_bcd_decimal[ucTemp];

                    if (g_ucRightVal > 99)
                    {
                        g_ucRightVal = 0;
                    }

             #if (APP_WATCH_TYPE_BUILD==APP_PULSAR_P3_WRIST_WATCH_12H_ODIN_MOD) || \
                 (APP_WATCH_TYPE_BUILD==APP_PULSAR_P4_WRIST_WATCH_12H_SIF_MOD)

                    g_ucLeftVal = 255;    // Show year with two digits.

             #else

                    g_ucLeftVal = 20;     // Show year using four digits.

             #endif
                break;

                case DISP_STATE_WEEKDAY:
                case DISP_STATE_SET_WEEKDAY:

              #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET
                case DISP_STATE_AUTOSET_WEEKDAY:
              #endif

                    RTCCFGbits.RTCPTR0 = 1;
                    RTCCFGbits.RTCPTR1 = 0;

                    /* Weekday */
                    ucTemp = RTCVALH;

                    if (ucTemp > 6)
                    {
                        ucTemp = 0;
                    }

                    // We do not use the left two digits.
                    g_ucRightVal = ucTemp;
                break;

             #if APP_BUZZER_ALARM_USAGE==1

                case DISP_STATE_ALARM:
                case DISP_STATE_TOGGLE_ALARM:

                    /* Alarm Hours */

                    ALRMCFGbits.ALRMPTR0 = 1;
                    ALRMCFGbits.ALRMPTR1 = 0;

                    ucTemp = ALRMVALL;   // read alarm hours
                    g_ucLeftVal = g_bcd_decimal[ucTemp];

                    if (g_ucLeftVal > 23)
                    {
                        g_ucLeftVal = 0;
                    }

                    /* Alarm Minutes */

                    ucTemp = ALRMVALH;   // dummy to decrement
                    ucTemp = ALRMVALH;   // read minutes
                    g_ucRightVal = g_bcd_decimal[ucTemp];

                    if (g_ucRightVal > 59)
                    {
                        g_ucRightVal = 0;
                    }

                    /* Alarm on/off */

                  #if APP_WATCH_TYPE_BUILD==APP_PULSAR_P3_WRIST_WATCH_24H_LOKI_MOD

                    g_ucDots = ALRMCFGbits.ALRMEN ? 2 : 0;

                  #elif APP_WATCH_TYPE_BUILD==APP_TABLE_WATCH

                    g_ucDots = ALRMCFGbits.ALRMEN ? 2 : 0;

                  #endif
                break;

             #endif // #if APP_BUZZER_ALARM_USAGE==1

                case DISP_STATE_SECONDS_STALLED:
                    g_ucRightVal = 7;
                break;

                default:
                break;
            }

            /* Depending on what to show, select the right digit table. */

            g_pDigits = ((ustate == DISP_STATE_WEEKDAY) || \
                         (ustate == DISP_STATE_SET_WEEKDAY) ||

              #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET

                         (ustate == DISP_STATE_AUTOSET_WEEKDAY) ||

              #endif

                         (ustate == DISP_STATE_SECONDS_STALLED)) ? \
                                   \
                                   g_weekday_7segment : \
                                   g_numerical_7segment;
        } // if (!ucPlex)

        /* Pointer to the 7-segment numerical conversion table. */

        const unsigned char *pb = g_pDigits;

        /* Left two digits. */

        if (ucPlex >= 2)
        {
            /* Hide the left two digits? */

            ucTemp = g_ucLeftVal;

            if (ucTemp != 255)
            {
                /* One hour digit */

                if (ucPlex == 2)
                {
                    /* Setting the accuracy value? */
                    
                    if (ucTemp == 128)
                    {
                        ucTemp = *(pb + 10 /*Minus*/);
                    }
                    else
                    {
                        ucTemp = *(pb + (g_mod10[ucTemp]));
                    }

                    /* Turn all common pins off by setting the outputs
                     * to tri-state high impedance by making inputs out of
                     * them. */

                  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

                    /* Set RB1/4/6 to input, keep RB0/2/3/5/7 as output. */

                    TRISB = 0x52;

                  #else

                    /* Set RB0/1/4/6 to input, keep RB2/3/5/7 as output. */

                    TRISB = 0x53;

                  #endif

            #if APP_BUZZER_ALARM_USAGE==1

                    /* Set RC0/1/2/4/5..7 to output and RC3 as input. */

                    TRISC = 0x08;

            #else

                    /* Set RC0/1/4/5..7 to output and RC3 and RC2(AN11)
                     * as input. */

                    TRISC = 0x0C;

            #endif

                    /* Turn all segment outputs off. */

             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

                    PORTC |= 0xF0;
                    PORTB |= 0x2C;

               #if APP_DATE_SPECIAL_DOT_USAGE==1

                    PORTA |= 0x40;

               #endif

             #else
                    PORTC &= 0;
                    PORTB &= 1;

               #if APP_DATE_SPECIAL_DOT_USAGE==1

                    PORTA &= 0xBF;

               #endif

             #endif

                    // segment a
                    if (ucTemp & 1)
                    {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                        LED_AA_B = 0;
             #else
                        LED_AA_B = 1;
             #endif
                    }

                    // segment b
                    if (ucTemp & 2)
                    {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                        LED_AB_TD = 0;
             #else
                        LED_AB_TD = 1;
             #endif
                    }

                    // segment c
                    if (ucTemp & 4)
                    {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                        LED_AC_LD = 0;
             #else
                        LED_AC_LD = 1;
             #endif
                    }

                    // segment d
                    if (ucTemp & 8)
                    {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                        LED_AD_C = 0;
             #else
                        LED_AD_C = 1;
             #endif
                    }

                    // segment e
                    if (ucTemp & 16)
                    {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                        LED_AE = 0;
             #else
                        LED_AE = 1;
             #endif
                    }

                    // segment f
                    if (ucTemp & 32)
                    {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                        LED_AF = 0;
             #else
                        LED_AF = 1;
             #endif
                    }

                    // segment g
                    if (ucTemp & 64)
                    {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                        LED_AG = 0;
             #else
                        LED_AG = 1;
             #endif
                    }

         #if APP_DATE_SPECIAL_DOT_USAGE==1

             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

                    if (g_ucDots & 1)
                    {
                        LED_DATE_DOT = 0;
                    }

             #else

                    if (g_ucDots & 1)
                    {
                        LED_DATE_DOT = 1;
                    }

             #endif

         #endif

         #if APP_WATCH_TYPE_BUILD!=APP_PROTOTYPE_BREAD_BOARD

             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

                    /* Turn the common anode on. */

                    LED_1H = 1;

             #else
                    /* Turn the common cathode on. */

                    LED_1H = 0;

             #endif

                  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

                    /* Set RB4/6 to input, keep RB0/1/2/3/5/7 as output. */

                    TRISB = 0x50;

                  #else

                    /* Set RB0/4/6 to input, keep RB1/2/3/5/7 as output. */

                    TRISB = 0x51;

                  #endif

         #else // #if APP_WATCH_TYPE_BUILD!=APP_PROTOTYPE_BREAD_BOARD

                    /* Turn the common cathode on. */

                    LED_10M = 0;

                  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

                    /* Set RB1/6 to input, keep RB0/2/3/4/5/7 as output. */

                    TRISB = 0x42;

                  #else

                    /* Set RB0/1/6 to input, keep RB2/3/4/5/7 as output. */

                    TRISB = 0x43;

                  #endif

         #endif // #else #if APP_WATCH_TYPE_BUILD!=APP_PROTOTYPE_BREAD_BOARD
                }
                else if (ucPlex == 3)
                {
           #if (APP_WATCH_TYPE_BUILD==APP_PULSAR_P3_WRIST_WATCH_12H_ODIN_MOD) || \
               (APP_WATCH_TYPE_BUILD==APP_PULSAR_P4_WRIST_WATCH_12H_SIF_MOD)

                    /* Turn all common pins off by setting the outputs
                     * to tri-state high impedance by making inputs out of
                     * them. */

                  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

                    /* Set RB1/4/6 to input, keep RB0/2/3/5/7 as output. */

                    TRISB = 0x52;

                  #else

                    /* Set RB0/1/4/6 to input, keep RB2/3/5/7 as output. */

                    TRISB = 0x53;

                  #endif

             #if APP_BUZZER_ALARM_USAGE==1

                    /* Set RC0/1/2/4/5..7 to output and RC3 as input. */

                    TRISC = 0x08;

             #else

                    /* Set RC0/1/4/5..7 to output and RC3 and RC2(AN11)
                     * as input. */

                    TRISC = 0x0C;

             #endif

                    /* Turn all segment outputs off. */

                    PORTC &= 0;
                    PORTB &= 1;

                    /* Ten hour digit and dots */

                    if ((ucTemp >= 10) && (ucTemp <= 12))
                    {
                        LED_AA_B = 1;
                        LED_AD_C = 1;
                    }

                    /* In time mode turn both dots on.
                     * In date mode upper dot is AM
                     * and lower do is PM.*/

                    LED_AB_TD = (g_ucDots & 1) ? 1 : 0;
                    LED_AC_LD = (g_ucDots & 2) ? 1 : 0;

                    /* Turn the common cathode on. */

                    LED_10H = 0;

                  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

                    /* Set RB1/4 to input, keep RB0/2/3/5/6/7 as output. */

                    TRISB = 0x12;

                  #else

                    /* Set RB0/1/4 to input, keep RB2/3/5/6/7 as output. */

                    TRISB = 0x13;

                  #endif

           #else // Not a 12h system watch.

                    /* Ten hour digit */

             #if APP_WATCH_TYPE_BUILD!=APP_PROTOTYPE_BREAD_BOARD

                    if (ucTemp < 10)
                    {
                        switch(g_uDispStateBackup)
                        {
                            case DISP_STATE_TIME:
                            case DISP_STATE_DATE:
                            case DISP_STATE_SET_HOURS:
                            case DISP_STATE_SET_MINUTES:
                            case DISP_STATE_SET_MONTH:
                            case DISP_STATE_SET_DAY:

                          #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET
                            case DISP_STATE_AUTOSET_TIME:
                            case DISP_STATE_AUTOSET_DATE:
                          #endif

                                ucTemp = 0; // blank
                                break;

                    #if APP_BUZZER_ALARM_USAGE==1

                            case DISP_STATE_ALARM:
                            case DISP_STATE_TOGGLE_ALARM:
                                ucTemp = 0; // blank
                                break;
                    #endif

                            default:
                                ucTemp = *pb; // '0'
                                break;
                        }
                    }
                    else if (ucTemp == 128) // Setting the accuracy.
                    {
                        ucTemp = 0; // blank
                    }
                    else
                    {
                        ucTemp = g_div10[ucTemp];
                        ucTemp = *(pb + ucTemp);
                    }

             #else // Not a Pulsar or table watch.

                    ucTemp = g_div10[g_ucLeftVal];
                    ucTemp = *(pb + ucTemp);

             #endif

                    /* Turn all common pins off by setting the outputs
                     * to tri-state high impedance by making inputs out of
                     * them. */

                  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

                    /* Set RB1/4/6 to input, keep RB0/2/3/5/7 as output. */

                    TRISB = 0x52;

                  #else

                    /* Set RB0/1/4/6 to input, keep RB2/3/5/7 as output. */

                    TRISB = 0x53;

                  #endif

             #if APP_BUZZER_ALARM_USAGE==1

                    /* Set RC0/1/2/4/5..7 to output and RC3 as input. */

                    TRISC = 0x08;

             #else

                    /* Set RC0/1/4/5..7 to output and RC3 and RC2(AN11)
                     * as input. */

                    TRISC = 0x0C;

             #endif

                    /* Turn all segment outputs off. */

             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

                    PORTC |= 0xF0;
                    PORTB |= 0x2C;

               #if APP_DATE_SPECIAL_DOT_USAGE==1

                    PORTA |= 0x40;

               #endif

             #else
                    PORTC &= 0;
                    PORTB &= 1;

               #if APP_DATE_SPECIAL_DOT_USAGE==1

                    PORTA &= 0xBF;

               #endif

             #endif

             #if (APP_WATCH_TYPE_BUILD==APP_PULSAR_P3_WRIST_WATCH_24H_LOKI_MOD) || \
                 (APP_WATCH_TYPE_BUILD==APP_PULSAR_P4_WRIST_WATCH_24H_HEL_MOD)
                    if (ucTemp)
                    {
             #endif
                        // segment a
                        if (ucTemp & 1)
                        {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                            LED_AA_B = 0;
             #else
                            LED_AA_B = 1;
             #endif
                        }
                        // segment b
                        if (ucTemp & 2)
                        {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                            LED_AB_TD = 0;
             #else
                            LED_AB_TD = 1;
             #endif
                        }

                        // segment c
                        if (ucTemp & 4)
                        {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                            LED_AC_LD = 0;
             #else
                            LED_AC_LD = 1;
             #endif
                        }

                        // segment d
                        if (ucTemp & 8)
                        {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                            LED_AD_C = 0;
             #else
                            LED_AD_C = 1;
             #endif
                        }

                        // segment e
                        if (ucTemp & 16)
                        {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                            LED_AE = 0;
             #else
                            LED_AE = 1;
             #endif
                        }

                        // segment f
                        if (ucTemp & 32)
                        {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                            LED_AF = 0;
             #else
                            LED_AF = 1;
             #endif
                        }

                        // segment g
                        if (ucTemp & 64)
                        {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                            LED_AG = 0;
             #else
                            LED_AG = 1;
             #endif
                        }

             #if APP_DATE_SPECIAL_DOT_USAGE==1

                #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

                       if (g_ucDots & 4)
                       {
                           LED_DATE_DOT = 0;
                       }

                #else

                       if (g_ucDots & 4)
                       {
                           LED_DATE_DOT = 1;
                       }

                #endif

             #endif

             #if APP_WATCH_TYPE_BUILD!=APP_PROTOTYPE_BREAD_BOARD

               #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

                        /* Turn the common anode on. */

                        LED_10H = 1;
               #else
                        /* Turn the common cathode on. */

                        LED_10H = 0;
               #endif

                  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

                    /* Set RB1/4 to input, keep RB0/2/3/5/6/7 as output. */

                    TRISB = 0x12;

                  #else

                    /* Set RB0/1/4 input, keep RB2/3/5/6/7 as output. */

                    TRISB = 0x13;

                  #endif

             #else // #if APP_WATCH_TYPE_BUILD!=APP_PROTOTYPE_BREAD_BOARD

                        /* Turn the common cathode on. */

                        LED_1M = 0;

                    #if APP_BUZZER_ALARM_USAGE==1

                        /* Set RC0..7 to outputs. */

                        TRISC = 0x00;

                    #else

                        /* Set RC0/1/3/4/5..7 output and RC2(AN11) as input. */

                        TRISC = 0x04;

                    #endif

             #endif // #else #if APP_WATCH_TYPE_BUILD!=APP_PROTOTYPE_BREAD_BOARD

             #if (APP_WATCH_TYPE_BUILD==APP_PULSAR_P3_WRIST_WATCH_24H_LOKI_MOD) || \
                 (APP_WATCH_TYPE_BUILD==APP_PULSAR_P4_WRIST_WATCH_24H_HEL_MOD)
                    }
             #endif

           #endif // A 24 h based watch.
                }
            }
            else // if (g_ucLeftVal != 255)
            {
               /* Turn all common pins off by setting the outputs to
                * tri-state high impedance by making inputs out of them. */

              #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

                /* Set RB1/4/6 to input, keep RB0/2/3/5/7 as output. */

                TRISB = 0x52;

              #else

                /* Set RB0/1/4/6 to input, keep RB2/3/5/7 as output. */

                TRISB = 0x53;

              #endif

            #if APP_BUZZER_ALARM_USAGE==1

               /* Set RC0/1/2/4/5..7 to output and RC3 as input. */

               TRISC = 0x08;

            #else

               /* Set RC0/1/4/5..7 to output and RC3 and RC2(AN11) as input. */

               TRISC = 0x0C;

            #endif

               /* Turn all segment outputs off. */

             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

                PORTC |= 0xF0;
                PORTB |= 0x2C;

               #if APP_DATE_SPECIAL_DOT_USAGE==1

                PORTA |= 0x40;

               #endif

             #else
                PORTC &= 0;
                PORTB &= 1;

               #if APP_DATE_SPECIAL_DOT_USAGE==1

                PORTA &= 0xBF;

               #endif

             #endif
            }
        }
        else // if (ucPlex >= 2)
        {
            /* Right two digits. */

            ucTemp = g_ucRightVal;

            if (ucTemp != 255)
            {
                /* One minute digit */

                if (!ucPlex)
                {
                    if (pb == g_weekday_7segment)
                    {
                        ucTemp = *(pb + (ucTemp << 1) + 1);
                    }
                    else
                    {
                        ucTemp = *(pb + (g_mod10[ucTemp]));
                    }

                    /* Turn all common pins off by setting the outputs
                     * to tri-state high impedance by making inputs out of
                     * them. */

                  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

                    /* Set RB1/4/6 to input, keep RB0/2/3/5/7 as output. */

                    TRISB = 0x52;

                  #else

                    /* Set RB0/1/4/6 to input, keep RB2/3/5/7 as output. */

                    TRISB = 0x53;

                  #endif

                #if APP_BUZZER_ALARM_USAGE==1

                    /* Set RC0/1/2/4/5..7 to output and RC3 as input. */

                    TRISC = 0x08;

                #else

                    /* Set RC0/1/4/5..7 to output and RC3/2(AN11) input. */

                    TRISC = 0x0C;

                #endif

                    /* Turn all segment outputs off. */

             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

                    PORTC |= 0xF0;
                    PORTB |= 0x2C;

               #if APP_DATE_SPECIAL_DOT_USAGE==1

                    PORTA |= 0x40;

               #endif

             #else
                    PORTC &= 0;
                    PORTB &= 1;

               #if APP_DATE_SPECIAL_DOT_USAGE==1

                    PORTA &= 0xBF;

               #endif

             #endif

                    // segment a
                    if (ucTemp & 1)
                    {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                        LED_AA_B = 0;
             #else
                        LED_AA_B = 1;
             #endif
                    }

                    // segment b
                    if (ucTemp & 2)
                    {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                        LED_AB_TD = 0;
             #else
                        LED_AB_TD = 1;
             #endif
                    }

                    // segment c
                    if (ucTemp & 4)
                    {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                        LED_AC_LD = 0;
             #else
                        LED_AC_LD = 1;
             #endif
                    }

                    // segment d
                    if (ucTemp & 8)
                    {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                        LED_AD_C = 0;
             #else
                        LED_AD_C = 1;
             #endif
                    }

                    // segment e
                    if (ucTemp & 16)
                    {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                        LED_AE = 0;
             #else
                        LED_AE = 1;
             #endif
                    }

                    // segment f
                    if (ucTemp & 32)
                    {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                        LED_AF = 0;
             #else
                        LED_AF = 1;
             #endif
                    }

                    // segment g
                    if (ucTemp & 64)
                    {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                        LED_AG = 0;
             #else
                        LED_AG = 1;
             #endif
                    }

         #if APP_DATE_SPECIAL_DOT_USAGE==1

             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

                    if (g_ucDots & 2)
                    {
                        LED_DATE_DOT = 0;
                    }

             #else

                    if (g_ucDots & 2)
                    {
                        LED_DATE_DOT = 1;
                    }

             #endif

         #endif

         #if APP_WATCH_TYPE_BUILD!=APP_PROTOTYPE_BREAD_BOARD

             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

                    /* Turn the common anodes on. */

                    LED_1M = 1;

             #else
                    /* Turn the common cathode on. */

                    LED_1M = 0;

             #endif

             #if APP_BUZZER_ALARM_USAGE==1

                    /* Set RC0..7 to outputs. */

                    TRISC = 0x00;

             #else

                    /* Set RC0/1/3/4/5..7 to output and RC2(AN11) input. */

                    TRISC = 0x04;

             #endif

         #else // #if APP_WATCH_TYPE_BUILD!=APP_PROTOTYPE_BREAD_BOARD

                    /* Turn the common cathode on. */

                    LED_10H = 0;

                  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

                    /* Set RB1/4 to input, keep RB0/2/3/5/6/7 as output. */

                    TRISB = 0x12;

                  #else

                    /* Set RB0/1/4 to input, keep RB2/3/5/6/7 as output. */

                    TRISB = 0x13;

                  #endif

         #endif // #else #if APP_WATCH_TYPE_BUILD!=APP_PROTOTYPE_BREAD_BOARD

                }
                else if (ucPlex == 1)
                {
                    /* Ten minute digit */

                    if (pb == g_weekday_7segment)
                    {
                        ucTemp = *(pb + (ucTemp << 1));
                    }
                    else
                    {

             #if APP_WATCH_TYPE_BUILD!=APP_PROTOTYPE_BREAD_BOARD

                        if (ucTemp < 10)
                        {
                            switch(g_uDispStateBackup)
                            {
                                case DISP_STATE_DATE:
                                case DISP_STATE_SET_MONTH:
                                case DISP_STATE_SET_DAY:

                              #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET
                                case DISP_STATE_AUTOSET_DATE:
                              #endif

                                    ucTemp = 0; // blank
                                    break;

                                default:
                                    ucTemp = *pb; // '0'
                                    break;
                            }
                        }
                        else
                        {
                            ucTemp = g_div10[ucTemp];
                            ucTemp = *(pb + ucTemp);
                        }

             #else // Not a Pulsar or table watch.

                        ucTemp = g_div10[ucTemp];
                        ucTemp = *(pb + ucTemp);

             #endif
                    }

                    /* Turn all common pins off by setting the outputs to tri-state
                     * high impedance by making inputs out of them. */

                  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

                    /* Set RB1/4/6 to input, keep RB0/2/3/5/7 as output. */

                    TRISB = 0x52;

                  #else

                    /* Set RB0/1/4/6 to input, keep RB2/3/5/7 as output. */

                    TRISB = 0x53;

                  #endif

                #if APP_BUZZER_ALARM_USAGE==1

                    /* Set RC0/1/2/4/5..7 to output and RC3 as input. */

                    TRISC = 0x08;

                #else

                    /* Set RC0/1/4/5..7 to output and RC3 and RC2(AN11) as input. */

                    TRISC = 0x0C;

                #endif

                    /* Turn all segment outputs off. */

             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

                    PORTC |= 0xF0;
                    PORTB |= 0x2C;

               #if APP_DATE_SPECIAL_DOT_USAGE==1

                    PORTA |= 0x40;

               #endif

             #else
                    PORTC &= 0;
                    PORTB &= 1;

               #if APP_DATE_SPECIAL_DOT_USAGE==1

                    PORTA &= 0xBF;

               #endif

             #endif

             #if APP_WATCH_TYPE_BUILD!=APP_PROTOTYPE_BREAD_BOARD
                    if (ucTemp)
                    {
             #endif
                        // segment a
                        if (ucTemp & 1)
                        {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                            LED_AA_B = 0;
             #else
                            LED_AA_B = 1;
             #endif
                        }

                        // segment b
                        if (ucTemp & 2)
                        {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                            LED_AB_TD = 0;
             #else
                            LED_AB_TD = 1;
             #endif
                        }

                        // segment c
                        if (ucTemp & 4)
                        {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                            LED_AC_LD = 0;
             #else
                            LED_AC_LD = 1;
             #endif
                        }

                        // segment d
                        if (ucTemp & 8)
                        {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                            LED_AD_C = 0;
             #else
                            LED_AD_C = 1;
             #endif
                        }

                        // segment e
                        if (ucTemp & 16)
                        {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                            LED_AE = 0;
             #else
                            LED_AE = 1;
             #endif
                        }

                        // segment f
                        if (ucTemp & 32)
                        {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                            LED_AF = 0;
             #else
                            LED_AF = 1;
             #endif
                        }

                        // segment g
                        if (ucTemp & 64)
                        {
             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE
                            LED_AG = 0;
             #else
                            LED_AG = 1;
             #endif
                        }

             #if APP_DATE_SPECIAL_DOT_USAGE==1

                #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

                       if (g_ucDots & 8)
                       {
                           LED_DATE_DOT = 0;
                       }

                #else

                       if (g_ucDots & 8)
                       {
                           LED_DATE_DOT = 1;
                       }

                #endif

             #endif

             #if APP_WATCH_TYPE_BUILD!=APP_PROTOTYPE_BREAD_BOARD

                 #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

                        /* Turn the common anode on. */

                        LED_10M = 1;
                 #else

                        /* Turn the common cathode on. */

                        LED_10M = 0;
                 #endif

                  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

                        /* Set RB1/6 to input, keep RB0/2/3/4/5/7 as output. */

                        TRISB = 0x42;

                  #else

                        /* Set RB0/1/6 to input, keep RB2/3/4/5/7 as output. */

                        TRISB = 0x43;

                  #endif

             #else // #if APP_WATCH_TYPE_BUILD!=APP_PROTOTYPE_BREAD_BOARD

                        /* Turn the common cathode on. */

                        LED_1H = 0;

                  #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

                        /* Set RB4/6 to input, keep RB0/1/2/3/5/7 as output. */

                        TRISB = 0x50;

                  #else

                        /* Set RB0/4/6 to input, keep RB1/2/3/5/7 as output. */

                        TRISB = 0x51;

                  #endif

             #endif // #else #if APP_WATCH_TYPE_BUILD!=APP_PROTOTYPE_BREAD_BOARD

             #if APP_WATCH_TYPE_BUILD!=APP_PROTOTYPE_BREAD_BOARD
                    }
             #endif
                }
            }
            else // if (g_ucRightVal != 255)
            {
                /* Turn all common pins off by setting the outputs to
                 * tri-state high impedance by making inputs out of them. */

              #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

                /* Set RB1/4/6 to input, keep RB0/2/3/5/7 as output. */

                TRISB = 0x52;

              #else

                /* Set RB0/1/4/6 to input, keep RB2/3/5/7 as output. */

                TRISB = 0x53;

              #endif

                #if APP_BUZZER_ALARM_USAGE==1

                    /* Set RC0/1/2/4/5..7 to output and RC3 as input. */

                    TRISC = 0x08;

                #else

                    /* Set RC0/1/4/5..7 output and RC3 and RC2(AN11) input. */

                    TRISC = 0x0C;

                #endif

                /* Turn all segment outputs off. */

             #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

                    PORTC |= 0xF0;
                    PORTB |= 0x2C;

               #if APP_DATE_SPECIAL_DOT_USAGE==1

                    PORTA |= 0x40;

               #endif

             #else
                    PORTC &= 0;
                    PORTB &= 1;

               #if APP_DATE_SPECIAL_DOT_USAGE==1

                    PORTA &= 0xBF;

               #endif

             #endif
            }
        }

        /* Update the segments of the digits. */

        if (++ucPlex >= 4)
        {
            ucPlex = 0;
        }

  #if APP_LIGHT_SENSOR_USAGE==1
    }
  #endif // #if APP_LIGHT_SENSOR_USAGE==1

    /* Store new multiplexer value. */

    g_ucMplexDigits = ucPlex;
}

/**
 * Main entry point, started via the cold start vector of the controller.
 */

void main(void)
{
    unsigned char udivider;

    /* Initialize and configure. */

    Init_Inputs_Outputs_Ports();
    Configure_Inputs_Outputs();

    Configure_Timer_0();
    Configure_Timer_1();
    Configure_Timer_2();
    Configure_Timer_3();
    Configure_Timer_4();

    Configure_Real_Time_Clock();
    
    Init_Button_States();

    /* If the controller has been waken up from deep sleep, we have to unlock
     * the general purpose inputs and outputs first. */

    if (WDTCONbits.DS == 1)
    {
        /* Clear deep sleep status bits and release GPIOS from
         * deep sleep lock. */

        WDTCONbits.DS = 0;

        // DEEP SLEEP CONTROL LOW BYTE REGISTER
        DSCONLbits.RELEASE = 0; //  Clear to unfreeze the I/O's.

        /* The two general purpose registers of the MCU are non-volatile and
         * survive the deep sleep mode. They are named DSGPR0 and DSGPR1. */

        // DSGPR0
        // DSGPR1
    }
    else
    {
        /* Turn the 'stay awake' timer off again. */

  #if APP_WATCH_ANY_PULSAR_MODEL!=APP_WATCH_GENERIC_4_BUTTON

        // Magnet set or Auto-set Pulsar wrist watch.
        
        TMR2 = 0;               // Zero the timer.
        T2CONbits.TMR2ON = 1;   // Turn timer 2 on.
        g_ucTimer2Usage = 1;    // Indicate using the timer.

        /* Set the display state to time reading. */

        g_uDispState = DISP_STATE_TIME;
        
#if APP_BUZZER_ALARM_USAGE==1

        /* Turn the alarm buzzer on. */

        Turn_Buzzer_On(448/*duration*/);

#endif // #if APP_BUZZER_ALARM_USAGE==1

  #else // #if APP_WATCH_ANY_PULSAR_MODEL!=APP_WATCH_GENERIC_4_BUTTON

        /* Set the display state to blank. */

        g_uDispState = DISP_STATE_BLANK;

  #endif
    }
    
    /* Enable global interrupts. */

    /* INTERRUPT CONTROL REGISTER */

    // Global Interrupt Enable bit
    INTCONbits.GIE = 1;      // Enables all unmasked interrupts
    // Peripheral Interrupt Enable bit
    INTCONbits.PEIE = 1;     // Enables all unmasked peripheral interrupts

    /* INTERRUPT CONTROL REGISTER 2 */

    // External Interrupt 0 Edge Select bit
    INTCON2bits.INTEDG0 = 1; // Interrupt on rising edge
    // External Interrupt 1 Edge Select bit
    INTCON2bits.INTEDG1 = 1; // Interrupt on rising edge
    // External Interrupt 2 Edge Select bit
    INTCON2bits.INTEDG2 = 1; // Interrupt on rising edge
    // External Interrupt 3 Edge Select bit
    INTCON2bits.INTEDG3 = 1; // Interrupt on rising edge

    /* PORTB Pull-up Disable bit
     * 
     * RBPU: PORTB Pull-up Enable bit
     * 1 - All PORTB pull-ups are disabled
     * 0 - PORTB pull-ups are enabled by individual port tri-state values
     */
    
    INTCON2bits.RBPU = 1;    // All PORTB pull-ups are disabled.

    /* Init Multiplexing for digits. */

    g_ucMplexDigits = 0;
    
  #if APP_LIGHT_SENSOR_USAGE==1

    g_ucDimmingCnt = 0;
    g_ucDimming = 0;
    g_ucDimmingRef = 0;

  #endif // #if APP_LIGHT_SENSOR_USAGE==1
    
    /* Init local variables. */

    g_ucStayAwake = 0;
    g_ucRollOver = 1;

    udivider = 0;

    while (1)
    {
        /* Check alarm. */

      #if APP_BUZZER_ALARM_USAGE==1

        if (PIR3bits.RTCCIF) // PERIPHERAL INTERRUPT REQUEST (FLAG) REGISTER 3
        {
            PIR3bits.RTCCIF = 0;     // Clear RTCC interrupt.

            /* Reset the alarm repeat. */

            ALRMRPT = 255;

            /* Turn the alarm buzzer on. */

            Turn_Buzzer_On(6000/*duration*/);
            
            /* Initialize dot animation. */
            
          #if APP_ALARM_SPECIAL_DOT_ANIMATION==1
            
            g_dot_banner_index = 0;
            
          #endif
        }

      #endif // #if APP_BUZZER_ALARM_USAGE==1

        /* Debounce the buttons. */

        g_ucStayAwake = DebounceButtons();

        /* Handle 'stay awake' timer for keeping the display
         * on for a short while. */

        if (g_ucTimer2Usage)
        {
            if (TMR2 >= 0xA0)
            {
                TMR2 = 0;

              #if APP_BUZZER_ALARM_USAGE==1

                /* Check if the alarm buzzer is still active. */

                unsigned short *palarm = &g_ucAlarm;

                if (*palarm)
                {
                    (*palarm)--;

                    /* Turn the alarm buzzer off, if required. */

                    const unsigned short ualarm = *palarm;

                    if (!ualarm)
                    {
                        Turn_Buzzer_Off();
                    }
                    else
                    {
                        Turn_Buzzer_Fancy((ualarm >> 6) & 0x07);
                        
                        /* Do the dot animation. */
            
                    #if APP_ALARM_SPECIAL_DOT_ANIMATION==1

                        unsigned char udot = g_dot_banner_index;
                        
                        udot++;
                        
                        if (udot >= (sizeof(g_dot_banner) << 2))
                        {
                            udot = 0;
                        }
                        
                        g_dot_banner_index = udot;

                    #endif
                    }

                    /* If there is still the alarm buzzer activated,
                     * restart the rollover counter with a short value. */

                    g_ucRollOver = 100;
                }

              #endif // #if APP_BUZZER_ALARM_USAGE==1

                /* Counter for keeping the display on. */

            #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

                const DisplayStateType istate = g_uDispState;
                const unsigned short ulimit = \

                            /* Using AutoSet, keep the display lit for long. */

                            (istate >= DISP_STATE_AUTOSET_TIME) ? 1250 : \

                            /* Grant more time on a wrist flick event. */
                                
                            g_WristFlick ? 500 : 375;
                
                if (++g_ucRollOver >= ulimit) // Rounds per second.
                {
            #else

                if (++g_ucRollOver >= 375) // Rounds per second.
                {
                    const DisplayStateType istate = g_uDispState;

            #endif
                    /* Check if all button states are idle. */

                    if ( /* No button pressed. */

                        (!g_ucPB0TIMEState) && \
                        (!g_ucPB1DATEState) &&
                            
                      #if APP_WATCH_ANY_PULSAR_MODEL!=APP_WATCH_PULSAR_AUTO_SET

                        (!g_ucPB2HOURState) && \
                        (!g_ucPB3MINTState) &&

                      #endif

                        /* Display not in 'watch stalled' mode. */
                            
                        (istate != DISP_STATE_SECONDS_STALLED))
                    {
                        /* If using the Pulsar Autoset button mode. */

                    #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET

                        if ((istate == DISP_STATE_AUTOSET_TIME) && \
                            (g_ucTimePressCnt == HINT_AUTOSET_MINUTES_SET))
                        {
                            /* Unlock write access to the RTC and disable the clock. */

                            Unlock_RTCC();

                            /* Enabling writing to the RTC. */

                            RTCCFGbits.RTCWREN = 1;

                            /* Stop the RTC */

                            RTCCFGbits.RTCEN = 0;

                            /* Set the RTC register to read/write. */

                            RTCCFG &= ~3;
                            //while(RTCCFGbits.RTCSYNC);

                            /* Zero the seconds. */

                            RTCVALL = 0;

                            /* Indicate the the watch had been stopped. */

                            g_uDispState = DISP_STATE_SECONDS_STALLED;
                        }
                        else

                    #endif // #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET

                        {
                            /* If no button is still pressed, turn the 'awake'
                             * timer off. */

                            T2CONbits.TMR2ON = 0;

                            /* Indicate that we do not need to stay awake
                             * anymore. */

                            g_ucRollOver = 0;

                            /* Indicate that timer 2 is not used anymore. */

                            g_ucTimer2Usage = 0;
                        }
                    }
                    else
                    {
                        /* If there is still at least one button pressed,
                         * restart the rollover counter with a short value. */

                        g_ucRollOver = 100;
                    }
                }
            }

            g_ucStayAwake |= g_ucRollOver ? 1 : 0;
        }
        else // if (g_ucTimer2Usage)
        {
            g_ucRollOver = 1;
        }

        /* Output the display. */

     #if APP_WATCH_TYPE_BUILD==APP_TABLE_WATCH

      #if APP_LIGHT_SENSOR_USAGE==1

        if (g_ucDimming)
        {
            /* Skip multiplexing, keep display off. */

            g_ucDimming--;

            udivider = 3;
        }
        else

      #endif

        if (!(udivider & 3))
        {
            Display_Digits();
        }

        udivider++;

        if (!g_ucStayAwake)
        {
            /* Set blank mode. */

            g_uDispState = DISP_STATE_BLANK;
        }

     #else // #if APP_WATCH_TYPE_BUILD==APP_TABLE_WATCH

        if (g_ucStayAwake)
        {
          #if APP_LIGHT_SENSOR_USAGE==1

            if (g_ucDimming)
            {
                /* Skip multiplexing, keep display off. */

                g_ucDimming--;
                
                udivider = 2;
            }
            else
                
          #endif
                
            if (udivider == 3)
            {
                Display_Digits();
                
                udivider = 1;
            }
            else
            {
                udivider++;
            }
        }
        else
        {
            /* If using the Pulsar Autoset button mode, there
             * are two button press counters for the TIME and DATE
             * buttons, that are reset, when the display is turned off. */

        #if APP_WATCH_ANY_PULSAR_MODEL == APP_WATCH_PULSAR_AUTO_SET

            g_ucTimePressCnt = 0;
            g_ucDatePressCnt = 0;

            /* Enter sleep again? */

            /* Turn all common pins off by setting the outputs to tri-state
             * high impedance by making inputs out of them. */

          #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

            /* Set none to input, keep RB0/1/2/3/4/5/6/7 as output. */

            TRISB  = 0x00;

          #else

            /* Set RB1/4/6 to input, keep RB0/2/3/5/7 as output. */

            TRISB = 0x52;

          #endif

        #else

          #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

            /* Set RB0 to input, keep RB1/2/3/4/5/6/7 as output. */

            TRISB  = 0x01;

          #else

            /* Set RB0/1/4/6 to input, keep RB2/3/5/7 as output. */

            TRISB = 0x53;

          #endif

        #endif

        #if APP_BUZZER_ALARM_USAGE==1

          #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

            /* Set RC0/1/2/3/4/5/6/7 to output and none as input. */

            TRISC = 0x00;

          #else

            /* Set RC0/1/2/4/5..7 to output and RC3 as input. */

            TRISC = 0x08;

          #endif

        #else

          #if APP_WATCH_COMMON_PIN_USING==APP_WATCH_COMMON_ANODE

            /* Set RC0/1/3/4/5/6/7 to output and RC2(AN11) as input. */

            TRISC = 0x04;

          #else

            /* Set RC0/1/4/5/6/7 to output and RC3 and RC2(AN11) as input. */

            TRISC = 0x0C;

          #endif

        #endif

            /* Turn all segment outputs off. */

            PORTC &= 0x00;
            PORTB &= 0x01;
            PORTA &= 0x27;

            /* Ensure the RTC to operate, if not being in 'stalled' state,
             * after having set the minutes. */

            Unlock_RTCC();

            /* Set write enable bit. */

            RTCCFGbits.RTCWREN = 1; // RTCC Value Registers Write Enable bit

            /* Enable setting pointers to read MIN and SEC. */

            RTCCFGbits.RTCEN = 1;   // RTCC module is enabled

            /* Set blank mode. */

            g_uDispState = DISP_STATE_BLANK;

            /* If entering sleep, ensure the light sensor value to be zero. */

          #if APP_LIGHT_SENSOR_USAGE==1

            g_ucLightSensor = 0;
            
          #endif

            /* Reset the buttons. */
            
            Init_Button_States();

            /* Reset timer values and states. */

            g_ucTimer0Usage = 0;
            g_ucTimer2Usage = 0;

            /* Init Multiplexing for digits. */

            g_ucMplexDigits = 0;

            /* Stop the debouncing timer used. */

            T0CONbits.TMR0ON = 0;

            /* Stop the stay awake timer used. */

            T2CONbits.TMR2ON = 0;

            /* Poll RTCSYNC until it has cleared.
             *
             * The RTCSYNC bit indicates a time window during
             * which the RTCC Clock Domain registers can be safely
             * read and written without concern about a rollover.
             * When RTCSYNC = 0, the registers can be safely accessed. */

            while(RTCCFGbits.RTCSYNC);

            /* Clear write enable bit. */

            RTCCFGbits.RTCWREN = 0; // RTCC Value Registers Write Enable bit

            /* Lock writing to the RTCC. */

            Lock_RTCC();

          #if APP_BUZZER_ALARM_USAGE==1

            /* Turn the alarm buzzer off. */

            Turn_Buzzer_Off();

          #endif // #if APP_BUZZER_ALARM_USAGE==1

            /* Reset wrist flick indication. */
            
          #if APP_WRIST_FLICK_USAGE==1

            g_WristFlick = 0;
            
          #endif
            /* PORTB Pull-up Disable bit
             * 
             * RBPU: PORTB Pull-up Enable bit
             * 1 - All PORTB pull-ups are disabled
             * 0 - PORTB pull-ups are enabled by individual port tri-state values
             */

            INTCON2bits.RBPU = 1;    // All PORTB pull-ups are disabled.

            /* Clear all interuppts. */

            // HOUR button
            INTCONbits.INT0IF = 0;  // Clear INT0 Flag
          #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET
            INTCONbits.INT0IE = 0;  // Disable INT0
          #else
            INTCONbits.INT0IE = 1;  // Enable INT0
          #endif // #if APP_WATCH_ANY_PULSAR_MODEL!=APP_WATCH_PULSAR_AUTO_SET

            // MIN/WRIST FLICK
            INTCON3bits.INT1IF = 0;  // Clear INT1 Flag
          #if APP_WATCH_ANY_PULSAR_MODEL==APP_WATCH_PULSAR_AUTO_SET
           #if APP_WRIST_FLICK_USAGE==1
            INTCON3bits.INT1IE = 1;  // Enable INT1 (Flick))
           #else
            INTCON3bits.INT1IE = 0;  // Disable INT1
           #endif
          #else
            INTCON3bits.INT1IE = 1;  // Enable INT1 (Min))
          #endif

            // DATE button
            INTCON3bits.INT2IF = 0;  // Clear INT2 Flag
            INTCON3bits.INT2IE = 1;  // Enable INT2

            // TIME button
            INTCON3bits.INT3IF = 0;  // Clear INT3 Flag
            INTCON3bits.INT3IE = 1;  // Enable INT3

            /* INTERRUPT CONTROL REGISTER */
            
            INTCONbits.TMR0IF = 0;    // TMR0 register did not overflow.

            /* Wake up on a rising edge of the HOUR button. */

            INTCON2bits.INTEDG0 = 1;    // External Interrupt 0 Edge Select bit

            /* Wake up on a rising edge of the DATE button. */

            INTCON2bits.INTEDG1 = 1;    // External Interrupt 1 Edge Select bit

            /* Wake up on a rising edge of the MIN or WRIST FLICK button. */

            INTCON2bits.INTEDG2 = 1;    // External Interrupt 2 Edge Select bit

            /* Wake up on a rising edge of the TIME button. */

            INTCON2bits.INTEDG3 = 1;    // External Interrupt 3 Edge Select bit
            
            /* PERIPHERAL INTERRUPT REQUEST (FLAG) REGISTER 1 */

            PIR1bits.TMR1IF = 0;    // TMR1 register did not overflow.

            /* PERIPHERAL INTERRUPT REQUEST (FLAG) REGISTER 3 */

            PIR3bits.RTCCIF = 0;    // No RTCC interrupt occurred.

      #if APP_BUZZER_ALARM_USAGE==1

            /* Enable the Alarm interrupt, if the alarm had been enabled. */

            PIE3bits.RTCCIE = ALRMCFGbits.ALRMEN ? 1 : 0;

      #endif // #if APP_BUZZER_ALARM_USAGE==1

            /* INTERRUPT CONTROL REGISTER */

            // Global Interrupt Enable bit
            INTCONbits.GIE = 1;    // Enables all unmasked interrupts
            // Peripheral Interrupt Enable bit
            INTCONbits.PEIE = 1;   // Enables all unmasked peripheral interrupts

            /* Turn the ADC (Analog-to-Digital Converter) and the Charge Time
             * Measurement Unit off again to save power - about 6uA. */

            CTMUCONHbits.CTMUEN = 0;// Disable the Charge Time Measurement Unit
            ADCON0bits.ADON = 0;    // Turn off ADC
            ANCON1bits.VBGEN = 0;   // Turn off the Bandgap to save power.
            
            /* Enter sleep mode. This might fail, for example if inputs are
             * still high, that are used to wake up the controller via
             * rising edge. */

            enterSleep();
            
            /* Global counter for the timer used to keep the display lit. */

            g_ucRollOver = 1;

            /* Reset light sensor variables. */

          #if APP_LIGHT_SENSOR_USAGE==1

            g_ucDimming = 0;
            g_ucDimmingCnt = 0;
            g_ucDimmingRef = 0;

          #endif

            /* Divider for going from one digit to the next. */

            udivider = 0;

           /* Configure I/O. */

           Init_Inputs_Outputs_Ports();
           Configure_Inputs_Outputs();
        }

     #endif // #else #if APP_WATCH_TYPE_BUILD==APP_TABLE_WATCH

    } // while (1)
}

/* EoF */