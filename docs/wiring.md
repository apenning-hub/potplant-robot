# Wiring - Phase 1

Only three things are wired in Phase 1: the shield, two motors, and three light
sensors. Everything else on the pin map is for later phases.

## Before you start

Unplug the USB cable. Do all wiring with no power connected.

## 1. The motor shield

The Funduino shield pushes down onto the UNO so that every pin lines up. It only
fits one way round - the two long pin headers are different lengths, so if it
seems to need forcing, it is the wrong way round.

Once it is on, the shield has taken these pins for itself and you must not use
them for anything else:

    D3  D4  D5  D6  D7  D8  D11  D12

## 2. The motors

Two screw terminal blocks on the shield, labelled **M1** and **M2**.

    Left motor   two wires   ->  M1
    Right motor  two wires   ->  M2

It does not matter which wire goes in which side of a terminal. If a wheel ends
up spinning backwards, do not swap the wires - open `config.h` and change
`MOTOR_LEFT_INVERT` or `MOTOR_RIGHT_INVERT` to `true`. That way the code and the
hardware always agree with each other.

## 3. The light sensors

Three LDRs across the front of the pot, looking left, straight ahead, and right.
Angle them apart - maybe 45 degrees between each - or all three see the same
thing and the robot cannot tell which way to turn.

              (front of pot)

         \          |          /
          \         |         /
        LEFT     CENTRE     RIGHT
         A0        A1         A2

Each LDR needs a fixed resistor to go with it. On its own an LDR is just a
resistor that changes with light, and the Arduino cannot measure resistance - it
can only measure voltage. Pairing it with a fixed resistor makes a **voltage
divider**, and the voltage in the middle moves as the light changes.

Build this three times, once per sensor:

      5V ──────┐
               │
              LDR
               │
               ├──────────────>  to A0 (then A1, then A2)
               │
             10k resistor
               │
              GND

With the LDR on the top leg like this, **more light means a higher number**.

If you have already built them the other way round, with the LDR at the bottom,
leave them. The code works either way - it asks you to cover the sensors at
start-up and works out which way round they are by itself.

A 10k resistor is the usual choice. If your readings sit nearly at 0 or nearly
at 1023 all the time, try 4.7k or 22k instead.

## 4. Power - read this part carefully

Phase 1 can run on USB alone with the wheels off the ground. Once you want it on
the floor, the motors need their own batteries.

The 4xAA holder goes to the terminal block marked **EXT_PWR**, red to +, black to -.

Next to it is a jumper marked **PWR**. This is the one that matters:

| PWR jumper | What it means |
|---|---|
| **Fitted** | Motors draw power from the Arduino's own 5V supply |
| **Removed** | Motors draw power only from EXT_PWR |

**When USB is plugged in, the PWR jumper must be OFF.** If it is fitted while a
battery pack is attached, battery voltage is pushed onto the Arduino's 5V rail
and back down the USB cable into your Mac. Take the jumper off and keep it
somewhere safe - you will not need it for this project.

So the rule is: **jumper off, batteries on EXT_PWR, USB for the brain.**

## Pin budget

Every pin on the UNO, accounted for. Phase 1 uses the ones marked "now".

| Pin | Use | When |
|---|---|---|
| A0 | LDR front-left | now |
| A1 | LDR front-right | now |
| A2 | LDR back-right | now |
| A3 | LDR back-left | now |
| A4 | I2C data (SDA) | Phase 3 |
| A5 | I2C clock (SCL) | Phase 3 |
| D2 | Ultrasonic, single-pin mode | Phase 2 |
| D9 | LED matrix DIN | Phase 2 |
| D10 | LED matrix CS | Phase 2 |
| D13 | LED matrix CLK | Phase 2 |
| D3-D8, D11, D12 | taken by the shield | always |
| D0, D1 | USB serial - never use | always |
| M1, M2 | left and right motors | now |

That is the lot for Phase 1 to 3.

Phases 4 onward need more: two downward cliff sensors, humidity, temperature,
real lux and UV. There is no spare pin for any of them.

The likely answer is **not** a bigger board. Nearly all of those sensors speak
I2C, and I2C is a bus - several sensors share the same two wires, A4 and A5. A
small expander chip hands back the pins those two currently use. See
[`design-notes.md`](design-notes.md) for the working.

If it does come to a Mega, the shield stacks on one unchanged, and because every
pin assignment lives in `config.h`, that move is a one-file change.
