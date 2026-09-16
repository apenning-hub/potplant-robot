#include "drive.h"
#include "config.h"
#include "tuning.h"
#include <AFMotor.h>

// AFMotor is the library for this style of motor shield. The shield does not
// have one pin per motor - it uses a shift register chip to control four
// motors through only a few pins. The library hides all of that.
static AF_DCMotor motorLeft(MOTOR_LEFT_CHANNEL);
static AF_DCMotor motorRight(MOTOR_RIGHT_CHANNEL);

static int targetL = 0, targetR = 0;   // where we want to be
static int actualL = 0, actualR = 0;   // where we currently are

// Move a value towards a target, but by no more than DRIVE_SLEW_PER_TICK.
// This is what stops the robot lurching.
static int easeTowards(int current, int target) {
  int step = (int)DRIVE_SLEW_PER_TICK;
  if (target > current) {
    current += step;
    if (current > target) current = target;
  } else if (target < current) {
    current -= step;
    if (current < target) current = target;
  }
  return current;
}

// Push a speed out to one physical motor.
static void applyTo(AF_DCMotor &motor, int speed, bool invert) {
  if (invert) speed = -speed;

  if (speed == 0) {
    motor.setSpeed(0);
    motor.run(RELEASE);     // let the motor coast rather than braking hard
    return;
  }

  int magnitude = abs(speed);
  // These gear motors will not turn at all below a certain drive level, they
  // just sit there humming and getting warm. Anything above zero gets nudged
  // up to the minimum that actually moves them.
  if (magnitude < (int)DRIVE_MIN_SPEED) magnitude = DRIVE_MIN_SPEED;
  if (magnitude > 255) magnitude = 255;

  motor.setSpeed((uint8_t)magnitude);
  motor.run(speed > 0 ? FORWARD : BACKWARD);
}

void driveBegin() {
  targetL = targetR = actualL = actualR = 0;
  motorLeft.setSpeed(0);
  motorRight.setSpeed(0);
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
}

void driveSetTargets(int left, int right) {
  targetL = constrain(left,  -(int)DRIVE_MAX_SPEED, (int)DRIVE_MAX_SPEED);
  targetR = constrain(right, -(int)DRIVE_MAX_SPEED, (int)DRIVE_MAX_SPEED);
}

void driveUpdate() {
  actualL = easeTowards(actualL, targetL);
  actualR = easeTowards(actualR, targetR);
  applyTo(motorLeft,  actualL, MOTOR_LEFT_INVERT);
  applyTo(motorRight, actualR, MOTOR_RIGHT_INVERT);
}

void driveStop() {
  targetL = targetR = actualL = actualR = 0;
  applyTo(motorLeft,  0, MOTOR_LEFT_INVERT);
  applyTo(motorRight, 0, MOTOR_RIGHT_INVERT);
}

int driveLeft()  { return actualL; }
int driveRight() { return actualR; }
