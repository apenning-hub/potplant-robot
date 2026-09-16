#ifndef CONFIG_H
#define CONFIG_H

// ---------------------------------------------------------------------------
// config.h - WHERE THINGS ARE PLUGGED IN
//
// This file describes the physical robot: which wire goes into which pin, and
// which way round the motors are wired. If you rewire the hardware, change it
// here and nowhere else.
//
// Nothing in this file changes the robot's personality. That lives in tuning.h.
// ---------------------------------------------------------------------------

#include <Arduino.h>

// --- Light sensors ---------------------------------------------------------
// Four LDRs (light dependent resistors), one at each corner. Each sits in a
// voltage divider with a fixed resistor, so the Arduino reads a number from
// 0 to 1023 that changes with how much light hits it.
//
//              FRONT
//         FL          FR
//         A0          A1
//            [ ROBOT ]
//         A3          A2
//         BL          BR
//              BACK
//
// Four rather than three, because three forward-facing sensors cannot tell a
// dark room from a window behind them - both read as "dim ahead". With one at
// each corner the robot knows when light is behind it and can turn round.
const uint8_t PIN_LDR_FL = A0;   // front left
const uint8_t PIN_LDR_FR = A1;   // front right
const uint8_t PIN_LDR_BR = A2;   // back right
const uint8_t PIN_LDR_BL = A3;   // back left

// --- Motors ----------------------------------------------------------------
// Channel numbers printed on the motor shield itself: M1, M2, M3, M4.
// We use M1 and M2. They share one L293D driver chip, which is fine at the
// small currents these TT gear motors draw.
// M1 and M2, not M3 or M4, and the reason is worth knowing.
//
// M1 and M2 take their speed control from Timer 2, where the frequency can be
// set - we ask for 64 kHz, which is far above hearing. M3 and M4 run off the
// same timer as millis(), and the library deliberately refuses to touch that
// timer's speed in order to keep the clock accurate. The side effect is that
// M3 and M4 ignore the frequency you ask for and always run near 976 Hz, which
// is right in the audible range and makes the motors whine.
//
// Mixing them would be worse still: two wheels on different PWM frequencies
// respond differently to the same speed number, and the robot veers.
const uint8_t MOTOR_LEFT_CHANNEL  = 1;   // M1
const uint8_t MOTOR_RIGHT_CHANNEL = 2;   // M2

// If a wheel spins the wrong way, flip the matching flag here rather than
// pulling the wires off the screw terminals and swapping them over.
const bool MOTOR_LEFT_INVERT  = false;
const bool MOTOR_RIGHT_INVERT = false;

// --- Serial ----------------------------------------------------------------
// 115200 rather than 9600. At 9600 baud a single telemetry line takes about
// 60 milliseconds to send, which is longer than one tick - the robot would
// spend more time talking than thinking.
const long SERIAL_BAUD = 115200;

// --- Pins reserved for later phases ----------------------------------------
// Not used yet. Listed here so the whole pin budget stays visible in one file,
// and so nobody accidentally uses one of them for something else.
//
//   D2   ultrasonic, single-pin mode (trigger and echo tied via 1k)
//   A4   I2C data  (SDA)         A5   I2C clock (SCL)
//   D9   LED matrix DIN          D10  LED matrix CS      D13  LED matrix CLK
//
// The PIR, touch pad and two cliff sensors hang off an I2C expander on A4/A5
// rather than taking pins of their own. Anything needing microsecond timing -
// the ultrasonic echo - must stay on a real pin.
//
// The motor shield occupies D3, D4, D5, D6, D7, D8, D11 and D12.
// D0 and D1 are the USB serial connection. Never use them for anything.
//
// Four LDRs take every analogue pin except A4 and A5, and those two are the
// I2C pins. The sensor bus is therefore not optional - it is the only way the
// rest of the project fits. See docs/design-notes.md.

#endif
