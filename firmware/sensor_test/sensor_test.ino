// ===========================================================================
// SENSOR TEST BENCH
//
// A diagnostic sketch. This is not the robot - it does not roam, seek light or
// have moods. It exists so you can check each piece of hardware works before
// trusting it, and so you can find out which motor is plugged into which
// terminal without guessing.
//
// Open it in the Arduino IDE, press Upload, then open the Serial Monitor and
// SET THE SPEED TO 115200. Type a letter and press Enter.
//
//   ?    show the menu again
//   l    live light readings, with bars
//   r    range test - gives you numbers to paste into tuning.h
//   m    motor sweep - runs all four channels in turn
//   1-4  run one motor channel only
//   u    ultrasonic distance
//   i    scan the I2C bus for connected devices
//   p    read A4 and A5 as plain digital inputs
//   f    free memory
//   x    stop everything now
//
// A note on delay(): the robot firmware never uses it, because a frozen robot
// cannot notice a table leg. This sketch uses it freely. Nothing here has to
// react to anything, and blocking makes a test rig much easier to read.
//
// Uses the Adafruit Motor Shield library (AFMotor.h) for the v1 shield.
// ===========================================================================

#include <AFMotor.h>
#include <Wire.h>

// --- What is plugged in where -----------------------------------------------
// Keep these matching config.h in the robot firmware.
const uint8_t PIN_LDR[4] = { A0, A1, A2, A3 };
const char*   LDR_NAME[4] = { "A0 front-left ", "A1 front-right",
                              "A2 back-right ", "A3 back-left  " };

const uint8_t PIN_ULTRASONIC = 2;    // trigger and echo commoned through 1k
const uint8_t PIN_SPARE_A    = A4;   // I2C data - also readable as digital
const uint8_t PIN_SPARE_B    = A5;   // I2C clock

// All four motor channels, so you can find out which is which.
// Channels 1 and 2 take 64 kHz, above hearing. Channels 3 and 4 ignore the
// frequency argument and run near 976 Hz - you will hear them whine, and that
// is exactly why the robot uses M1 and M2.
AF_DCMotor motor1(1, MOTOR12_64KHZ);
AF_DCMotor motor2(2, MOTOR12_64KHZ);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

const uint8_t TEST_SPEED = 140;

// ---------------------------------------------------------------------------

int freeRam() {
  extern int __heap_start, *__brkval;
  int here;
  return (int)&here - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

void allMotorsOff() {
  motor1.run(RELEASE); motor2.run(RELEASE);
  motor3.run(RELEASE); motor4.run(RELEASE);
  motor1.setSpeed(0);  motor2.setSpeed(0);
  motor3.setSpeed(0);  motor4.setSpeed(0);
}

void menu() {
  Serial.println();
  Serial.println(F("=== SENSOR TEST BENCH ======================="));
  Serial.println(F("  l   live light readings"));
  Serial.println(F("  r   range test (gives you tuning numbers)"));
  Serial.println(F("  m   motor sweep, all four channels"));
  Serial.println(F("  1-4 run one motor channel"));
  Serial.println(F("  u   ultrasonic distance"));
  Serial.println(F("  i   scan the I2C bus"));
  Serial.println(F("  p   read A4/A5 as digital inputs"));
  Serial.println(F("  f   free memory"));
  Serial.println(F("  x   stop everything"));
  Serial.println(F("  ?   this menu"));
  Serial.println(F("============================================="));
}

// --- Light -------------------------------------------------------------------

// Draw one reading as a bar, so you can see it move rather than read numbers.
void drawBar(int value) {
  int filled = map(value, 0, 1023, 0, 32);
  Serial.print('[');
  for (int i = 0; i < 32; i++) Serial.print(i < filled ? '#' : '.');
  Serial.print(']');
}

void liveLight() {
  Serial.println();
  Serial.println(F("Live light. Wave a torch at each sensor in turn."));
  Serial.println(F("Send any character to stop."));
  Serial.println();
  while (!Serial.available()) {
    for (uint8_t i = 0; i < 4; i++) {
      int v = analogRead(PIN_LDR[i]);
      Serial.print(LDR_NAME[i]);
      Serial.print(' ');
      drawBar(v);
      Serial.print(' ');
      if (v < 1000) Serial.print(' ');
      if (v < 100)  Serial.print(' ');
      if (v < 10)   Serial.print(' ');
      Serial.println(v);
    }
    Serial.println();
    delay(250);
  }
  while (Serial.available()) Serial.read();
  Serial.println(F("Stopped."));
}

// Watch for 20 seconds while you cover and uncover the sensors, then report
// the range each one saw and suggest values for tuning.h.
void rangeTest() {
  int lo[4], hi[4];
  for (uint8_t i = 0; i < 4; i++) { lo[i] = 1023; hi[i] = 0; }

  Serial.println();
  Serial.println(F("RANGE TEST - 20 seconds."));
  Serial.println(F("Cover each sensor with your hand, then uncover it."));
  Serial.println(F("Shine a torch on each one. Get the extremes."));
  Serial.println();

  unsigned long start = millis();
  uint8_t lastSecond = 99;
  while (millis() - start < 20000) {
    for (uint8_t i = 0; i < 4; i++) {
      int v = analogRead(PIN_LDR[i]);
      if (v < lo[i]) lo[i] = v;
      if (v > hi[i]) hi[i] = v;
    }
    uint8_t sec = (20000 - (millis() - start)) / 1000;
    if (sec != lastSecond) {
      lastSecond = sec;
      if (sec % 5 == 0) { Serial.print(sec); Serial.println(F("...")); }
    }
    delay(10);
  }

  Serial.println();
  Serial.println(F("--- RESULTS ---"));
  int worstSpan = 1023;
  long sumLo = 0, sumHi = 0;
  for (uint8_t i = 0; i < 4; i++) {
    int span = hi[i] - lo[i];
    Serial.print(LDR_NAME[i]);
    Serial.print(F("  min ")); Serial.print(lo[i]);
    Serial.print(F("  max ")); Serial.print(hi[i]);
    Serial.print(F("  span ")); Serial.print(span);
    if (span < 50) Serial.print(F("   <-- TOO SMALL, check this one"));
    Serial.println();
    if (span < worstSpan) worstSpan = span;
    sumLo += lo[i]; sumHi += hi[i];
  }

  Serial.println();
  if (worstSpan < 50) {
    Serial.println(F("At least one sensor barely moved. Likely causes:"));
    Serial.println(F("  - missing its fixed resistor (an LDR alone cannot be read)"));
    Serial.println(F("  - wire in the wrong hole"));
    Serial.println(F("  - you did not actually cover that one"));
  } else {
    // Suggest a floor a little above the darkest reading seen, and a deadband
    // scaled to how much range the sensors actually have.
    int floorSuggest = (int)(sumLo / 4) + (worstSpan / 8);
    int bandSuggest  = worstSpan / 12;
    if (bandSuggest < 6) bandSuggest = 6;
    Serial.println(F("Suggested starting values for tuning.h:"));
    Serial.print(F("  LIGHT_FLOOR    = ")); Serial.println(floorSuggest);
    Serial.print(F("  LIGHT_DEADBAND = ")); Serial.println(bandSuggest * 2);
    Serial.println(F("  (deadband is doubled because steering sums two sensors"));
    Serial.println(F("   per side, not one)"));
  }
}

// --- Motors ------------------------------------------------------------------

void runOne(uint8_t channel) {
  AF_DCMotor *m;
  switch (channel) {
    case 1: m = &motor1; break;
    case 2: m = &motor2; break;
    case 3: m = &motor3; break;
    default: m = &motor4; break;
  }
  Serial.println();
  Serial.print(F("Channel M")); Serial.print(channel);
  Serial.println(F(" - watch which wheel turns."));

  m->setSpeed(TEST_SPEED);
  Serial.println(F("  FORWARD"));
  m->run(FORWARD);
  delay(1500);
  m->run(RELEASE);
  delay(600);

  Serial.println(F("  BACKWARD"));
  m->run(BACKWARD);
  delay(1500);
  m->run(RELEASE);
  m->setSpeed(0);
  Serial.println(F("  stopped"));
}

void motorSweep() {
  Serial.println();
  Serial.println(F("MOTOR SWEEP"));
  Serial.println(F("Wheels off the ground, please. Prop it on a book."));
  Serial.println(F("Each channel runs forwards then backwards for 1.5 s."));
  Serial.println(F("Note which wheel moves on which channel - that is what"));
  Serial.println(F("goes into MOTOR_LEFT_CHANNEL and MOTOR_RIGHT_CHANNEL."));
  Serial.println();
  Serial.println(F("Starting in 3 seconds..."));
  delay(3000);
  for (uint8_t c = 1; c <= 4; c++) runOne(c);
  allMotorsOff();
  Serial.println();
  Serial.println(F("Sweep done. Nothing should be moving now."));
}

// --- Ultrasonic ---------------------------------------------------------------

// Single-pin mode: trigger and echo share one pin, commoned through a 1k
// resistor. The pin is an output for the pulse, then an input for the echo.
long readDistanceMm() {
  pinMode(PIN_ULTRASONIC, OUTPUT);
  digitalWrite(PIN_ULTRASONIC, LOW);
  delayMicroseconds(3);
  digitalWrite(PIN_ULTRASONIC, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_ULTRASONIC, LOW);

  pinMode(PIN_ULTRASONIC, INPUT);
  unsigned long us = pulseIn(PIN_ULTRASONIC, HIGH, 30000UL);
  if (us == 0) return -1;         // nothing came back within range
  // Sound travels about 343 m/s, and the pulse makes a round trip.
  return (long)(us * 0.1715);
}

void ultrasonicTest() {
  Serial.println();
  Serial.println(F("Ultrasonic on D2, single-pin mode."));
  Serial.println(F("If every reading says NO ECHO, check that trigger and echo"));
  Serial.println(F("are commoned through a 1k resistor to D2."));
  Serial.println(F("Send any character to stop."));
  Serial.println();
  while (!Serial.available()) {
    long mm = readDistanceMm();
    if (mm < 0) {
      Serial.println(F("  NO ECHO  (nothing within range, or not wired)"));
    } else {
      Serial.print(F("  "));
      Serial.print(mm);
      Serial.print(F(" mm   "));
      drawBar(constrain(map(mm, 0, 2000, 0, 1023), 0, 1023));
      Serial.println();
    }
    delay(300);
  }
  while (Serial.available()) Serial.read();
  Serial.println(F("Stopped."));
}

// --- I2C ----------------------------------------------------------------------

void i2cScan() {
  Serial.println();
  Serial.println(F("Scanning the I2C bus on A4 (data) and A5 (clock)..."));
  uint8_t found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("  device at 0x"));
      if (addr < 16) Serial.print('0');
      Serial.print(addr, HEX);
      // Name the ones this project expects.
      if (addr >= 0x20 && addr <= 0x27) Serial.print(F("   PCF8574 expander"));
      if (addr == 0x38)                 Serial.print(F("   AHT20 temp/humidity"));
      if (addr == 0x5A)                 Serial.print(F("   MLX90614 thermometer"));
      if (addr == 0x23 || addr == 0x5C) Serial.print(F("   BH1750 lux"));
      Serial.println();
      found++;
    }
  }
  if (found == 0) {
    Serial.println(F("  nothing found."));
    Serial.println(F("  That is expected if you have not fitted any I2C parts yet."));
    Serial.println(F("  If you have: check power, ground, and that data and clock"));
    Serial.println(F("  are not swapped."));
  } else {
    Serial.print(F("  ")); Serial.print(found); Serial.println(F(" device(s)."));
  }
}

// --- Spare pins ----------------------------------------------------------------

void digitalTest() {
  Serial.println();
  Serial.println(F("Reading A4 and A5 as plain digital inputs."));
  Serial.println(F("Useful for checking a touch pad or cliff sensor before the"));
  Serial.println(F("expander is fitted. Send any character to stop."));
  Serial.println();
  pinMode(PIN_SPARE_A, INPUT);
  pinMode(PIN_SPARE_B, INPUT);
  while (!Serial.available()) {
    Serial.print(F("  A4 = ")); Serial.print(digitalRead(PIN_SPARE_A));
    Serial.print(F("    A5 = ")); Serial.println(digitalRead(PIN_SPARE_B));
    delay(250);
  }
  while (Serial.available()) Serial.read();
  Serial.println(F("Stopped."));
}

// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  Wire.begin();
  allMotorsOff();

  Serial.println();
  Serial.println(F("Sensor test bench ready."));
  Serial.print(F("Free memory: "));
  Serial.print(freeRam());
  Serial.println(F(" bytes of 2048"));
  menu();
}

void loop() {
  if (!Serial.available()) return;
  char c = Serial.read();
  if (c == '\n' || c == '\r' || c == ' ') return;

  switch (c) {
    case 'l': case 'L': liveLight();    break;
    case 'r': case 'R': rangeTest();    break;
    case 'm': case 'M': motorSweep();   break;
    case '1': runOne(1); allMotorsOff(); break;
    case '2': runOne(2); allMotorsOff(); break;
    case '3': runOne(3); allMotorsOff(); break;
    case '4': runOne(4); allMotorsOff(); break;
    case 'u': case 'U': ultrasonicTest(); break;
    case 'i': case 'I': i2cScan();      break;
    case 'p': case 'P': digitalTest();  break;
    case 'f': case 'F':
      Serial.print(F("Free memory: "));
      Serial.print(freeRam());
      Serial.println(F(" bytes of 2048"));
      break;
    case 'x': case 'X':
      allMotorsOff();
      Serial.println(F("All motors released."));
      break;
    case '?': menu(); break;
    default:
      Serial.print(F("Unknown command '"));
      Serial.print(c);
      Serial.println(F("'. Send ? for the menu."));
      break;
  }
}
