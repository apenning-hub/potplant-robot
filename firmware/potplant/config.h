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
// Three LDRs (light dependent resistors) looking left, straight ahead, and
// right. Each sits in a voltage divider with a fixed resistor, so the Arduino
// reads a number from 0 to 1023 that changes with how much light hits it.
const uint8_t PIN_LDR_LEFT   = A0;
const uint8_t PIN_LDR_CENTRE = A1;
const uint8_t PIN_LDR_RIGHT  = A2;

// --- Motors ----------------------------------------------------------------
// Channel numbers printed on the motor shield itself: M1, M2, M3, M4.
// We use M1 and M2. They share one L293D driver chip, which is fine at the
// small currents these TT gear motors draw.
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
//   D2   ultrasonic trigger      A3   ultrasonic echo
//   A4   PIR motion sensor       A5   capacitive touch pad
//   D9   LED matrix DIN          D10  LED matrix CS      D13  LED matrix CLK
//
// The motor shield occupies D3, D4, D5, D6, D7, D8, D11 and D12.
// D0 and D1 are the USB serial connection. Never use them for anything.
//
// That is every pin on the UNO accounted for. Temperature and soil moisture
// in Phase 5 will not fit, which is the point at which we discuss a Mega.

#endif
