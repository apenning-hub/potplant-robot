#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// ---------------------------------------------------------------------------
// sensors.h - reading the world
//
// Right now this is only the three light sensors. Phase 2 adds the ultrasonic
// distance sensor, Phase 3 the motion and touch sensors. They will all land in
// this one file so the rest of the code never talks to a pin directly.
// ---------------------------------------------------------------------------

// One set of light readings, already smoothed, calibrated, and turned the
// right way up so that a BIGGER number always means BRIGHTER.
struct LightReading {
  int left;       // 0 - 1023
  int centre;
  int right;
  int error;      // right minus left. Positive means brighter to the right.
  int brightest;  // whichever of the three is reading highest
};

void sensorsBegin();
void sensorsCalibrate();       // blocking, runs once at start-up only
void sensorsUpdate();          // call once per tick
LightReading sensorsLight();

#endif
