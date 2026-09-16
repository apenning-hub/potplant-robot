// ===========================================================================
// SENSOR TEST BENCH - FIRMWARE SIDE
//
// This sketch is deliberately stupid. It answers questions and does nothing
// else. All the thinking - drawing bars, working out ranges, suggesting tuning
// values, walking you through identifying the motors - happens in Python, in
// tools/bench.py on your computer.
//
// The reason for splitting it this way: changing how a test WORKS then costs
// nothing. Edit the Python, run it again. No compiling, no uploading, no
// waiting. Only the parts that must live on the board live on the board.
//
// Python itself cannot run on an Arduino - an ATmega328P has two kilobytes of
// memory and the interpreter alone would not fit. So the board is programmed
// in C++ and driven from Python over the USB cable.
//
// Upload this once, then use tools/bench.py.
//
// Uses the Adafruit Motor Shield library (AFMotor.h) for the v1 shield.
//
// ---------------------------------------------------------------------------
// PROTOCOL
//
// One command per line in. One or more lines out, machine readable.
//
//   V          -> VER,potplant-bench,2
//   L          -> LDR,<a0>,<a1>,<a2>,<a3>
//   U          -> DIST,<mm>          (-1 means no echo)
//   P          -> DIG,<a4>,<a5>
//   I          -> I2C,<addr> ... then I2C,END
//   R          -> RAM,<bytes free>
//   M<n><d>    -> MOT,<n>,<d>,OK     n = 1..4, d = f|b
//   S          -> STOP,OK
//
// Anything unrecognised gives ERR,<char>.
// ===========================================================================

#include <AFMotor.h>
#include <Wire.h>

const uint8_t PIN_LDR[4]     = { A0, A1, A2, A3 };
const uint8_t PIN_ULTRASONIC = 2;
const uint8_t PIN_SPARE_A    = A4;
const uint8_t PIN_SPARE_B    = A5;

// All four channels, so Python can walk you through finding which is which.
// Channels 1 and 2 are given 64 kHz, above hearing. Channels 3 and 4 ignore the
// frequency argument and run near 976 Hz - you will hear them whine, which is
// exactly why the robot uses M1 and M2.
AF_DCMotor motor1(1, MOTOR12_64KHZ);
AF_DCMotor motor2(2, MOTOR12_64KHZ);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

const uint8_t  TEST_SPEED   = 140;
const unsigned DEADMAN_MS   = 1500;   // motors stop if Python goes quiet

static unsigned long motorStartedAt = 0;
static bool motorRunning = false;

int freeRam() {
  extern int __heap_start, *__brkval;
  int here;
  return (int)&here - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

AF_DCMotor *channel(uint8_t n) {
  switch (n) {
    case 1:  return &motor1;
    case 2:  return &motor2;
    case 3:  return &motor3;
    default: return &motor4;
  }
}

void stopAll() {
  for (uint8_t n = 1; n <= 4; n++) { channel(n)->run(RELEASE); channel(n)->setSpeed(0); }
  motorRunning = false;
}

// Trigger and echo share one pin, commoned through a 1k resistor. The pin is an
// output for the pulse, then an input for the echo that comes back.
long readDistanceMm() {
  pinMode(PIN_ULTRASONIC, OUTPUT);
  digitalWrite(PIN_ULTRASONIC, LOW);
  delayMicroseconds(3);
  digitalWrite(PIN_ULTRASONIC, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_ULTRASONIC, LOW);
  pinMode(PIN_ULTRASONIC, INPUT);
  unsigned long us = pulseIn(PIN_ULTRASONIC, HIGH, 30000UL);
  if (us == 0) return -1;
  return (long)(us * 0.1715);   // round trip, sound at roughly 343 m/s
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  Wire.begin();
  stopAll();
  Serial.println(F("VER,potplant-bench,2"));
}

void loop() {
  // Deadman. If Python stops talking - crashed, window closed, cable pulled -
  // the motors stop by themselves rather than driving the robot off the bench.
  if (motorRunning && millis() - motorStartedAt > DEADMAN_MS) {
    stopAll();
    Serial.println(F("STOP,DEADMAN"));
  }

  if (!Serial.available()) return;
  char c = Serial.read();
  if (c == '\n' || c == '\r' || c == ' ') return;

  switch (c) {
    case 'V':
      Serial.println(F("VER,potplant-bench,2"));
      break;

    case 'L': {
      Serial.print(F("LDR"));
      for (uint8_t i = 0; i < 4; i++) { Serial.print(','); Serial.print(analogRead(PIN_LDR[i])); }
      Serial.println();
      break;
    }

    case 'U':
      Serial.print(F("DIST,"));
      Serial.println(readDistanceMm());
      break;

    case 'P':
      pinMode(PIN_SPARE_A, INPUT);
      pinMode(PIN_SPARE_B, INPUT);
      Serial.print(F("DIG,"));
      Serial.print(digitalRead(PIN_SPARE_A)); Serial.print(',');
      Serial.println(digitalRead(PIN_SPARE_B));
      break;

    case 'I':
      for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
          Serial.print(F("I2C,")); Serial.println(addr);
        }
      }
      Serial.println(F("I2C,END"));
      break;

    case 'R':
      Serial.print(F("RAM,"));
      Serial.println(freeRam());
      break;

    case 'S':
      stopAll();
      Serial.println(F("STOP,OK"));
      break;

    case 'M': {
      // Two more characters follow: channel number, then f or b.
      unsigned long waitUntil = millis() + 200;
      while (Serial.available() < 2 && millis() < waitUntil) { ; }
      if (Serial.available() < 2) { Serial.println(F("ERR,M")); break; }
      char nc = Serial.read();
      char dc = Serial.read();
      uint8_t n = nc - '0';
      if (n < 1 || n > 4 || (dc != 'f' && dc != 'b')) { Serial.println(F("ERR,M")); break; }

      stopAll();
      AF_DCMotor *m = channel(n);
      m->setSpeed(TEST_SPEED);
      m->run(dc == 'f' ? FORWARD : BACKWARD);
      motorRunning = true;
      motorStartedAt = millis();

      Serial.print(F("MOT,")); Serial.print(n);
      Serial.print(','); Serial.print(dc);
      Serial.println(F(",OK"));
      break;
    }

    default:
      Serial.print(F("ERR,"));
      Serial.println(c);
      break;
  }
}
