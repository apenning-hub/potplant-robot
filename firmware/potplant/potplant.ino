// ===========================================================================
// POT PLANT ROBOT - PHASE 1: LIGHT SEEKER
//
// A pot plant on wheels that drives towards the brightest light it can see.
// No moods, no needs, no face yet - this phase exists to prove two things:
// that the motors do what we expect, and that the light sensors give sensible
// numbers. Everything later is built on top of those two facts.
//
// How it works, once per tick (20 times a second):
//   1. read the four corner light sensors
//   2. compare the two sides, and the front against the back
//   3. steer towards the brighter side - or turn round if the light is behind
//   4. occasionally report what it is doing over USB
//
// There is no delay() in the loop. delay() means "freeze completely", and a
// frozen robot cannot notice a table leg. Instead we watch the clock with
// millis() and act when enough time has passed. See tuning.h for TICK_MS.
//
// Files:
//   config.h      which wire goes to which pin
//   tuning.h      the personality numbers - this is the one to play with
//   sensors.*     reading and cleaning up the light sensors
//   drive.*       talking to the motors
//   telemetry.*   printing what it is thinking
// ===========================================================================

#include "config.h"
#include "tuning.h"
#include "sensors.h"
#include "drive.h"
#include "telemetry.h"

// The three things the robot can be doing in Phase 1.
enum State {
  STATE_SEEK,     // it can see a light ahead, and is driving towards it
  STATE_TURN,     // the light is behind it, so it is turning round
  STATE_SEARCH    // too dark to tell, so turn slowly and look around
};

static State state = STATE_SEARCH;
static unsigned long lastTick = 0;
static uint8_t tickCounter = 0;

// Used by STATE_SEARCH to decide which way to spin, and when to give up and
// try the other direction.
static unsigned long searchStarted = 0;
static int searchDirection = 1;

// Used by STATE_TURN: which way it chose to spin, and when it started, so it
// can give up rather than spinning for ever in an evenly lit room.
static int turnDirection = 1;
static unsigned long turnStarted = 0;

// How much RAM is left over. The UNO has 2048 bytes in total and it runs out
// long before the 32 KB of program space does, so this number is worth
// watching as the project grows.
static int freeRam() {
  extern int __heap_start, *__brkval;
  int here;
  return (int)&here - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial) { ; }   // only matters on boards with native USB

  Serial.println();
  Serial.println(F("# pot plant robot - phase 1, light seeker, four corner sensors"));
  Serial.print(F("# free RAM at start-up: "));
  Serial.print(freeRam());
  Serial.println(F(" bytes of 2048"));

  driveBegin();
  driveStop();            // make very sure nothing is moving while we calibrate

  sensorsBegin();
  sensorsCalibrate();     // blocking, but this runs before the loop starts

  telemetryHeader();

  searchStarted = millis();
  lastTick = millis();
}

// Work out motor speeds from a set of light readings.
static void seekLight(const LightReading &light) {
  // steer is positive when the right-hand pair see more light than the left.
  int error = light.steer;

  // Small differences are just noise. Ignore them, or the robot weaves.
  if (abs(error) < LIGHT_DEADBAND) error = 0;

  // Turn the brightness difference into a steering amount.
  int steer = ((long)error * STEER_GAIN_PERCENT) / 100;

  // Differential steering: to turn right, slow the right wheel and speed up
  // the left one. The two wheels always add up to roughly the same total, so
  // it curves rather than stopping and pivoting.
  int left  = (int)DRIVE_BASE_SPEED + steer;
  int right = (int)DRIVE_BASE_SPEED - steer;

  driveSetTargets(left, right);
}

// Spin on the spot towards whichever side the light is on, until it is in front.
static void turnRound() {
  driveSetTargets( (int)TURN_SPEED * turnDirection,
                  -(int)TURN_SPEED * turnDirection );
}

// Spin slowly on the spot, reversing every so often, hunting for any light.
static void searchForLight() {
  if (millis() - searchStarted > SEARCH_SWEEP_MS) {
    searchDirection = -searchDirection;
    searchStarted = millis();
  }
  driveSetTargets( (int)SEARCH_SPEED * searchDirection,
                  -(int)SEARCH_SPEED * searchDirection );
}

void loop() {
  // Has a tick's worth of time passed? If not, fall straight out of loop()
  // and come back in again. The robot stays responsive the whole time.
  if (millis() - lastTick < TICK_MS) return;
  lastTick += TICK_MS;

  sensorsUpdate();
  LightReading light = sensorsLight();

  // Decide which state we are in.
  //
  // Too dark to steer by comes first - without useful light, nothing else is
  // worth deciding. There is deliberately a gap between the threshold for
  // giving up and the one for starting again, so a reading sitting right on the
  // boundary does not flip the robot between states several times a second.
  if (light.brightest < LIGHT_FLOOR) {
    if (state != STATE_SEARCH) {
      state = STATE_SEARCH;
      searchStarted = millis();
    }
  } else if (state == STATE_SEARCH && light.brightest > LIGHT_FLOOR + LIGHT_DEADBAND) {
    state = STATE_SEEK;
  }

  if (state == STATE_SEEK && light.frontBack < -LIGHT_BEHIND_THRESHOLD) {
    // The back pair are clearly beating the front pair - the light is behind.
    // Turn towards whichever side is brighter, so it takes the shorter way
    // round rather than always spinning the same direction.
    state = STATE_TURN;
    turnDirection = (light.steer >= 0) ? 1 : -1;
    turnStarted = millis();
  } else if (state == STATE_TURN) {
    bool nowInFront = light.frontBack > LIGHT_AHEAD_THRESHOLD;
    bool givenUp    = millis() - turnStarted > TURN_TIMEOUT_MS;
    if (nowInFront || givenUp) state = STATE_SEEK;
  }

  switch (state) {
    case STATE_SEEK:   seekLight(light);   break;
    case STATE_TURN:   turnRound();        break;
    case STATE_SEARCH: searchForLight();   break;
  }

  driveUpdate();

  if (++tickCounter >= TELEMETRY_EVERY_N_TICKS) {
    tickCounter = 0;
    const char *name = "SEARCH";
    if (state == STATE_SEEK) name = "SEEK";
    else if (state == STATE_TURN) name = "TURN";
    telemetryLine(light, name, "none");
  }
}
