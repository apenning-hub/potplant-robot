#include "telemetry.h"
#include "drive.h"

// Every literal string is wrapped in F(). Without it, the text would be copied
// into the Arduino's 2 KB of RAM at start-up and sit there forever. F() leaves
// it in the 32 KB of program flash instead, and reads it out when needed.
// On a chip this small that difference decides whether the project fits.

void telemetryHeader() {
  Serial.println(F("# potplant telemetry. Lines starting with # are notes, not data."));
  Serial.println(F("ms,ldrL,ldrC,ldrR,err,dist,pir,touch,light,warmth,affection,water,rest,spdL,spdR,state,mood"));
}

void telemetryLine(const LightReading &light, const char *state, const char *mood) {
  Serial.print(millis());       Serial.print(',');
  Serial.print(light.left);     Serial.print(',');
  Serial.print(light.centre);   Serial.print(',');
  Serial.print(light.right);    Serial.print(',');
  Serial.print(light.error);    Serial.print(',');

  // dist, pir, touch - Phase 2 and 3. Empty for now.
  Serial.print(F(",,,"));

  // light, warmth, affection, water, rest - the needs model, Phase 2 onwards.
  Serial.print(F(",,,,,"));

  Serial.print(driveLeft());    Serial.print(',');
  Serial.print(driveRight());   Serial.print(',');
  Serial.print(state);          Serial.print(',');
  Serial.println(mood);
}
