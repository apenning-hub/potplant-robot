#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <Arduino.h>
#include "sensors.h"

// ---------------------------------------------------------------------------
// telemetry.h - the robot telling you what it is thinking
//
// One line of comma separated values per report, including columns for sensors
// and needs that do not exist yet - those stay empty until their phase arrives.
// Keeping the format fixed means a tool written today still works in Phase 7.
//
// Columns:
//   ms,ldrFL,ldrFR,ldrBR,ldrBL,steer,fb,dist,pir,touch,
//   light,warmth,affection,water,rest,spdL,spdR,state,mood
// ---------------------------------------------------------------------------

void telemetryHeader();
void telemetryLine(const LightReading &light, const char *state, const char *mood);

#endif
