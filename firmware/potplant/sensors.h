#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// ---------------------------------------------------------------------------
// sensors.h - reading the world
//
// Right now this is only the four light sensors. Phase 2 adds the ultrasonic
// module and the cliff sensors, Phase 3 motion and touch. They all land in this
// one file so the rest of the code never talks to a pin directly.
// ---------------------------------------------------------------------------

// One set of light readings, smoothed, calibrated, and turned the right way up
// so that a BIGGER number always means BRIGHTER.
struct LightReading {
  int fl, fr, br, bl;   // the four corners, 0 - 1023

  // Two comparisons the steering actually uses, worked out once here rather
  // than recalculated wherever they are needed.
  int steer;      // (right side) - (left side). Positive means brighter right.
  int frontBack;  // (front) - (back).          Positive means brighter ahead.

  int brightest;  // the strongest single corner
};

void sensorsBegin();
void sensorsCalibrate();       // blocking, runs once at start-up only
void sensorsUpdate();          // call once per tick
LightReading sensorsLight();

#endif
