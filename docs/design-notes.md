# Design notes

Decisions made while workshopping, and the reasoning behind them. This is the
document to read before starting a new phase. It records *why* things are shaped
the way they are, which the code itself cannot tell you.

---

## Two plants, one binary

The project builds **two robots running byte-identical firmware**, differing only
in their copy of `tuning.h`.

| | Plant A | Plant B |
|---|---|---|
| Species | Jade (*Crassula ovata*) | Moth orchid (*Phalaenopsis*) |
| Wants | Full sun, dry, warm | Bright shade, humid, cool nights |
| Temperament | Bold, sociable, quick | Fussy, retiring, slow |

They are deliberately opposite extremes of houseplant care, so nearly every
constant differs. Watching them share a room is the argument for the whole
project: same code, different numbers, visibly different creatures.

Use plastic pots, not terracotta. The TT gear motors have limited payload and a
jade in a terracotta pot can exceed a kilogram before soil.

---

## The light need seeks a target, not a maximum

**This is the most important structural decision, and it must be built in from
Phase 2.**

The obvious model is "drive towards the brightest sensor". That cannot express an
orchid. A *Phalaenopsis* is an epiphyte - in the wild it grows on tree trunks
under the canopy - and direct sun scorches its leaves. It does not want maximum
light. It wants roughly 12,000 lux, and both more and less are wrong.

So the model is **"get to your preferred brightness"**:

- below target, climb the gradient towards light
- above target, climb down it, into shade
- within tolerance, the need is satisfied and something else takes over

Seeking and fleeing become the same code path, and which way the robot turns
falls out of whether it is currently above or below its own target. A sun-lover
has a target so high it can rarely be reached, so it seeks light nearly always -
which is the Phase 1 behaviour, arrived at honestly rather than special-cased.

This also makes the light need work the same way the warmth need already does, as
a comfort band. The model gets more consistent, not less.

**Why it must be early:** "go to brightest" and "go to preferred" are different
shapes of code. Building the first and swapping later means rewriting the
behaviour, the face logic and the simulator. Building the second now costs
nothing - it uses raw LDR units until a real lux sensor arrives in Phase 5, and
then only the meaning of the numbers changes.

Get the shape right early, swap the units later.

---

## Needs are switched off by numbers, never by code

`COLD_NIGHTS_NEEDED = 0` means the jade simply never develops that need. The
same compiled binary runs on both chips. If a need has to be removed with an
`#ifdef`, the design is wrong.

---

## Affection: company counts, a little

Two robots in one room will detect each other on the ultrasonic sensor. That is
unavoidable, so it should mean something.

**Being near something that is not blocking the path trickles affection back,
slowly.** A pat on the touch pad is worth far more.

Known limitation, accepted deliberately: **the robot cannot tell another plant
from a chair leg.** The ultrasonic returns a distance, not an identity. So a
lonely plant will take comfort from furniture. This is a feature.

One distinction does come free: the PIR only fires for warm moving bodies.

- ultrasonic **and** PIR -> a person, worth a lot
- ultrasonic **without** PIR -> an object or another plant, worth a little

### Rejected for now: the face as a beacon

Both plants carry a bright 8x8 red matrix. A plant could blink a recognisable
pattern and watch for another answering - real recognition, no new hardware. It
is fragile in a bright room and fights with the light sensing. Worth trying as an
experiment once the rest is solid, not before.

---

## Containment

### Cliff detection is not optional

Stairs will destroy this robot. Two downward-facing IR reflectance sensors at the
front, and like the obstacle check it is an **override that interrupts whatever
is happening**, not a need that competes for attention. This belongs in Phase 2,
because it is obstacle avoidance pointed downward.

### Cliff sensors and line sensors are the same hardware

A downward IR sensor asks one question: how reflective is the thing below me?

- nothing below at all -> a cliff
- black tape below -> a boundary

Same two sensors, same two pins, different threshold. Boundary marking therefore
costs nothing once cliff detection exists.

### Boundaries, in order of preference

1. **Floor tape.** Free, given the cliff sensors. Utterly reliable. Ugly at home,
   fine in a studio.
2. **Virtual wall beacon.** A small battery-powered IR emitter in a doorway
   shining a beam across it; the robot carries a cheap IR receiver and turns
   away. This is what a Roomba does. Invisible when off, portable, one per
   doorway. Costs a second small circuit and needs line of sight.

### Rejected: wheel encoders

Counting wheel rotations to track position drifts uselessly within minutes on
carpet, where the wheels slip. Not worth the pins.

### Later possibility

Point a beacon at a home base and the robot can find its way back, which makes
homesickness possible as a need, and eventually a charging dock.

---

## Sensors for an orchid

An orchid breaks two assumptions in the original brief.

**A soil moisture probe will not work on it.** Orchids live in chunky bark, not
soil. A capacitive probe in bark reads the air gap between two chips. Phase 5
should not assume a soil probe for Plant B.

Better answers for an orchid's thirst:

- **A load cell under the pot (HX711).** How experienced growers actually check:
  they pick it up. A dry bark pot is dramatically lighter. Only reads reliably
  when stationary - which is often, since the robot rests and parks.
- **A colour sensor aimed at the aerial roots.** Orchid roots genuinely change
  colour, silvery-white when thirsty and bright green when watered. The plant's
  own indicator. Hardest to execute, because aerial roots wander.

**It needs cold nights to flower.** A *Phalaenopsis* spikes after two to four
weeks of nights near 15-18 C against days near 25 C. This is a need on a
timescale of *weeks*, which nothing else in the model has, and it makes the
orchid hunt for a cold windowsill every evening in autumn for a reward that
arrives a month later.

### Worth adding

| Sensor | Chip | Gives |
|---|---|---|
| Humidity + temperature | AHT20 or SHT31 | Orchids want 50-70% RH; Adelaide is dry |
| Real lux | BH1750 or VEML7700 | Absolute light, so targets can be in real units |
| UV | LTR390 | Tells direct sun from a bright wall - what makes fleeing possible |
| Weight | HX711 + load cell | Thirst, for plants in bark |

The LDRs stay regardless. They are fast and give a left-right gradient for
steering; a lux sensor gives an accurate absolute reading but a narrow view.
Different jobs.

---

## Phase 5 is probably I2C, not a Mega

Nearly every sensor above speaks **I2C**, and I2C is a bus - humidity, lux, UV,
a colour sensor and a clock can all share the same two wires. Five sensors, two
pins.

Those two pins are A4 and A5, currently allocated to the PIR and the touch pad.
A small I2C GPIO expander hands those back with pins to spare, on the same bus.

So the Phase 5 conversation is likely not "buy a bigger board" but "here is how a
sensor bus works" - which is a better thing for students to learn, and cheaper.

**Still to check:** cliff sensors are safety critical and an expander adds polling
latency. At a 20 Hz tick the robot moves only a few millimetres per tick, so it
is probably fine, but cliff sensors should get direct pins if any are spare.

---

## Four light sensors, at the corners

Settled: **four LDRs, one at each corner** - front-left, front-right, back-right,
back-left - rather than three facing forward.

The reason is that three forward-facing sensors cannot tell a dark room from a
window behind them. Both read as "dim ahead". With four, the robot knows light is
*behind* it and can turn round, which matters a great deal for a machine whose
entire first behaviour is finding light.

This is the arrangement the original Tech Talkies sketch used, and it was the
right call.

### The pin map this forces

Four LDRs take A0 to A3, which leaves A4 and A5 as the only analogue pins - and
those are the I2C pins. So the sensor bus is no longer a Phase 5 idea to consider.
It is the only way the rest of the project fits.

| Pin | Carries |
|---|---|
| A0, A1, A2, A3 | LDR front-left, front-right, back-right, back-left |
| A4, A5 | **I2C bus** - humidity, lux, UV, and the GPIO expander |
| D2 | Ultrasonic, **single-pin mode** |
| D9, D10, D13 | LED matrix DIN, CS, CLK |
| D3-D8, D11, D12 | Motor shield |
| D0, D1 | USB serial, never use |
| Expander pin 0, 1 | Cliff sensor left, right |
| Expander pin 2 | PIR motion |
| Expander pin 3 | Touch pad |
| Expander pin 4-7 | Spare |

### Two things that make this work

**The ultrasonic runs on one pin, not two.** An HC-SR04 normally wants a separate
trigger and echo, and there is no second pin to give it. Trigger and echo can be
tied together through a 1k resistor and driven from a single pin; the NewPing
library supports this directly. Without that trick the map does not close.

**The echo pin cannot go on the expander.** The measurement *is* the width of the
echo pulse, timed in microseconds. An I2C round trip destroys it. Anything
requiring microsecond timing needs a real pin - which is why the ultrasonic gets
D2 and the slow digital sensors get the expander.

### A safety consequence: cliff detection must bypass the slew limiter

The slew limiter eases motor speed by `DRIVE_SLEW_PER_TICK` each tick so the
gearboxes are not slammed. From cruising speed that is roughly seven ticks, about
350 ms, to reach a stop - during which the robot travels something like five
centimetres.

That is enough to carry it over the edge of a step.

So the cliff override must **set the actual motor speed to zero immediately**,
not ask the slew limiter to ease towards it. Graceful movement is for ordinary
behaviour. An emergency stop is not ordinary behaviour.

The polling latency of reading the cliff sensors over I2C is negligible by
comparison - well under a millisecond - so putting them on the expander is fine.
The tick rate and the slew limiter are the real risks, not the bus.
