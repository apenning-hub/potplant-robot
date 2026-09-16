# Photosynthetic Agency

A swarm of five mobile plants on two-wheel robotic platforms. Each carries its
own light and CO2 sensors and navigates autonomously, seeking the conditions it
needs to photosynthesise. The robots are prosthetic bodies, extending the plants'
capacity to sense and act.

All five run **identical firmware**. They behave differently because one file of
constants differs between them - which is the point.

Because indoor CO2 comes from human breath rather than from plants, a
CO2-seeking plant seeks out *people*. The approach is gas exchange, not
affection.

Five needs, each known a different way - one measured, one inferred, one
delegated to a human, one internal, and one with no biological basis at all.
See [`docs/concept.md`](docs/concept.md).

Written to be read by someone who has not done this before.

## Where things are

    firmware/potplant/    the code that runs on the Arduino
    docs/wiring.md        how to wire it up
    docs/design-notes.md  why things are shaped the way they are
    sim/                  a web page for tuning its personality (Phase 4)
    tools/                a face editor and a telemetry plotter (later phases)

## The firmware files

The whole program is split into small files, each with one job.

| File | What is in it |
|---|---|
| `potplant.ino` | The main file. Start up, then repeat forever. |
| `config.h` | Which wire goes to which pin. Hardware facts. |
| `tuning.h` | **The personality.** Every number that changes behaviour. |
| `sensors.h/.cpp` | Reading the light sensors and cleaning up the numbers. |
| `drive.h/.cpp` | Talking to the motors. |
| `telemetry.h/.cpp` | Printing what the robot is thinking. |

If you only ever open one file, make it `tuning.h`.

## Building and uploading

You can use either the Arduino IDE (click Upload) or the command line. Both read
the same files, so you can switch between them freely.

    arduino-cli compile --fqbn arduino:avr:uno firmware/potplant
    arduino-cli upload  --fqbn arduino:avr:uno -p /dev/cu.usbmodem113301 firmware/potplant

The port after `-p` changes depending on which USB socket you use. To find it:

    arduino-cli board list

### On an Apple Silicon Mac

Arduino's AVR compiler is still an Intel program, so it needs Rosetta:

    softwareupdate --install-rosetta --agree-to-license

This is a one-off. Without it you get `bad CPU type in executable`.

## Testing the hardware first

Before running the robot, there is a test bench. It is in two halves:

    firmware/sensor_test/   a small sketch that answers questions
    tools/bench.py          the actual test bench, which runs on your computer

Upload the sketch **once** - Arduino IDE, or `arduino-cli compile/upload` - then
leave it alone. Everything else lives in the Python, so changing how a test works
costs nothing: edit the file, run it again. No compiling, no uploading.

One-off setup:

    python3 -m pip install --user pyserial

Then:

    python3 tools/bench.py

It finds the board by itself. Pass a port if you need to override it.

| | Test | What it gives you |
|---|---|---|
| 1 | Live light | Four bars that move as you wave a torch, plus the steer and front/back figures the robot actually steers on |
| 2 | **Range test** | Cover and uncover each sensor for 20 s, and it prints `LIGHT_FLOOR` and `LIGHT_DEADBAND` values calculated from your hardware |
| 3 | **Motor identification** | Runs each channel in turn and asks which wheel moved, then prints the `config.h` lines |
| 4 | Ultrasonic | Live distance |
| 5 | I2C scan | Lists devices by address and names the ones this project expects |
| 6 | Spare pins | A4/A5 as digital, for testing a touch pad before the expander is fitted |
| 7 | Free memory | On the board |

Wheels off the ground before any motor test. Prop the chassis on a book.

### Why Python cannot run on the Arduino

It is a fair question. An ATmega328P has two kilobytes of memory - the Python
interpreter alone would not fit, by a wide margin. So the board is programmed in
C++ and *driven* from Python over the USB cable.

The split is deliberate. The board does only what must happen on the board:
reading pins, driving motors, timing an ultrasonic echo in microseconds.
Everything else - drawing bars, working out ranges, suggesting values, walking
you through the motors - is Python, where it can be changed in seconds.

### The deadman

While a motor test is running, the board stops the motors by itself if Python
goes quiet for more than 1.5 seconds. Close the window, pull the cable, crash the
script - the robot stops rather than driving off the bench. Worth knowing the
safety is in the firmware, not in the script that might be the thing that failed.

## Watching it think

Once uploaded, the robot reports itself over the USB cable. Open the Serial
Monitor in the Arduino IDE, or:

    arduino-cli monitor -p /dev/cu.usbmodem113301 -c baudrate=115200

**Set the speed to 115200**, or you will see nothing but rubbish characters.

Lines starting with `#` are notes. Every other line is one comma separated
reading:

    ms,ldrFL,ldrFR,ldrBR,ldrBL,steer,fb,dist,pir,touch,light,warmth,affection,water,rest,spdL,spdR,state,mood

`steer` compares the two sides and `fb` compares front against back. Columns for
things that do not exist yet are left empty. The format is fixed from here on, so
any tool written against it keeps working.

The Arduino IDE's **Serial Plotter** draws these as live graphs, which is by far
the easiest way to see whether a sensor is working.

## Tuning a personality

Open `tuning.h`. Change a number. Upload. Watch.

Some good first experiments:

- `DRIVE_BASE_SPEED` - how fast it cruises. Try 70 for something cautious.
- `STEER_GAIN_PERCENT` - how eagerly it turns towards light. Raise it to 120 and
  it will overshoot and weave about. Drop it to 30 and it drifts lazily.
- `LIGHT_DEADBAND` - how big a difference it bothers reacting to. Larger means
  calmer and more decisive; smaller means fidgety.
- `DRIVE_SLEW_PER_TICK` - how quickly it is allowed to change speed. Small
  numbers give a smooth, heavy, dignified movement.

## Phases

1. **Light seeker** - motors and four corner light sensors. *(built)*
2. Obstacles, **cliff detection**, the needs model, and the LED face.
   The light need seeks a *target* brightness from the start, not a maximum.
3. Affection - touch and motion. A pat is worth far more than company.
4. Water, and the personality simulator - now showing two plants side by side.
5. The I2C sensor bus - humidity, temperature, real lux, UV.
6. Two chassis. A jade and an orchid, same binary, two `tuning.h` files.
7. The orchid's flowering quest - a need measured in weeks.

See [`docs/design-notes.md`](docs/design-notes.md) for why the phases are shaped
this way, and what was deliberately rejected.
