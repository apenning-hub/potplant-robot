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

// Channel order throughout this file is FL, FR, BR, BL - clockwise from the
// front left, the same order the corners are listed in config.h.
static const uint8_t CHANNELS = 4;

static long smoothed[CHANNELS];                    // running averages, 8x scale
static int  gain[CHANNELS] = {256, 256, 256, 256}; // correction, 256 = no change
static bool brighterIsHigher = true;

static const uint8_t LDR_PINS[CHANNELS] = {
  PIN_LDR_FL, PIN_LDR_FR, PIN_LDR_BR, PIN_LDR_BL
};

// Read all three pins right now, no smoothing, no correction.
static void readRaw(int out[CHANNELS]) {
  for (uint8_t i = 0; i < CHANNELS; i++) {
    out[i] = analogRead(LDR_PINS[i]);
  }
}

// Average several raw reads over a period, to get a settled baseline.
static void averageOver(unsigned long ms, int out[CHANNELS]) {
  long total[CHANNELS] = {0, 0, 0, 0};
  long count = 0;
  unsigned long start = millis();
  while (millis() - start < ms) {
    int raw[CHANNELS];
    readRaw(raw);
    for (uint8_t i = 0; i < CHANNELS; i++) total[i] += raw[i];
    count++;
    delay(5);   // safe here: calibration runs once, before the main loop starts
  }
  for (uint8_t i = 0; i < CHANNELS; i++) out[i] = (int)(total[i] / count);
}

// Print four readings on one line, in corner order.
static void printFour(const int v[CHANNELS]) {
  for (uint8_t i = 0; i < CHANNELS; i++) {
    Serial.print(v[i]);
    if (i < CHANNELS - 1) Serial.print(' ');
  }
  Serial.println();
}

void sensorsBegin() {
  int raw[CHANNELS];
  readRaw(raw);
  for (uint8_t i = 0; i < CHANNELS; i++) smoothed[i] = (long)raw[i] << 3;
}

void sensorsCalibrate() {
  int ambient[CHANNELS];
  int covered[CHANNELS];

  Serial.println(F("# --- Light sensor calibration ---"));
  Serial.println(F("# Order is FL FR BR BL, clockwise from the front left."));
  Serial.println(F("# Put the robot where it normally lives and leave it alone."));
  averageOver(CALIBRATE_SETTLE_MS, ambient);
  Serial.print(F("# ambient raw: ")); printFour(ambient);

  Serial.println(F("# NOW COVER ALL FOUR SENSORS WITH YOUR HANDS."));
  delay(1500);   // a moment to actually get your hands there
  averageOver(CALIBRATE_COVER_MS, covered);
  Serial.print(F("# covered raw: ")); printFour(covered);

  // Which direction does the number move when light is blocked? Average the
  // change across all three so one dud sensor cannot fool us.
  long swing = 0;
  for (uint8_t i = 0; i < CHANNELS; i++) swing += (long)covered[i] - ambient[i];

  if (abs(swing) < (long)CALIBRATE_MIN_SWING * CHANNELS) {
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
  int level[CHANNELS];
  long sum = 0;
  for (uint8_t i = 0; i < CHANNELS; i++) {
    level[i] = brighterIsHigher ? ambient[i] : (1023 - ambient[i]);
    if (level[i] < 1) level[i] = 1;   // never divide by zero
    sum += level[i];
  }
  long mean = sum / CHANNELS;
  for (uint8_t i = 0; i < CHANNELS; i++) {
    gain[i] = (int)((mean << 8) / level[i]);
    // Refuse to correct by more than about 2x either way. A gain that extreme
    // means a sensor is faulty or unplugged, not merely a bit off.
    if (gain[i] < 128) gain[i] = 128;
    if (gain[i] > 512) gain[i] = 512;
  }

  Serial.print(F("# channel gains (256 = no correction): ")); printFour(gain);
  Serial.println(F("# Calibration done. Uncover the sensors."));
  Serial.println(F("# --------------------------------"));

  sensorsBegin();   // reset the smoothing so it does not start from stale data
}

void sensorsUpdate() {
  int raw[CHANNELS];
  readRaw(raw);
  // Exponential moving average, done in integers. Each tick we move the stored
  // value a fraction of the way towards the new reading. LDR_SMOOTHING decides
  // how small that fraction is.
  for (uint8_t i = 0; i < CHANNELS; i++) {
    long target = (long)raw[i] << 3;
    smoothed[i] += (target - smoothed[i]) >> LDR_SMOOTHING;
  }
}

LightReading sensorsLight() {
  LightReading r;
  int v[CHANNELS];
  for (uint8_t i = 0; i < CHANNELS; i++) {
    int s = (int)(smoothed[i] >> 3);
    if (!brighterIsHigher) s = 1023 - s;    // flip so bigger always means brighter
    long corrected = ((long)s * gain[i]) >> 8;
    if (corrected < 0) corrected = 0;
    if (corrected > 1023) corrected = 1023;
    v[i] = (int)corrected;
  }
  r.fl = v[0];
  r.fr = v[1];
  r.br = v[2];
  r.bl = v[3];

  // Steering compares the two sides, using all four corners rather than just
  // the front pair. Front and back together give a steadier reading.
  r.steer     = (r.fr + r.br) - (r.fl + r.bl);

  // Front against back is what tells the robot the light is behind it - the
  // thing three forward-facing sensors could never know.
  r.frontBack = (r.fl + r.fr) - (r.br + r.bl);

  r.brightest = max(max(v[0], v[1]), max(v[2], v[3]));
  return r;
}
