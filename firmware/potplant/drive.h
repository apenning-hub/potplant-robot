#ifndef DRIVE_H
#define DRIVE_H

#include <Arduino.h>

// ---------------------------------------------------------------------------
// drive.h - moving
//
// The rest of the code never talks to the motor library directly. It just says
// "I would like to be going this fast on the left and this fast on the right"
// and this file works out how to get there smoothly.
//
// Speeds are signed: -255 is full reverse, 0 is stopped, 255 is full forward.
// ---------------------------------------------------------------------------

void driveBegin();
void driveSetTargets(int left, int right);  // what we WANT to be doing
void driveUpdate();                          // call once per tick; eases towards it
void driveStop();                            // immediate, no easing

int driveLeft();    // what the motors are ACTUALLY being given right now
int driveRight();

#endif
