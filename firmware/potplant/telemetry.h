#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <Arduino.h>
#include "sensors.h"

// ---------------------------------------------------------------------------
// telemetry.h - the robot telling you what it is thinking
//
// One line of comma separated values per report. The column list is fixed for
// the whole life of the project, including columns for sensors and needs that
// do not exist yet - those stay empty until their phase arrives. Keeping the
// format stable means tools/telemetry.py never has to be rewritten.
//
// Columns:
//   ms,ldrL,ldrC,ldrR,err,dist,pir,touch,
//   light,warmth,affection,water,rest,spdL,spdR,state,mood
// ---------------------------------------------------------------------------

void telemetryHeader();
void telemetryLine(const LightReading &light, const char *state, const char *mood);

#endif
