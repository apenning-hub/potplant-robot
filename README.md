# Pot Plant Robot

A pot plant on a two-wheel chassis that roams an indoor room looking after
itself. It hunts for the light it prefers, wants attention, asks to be watered,
and rests when it is tired. It shows how it is feeling on an 8x8 LED face.

Eventually there are **two** of them running identical firmware - a sun-loving
jade and a shade-loving orchid - which behave completely differently because one
file of numbers differs. That is the point of the whole thing.

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
