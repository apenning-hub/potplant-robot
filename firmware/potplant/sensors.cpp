#include "sensors.h"
#include "config.h"
#include "tuning.h"

// ---------------------------------------------------------------------------
// The three LDR dividers will never match each other exactly. Resistors are
// only accurate to 5 percent, and no two LDRs are identical either. So at
// start-up we take a baseline and work out a small correction for each channel.
//
// We also work out, by asking you to cover the sensors, whether your wiring
// gives a HIGHER number in brighter light or a LOWER one. That depends on
// whether the LDR is the top or bottom leg of the voltage divider, and it
// means you do not have to trace a single wire to find out.
// ---------------------------------------------------------------------------

static long smoothed[3];              // running averages, kept at 8x scale
static int  gain[3] = {256, 256, 256}; // per-channel correction, 256 = no change
static bool brighterIsHigher = true;
static bool calibrated = false;

static const uint8_t LDR_PINS[3] = { PIN_LDR_LEFT, PIN_LDR_CENTRE, PIN_LDR_RIGHT };

// Read all three pins right now, no smoothing, no correction.
static void readRaw(int out[3]) {
  for (uint8_t i = 0; i < 3; i++) {
    out[i] = analogRead(LDR_PINS[i]);
  }
}

// Average several raw reads over a period, to get a settled baseline.
static void averageOver(unsigned long ms, int out[3]) {
  long total[3] = {0, 0, 0};
  long count = 0;
  unsigned long start = millis();
  while (millis() - start < ms) {
    int raw[3];
    readRaw(raw);
    for (uint8_t i = 0; i < 3; i++) total[i] += raw[i];
    count++;
    delay(5);   // safe here: calibration runs once, before the main loop starts
  }
  for (uint8_t i = 0; i < 3; i++) out[i] = (int)(total[i] / count);
}

void sensorsBegin() {
  int raw[3];
  readRaw(raw);
  for (uint8_t i = 0; i < 3; i++) smoothed[i] = (long)raw[i] << 3;
}

void sensorsCalibrate() {
  int ambient[3];
  int covered[3];

  Serial.println(F("# --- Light sensor calibration ---"));
  Serial.println(F("# Put the robot where it normally lives and leave it alone."));
  averageOver(CALIBRATE_SETTLE_MS, ambient);
  Serial.print(F("# ambient raw: "));
  Serial.print(ambient[0]); Serial.print(' ');
  Serial.print(ambient[1]); Serial.print(' ');
  Serial.println(ambient[2]);

  Serial.println(F("# NOW COVER ALL THREE SENSORS WITH YOUR HAND."));
  delay(1200);   // a moment to actually get your hand there
  averageOver(CALIBRATE_COVER_MS, covered);
  Serial.print(F("# covered raw: "));
  Serial.print(covered[0]); Serial.print(' ');
  Serial.print(covered[1]); Serial.print(' ');
  Serial.println(covered[2]);

  // Which direction does the number move when light is blocked? Average the
  // change across all three so one dud sensor cannot fool us.
  long swing = 0;
  for (uint8_t i = 0; i < 3; i++) swing += (long)covered[i] - ambient[i];

  if (abs(swing) < (long)CALIBRATE_MIN_SWING * 3) {
    // Not enough of a change to be sure. Keep whatever we assumed and say so.
    Serial.println(F("# WARNING: sensors barely changed when covered."));
    Serial.println(F("# Either your hand missed them, or the room is very dim,"));
    Serial.println(F("# or a sensor is not wired up. Assuming higher = brighter."));
    brighterIsHigher = true;
  } else {
    // If covering them made the numbers go DOWN, then high means bright.
    brighterIsHigher = (swing < 0);
    Serial.print(F("# wiring detected: higher reading means "));
    Serial.println(brighterIsHigher ? F("BRIGHTER") : F("DARKER"));
  }

  // Now balance the three channels against each other, using the ambient
  // readings turned the right way up.
  int level[3];
  for (uint8_t i = 0; i < 3; i++) {
    level[i] = brighterIsHigher ? ambient[i] : (1023 - ambient[i]);
    if (level[i] < 1) level[i] = 1;   // never divide by zero
  }
  long mean = ((long)level[0] + level[1] + level[2]) / 3;
  for (uint8_t i = 0; i < 3; i++) {
    gain[i] = (int)((mean << 8) / level[i]);
    // Refuse to correct by more than about 2x either way. A gain that extreme
    // means a sensor is faulty or unplugged, not merely a bit off.
    if (gain[i] < 128) gain[i] = 128;
    if (gain[i] > 512) gain[i] = 512;
  }

  Serial.print(F("# channel gains (256 = no correction): "));
  Serial.print(gain[0]); Serial.print(' ');
  Serial.print(gain[1]); Serial.print(' ');
  Serial.println(gain[2]);
  Serial.println(F("# Calibration done. Uncover the sensors."));
  Serial.println(F("# --------------------------------"));

  calibrated = true;
  sensorsBegin();   // reset the smoothing so it does not start from stale data
}

void sensorsUpdate() {
  int raw[3];
  readRaw(raw);
  // Exponential moving average, done in integers. Each tick we move the stored
  // value a fraction of the way towards the new reading. LDR_SMOOTHING decides
  // how small that fraction is.
  for (uint8_t i = 0; i < 3; i++) {
    long target = (long)raw[i] << 3;
    smoothed[i] += (target - smoothed[i]) >> LDR_SMOOTHING;
  }
}

LightReading sensorsLight() {
  LightReading r;
  int v[3];
  for (uint8_t i = 0; i < 3; i++) {
    int s = (int)(smoothed[i] >> 3);
    if (!brighterIsHigher) s = 1023 - s;    // flip so bigger always means brighter
    long corrected = ((long)s * gain[i]) >> 8;
    if (corrected < 0) corrected = 0;
    if (corrected > 1023) corrected = 1023;
    v[i] = (int)corrected;
  }
  r.left   = v[0];
  r.centre = v[1];
  r.right  = v[2];
  r.error  = v[2] - v[0];
  r.brightest = max(v[0], max(v[1], v[2]));
  return r;
}
