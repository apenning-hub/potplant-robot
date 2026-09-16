#ifndef TUNING_H
#define TUNING_H

// ---------------------------------------------------------------------------
// tuning.h - THE PERSONALITY
//
// Every number that changes how the robot BEHAVES lives in this one file.
// Nothing here describes the hardware. You can change any of these, upload,
// and watch the robot act differently, without understanding the rest of the
// code at all. This is the file to play with.
//
// Two plants running identical code but different copies of this file would
// have noticeably different temperaments. That is the whole idea.
// ---------------------------------------------------------------------------

#include <Arduino.h>

// --- How often the robot thinks ---------------------------------------------
// One "tick" is one round of: read sensors, decide, move. 50 ms means it
// thinks 20 times a second. Faster is twitchier, slower is more ponderous.
const unsigned long TICK_MS = 50;

// Printing a telemetry line every single tick floods the screen. Every 4th
// tick is about 5 lines a second, which is readable and still plots nicely.
const uint8_t TELEMETRY_EVERY_N_TICKS = 4;

// --- Smoothing the light sensors --------------------------------------------
// Raw sensor readings jitter, partly from electrical noise and partly because
// mains-powered room lights actually flicker 100 times a second. We average
// each reading with its own history to settle it down.
//
// This is a "shift" value: 1 is barely any smoothing, 5 is very heavy and
// sluggish. 3 is a good middle.
const uint8_t LDR_SMOOTHING = 3;

// --- Start-up calibration ----------------------------------------------------
// How long to hold still while it takes its baseline readings, in milliseconds.
const unsigned long CALIBRATE_SETTLE_MS  = 2500;
// How long you get to cover the sensors with your hand.
const unsigned long CALIBRATE_COVER_MS   = 4000;
// How much the reading must change while covered before we trust it. If your
// room is very dim to begin with, lower this.
const int CALIBRATE_MIN_SWING = 40;

// --- Seeking the light -------------------------------------------------------
// If the difference between the two sides is smaller than this, treat it as
// "even" and drive straight. Without a deadband the robot weaves constantly,
// chasing meaningless noise.
//
// Note this compares the sum of two sensors per side, not one, so the numbers
// here are roughly twice what they would be with a single sensor each side.
const int LIGHT_DEADBAND = 24;

// If even the brightest sensor reads below this, there is no useful light
// gradient to follow and the robot searches instead of seeking.
const int LIGHT_FLOOR = 60;

// How hard it turns towards the bright side. 100 means "turn as hard as the
// brightness difference suggests". 50 is half as eager, 200 is twice as eager
// and will overshoot and weave.
const int STEER_GAIN_PERCENT = 60;

// --- Turning round ------------------------------------------------------------
// Four corner sensors let the robot notice that the light is BEHIND it, which
// three forward-facing sensors could never do. When the back pair beat the front
// pair by this much, it turns round rather than driving hopefully onward.
const int LIGHT_BEHIND_THRESHOLD = 40;

// Once it has started turning, it keeps going until the front pair beat the back
// pair by this much. Requiring more to stop than to start is what stops it
// dithering back and forth when the light is exactly off to one side.
const int LIGHT_AHEAD_THRESHOLD = 25;

// How fast it spins while turning round, and how long before it gives up and
// goes back to driving. Without the timeout a robot in an evenly lit room can
// spin for ever.
const uint8_t TURN_SPEED = 85;
const unsigned long TURN_TIMEOUT_MS = 3000;

// --- Motor speeds (0 to 255) --------------------------------------------------
// Cruising speed when driving straight towards light.
const uint8_t DRIVE_BASE_SPEED = 90;
// Never exceed this, even mid-turn.
const uint8_t DRIVE_MAX_SPEED  = 180;
// Below this these gear motors just buzz and stall instead of turning, so any
// commanded speed between 1 and this gets pushed up to it.
const uint8_t DRIVE_MIN_SPEED  = 55;

// How fast the motors are allowed to change speed, per tick. This stops the
// robot slamming from full forward into full reverse, which is hard on plastic
// gearboxes and makes the whole pot lurch. Lower is gentler and more graceful.
const uint8_t DRIVE_SLEW_PER_TICK = 12;

// --- Searching (when it is too dark to steer) ---------------------------------
// Speed of the slow spin it does while hunting for any light at all.
const uint8_t SEARCH_SPEED = 75;
// How long it spins one way before trying the other way.
const unsigned long SEARCH_SWEEP_MS = 1400;

#endif
