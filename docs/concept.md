# Photosynthetic Agency: Plants, Machines and the Anthropocene

The project statement, and the technical findings that bear on it.

---

## What this is

A swarm of five mobile plants on small robotic platforms. Each carries its own
light and CO2 sensors and navigates autonomously, seeking conditions favourable
to photosynthesis. Rather than passive decorative objects, the plants become
active participants whose biological needs drive movement and spatial occupation.
The robots function as prosthetic bodies, extending the plants' capacity to
sense, explore and respond to their environment.

Situated in debates around the Anthropocene, the project asks how design might
move beyond treating nature as a passive resource to be managed, and imagines
architectures that negotiate with living systems rather than containing them.

**Scope: the swarm only.** An intelligent wall was considered as a second,
environment-modulating system and deliberately cut. See "Why there is no wall".

---

## Finding: indoor CO2 comes from people, not plants

Room CO2 sits near 400 ppm when empty and climbs to 1,000-1,500 ppm with people
in it. Plants do not measurably shift it at room scale. Humans do - we exhale it.

**So a CO2-seeking plant robot seeks out humans.** Not because it has been
programmed to want attention, but because we are the food source.

This is the strongest image the project has. It inverts the relationship the work
is arguing about: the plant is neither decorative nor dependent on our care. It
is harvesting us. The approach behaviour is gas exchange, not sentiment - and it
is biologically true rather than a metaphor imposed on the machine.

It also collapses two needs into one. Affection and CO2 become a single drive
with a real physiological basis, which is a tighter argument than treating a
desire for company and a desire for carbon as separate things.

---

## Finding: photosynthesis is a negotiation, not a gradient

Light alone does not photosynthesise. CO2 alone does not either. The plant needs
the *intersection*, and indoors the two rarely coincide - the brightest spot is
the window with nobody near it, the CO2-rich spot is the dark corner where people
sit and talk.

So the robot cannot simply climb a gradient. It has to trade one against the
other, and each plant weights that trade differently through its tuning file.
The negotiation with living systems happens in the movement itself rather than
in the wall text.

### A detail worth using

Orchids and other CAM plants take in CO2 at **night**, keeping their stomata shut
during the day to conserve water. A CAM plant in the swarm would hunt people
after dark and light during the day, on an inverted schedule - visibly different
behaviour arising from real biology, with no code change beyond its constants.

---

## Warning: most cheap "CO2 sensors" do not measure CO2

This matters for work that will be examined and critiqued.

- **MQ-135** (~$3), sold widely as a CO2 sensor, is a tin-oxide sensor that
  responds to alcohol, smoke and VOCs generally. The CO2 figure is inferred from
  a relationship that does not hold indoors.
- **CCS811** and **SGP30** report "eCO2" - *equivalent* CO2, estimated from VOC
  readings. More honest about it, still not a measurement.

For a project whose intellectual claim rests on plants responding to actual
photosynthetic conditions, a sensor that responds to hand sanitiser is a real
vulnerability.

Genuine CO2 sensing means NDIR or photoacoustic, and the floor is about $20-30:

| Part | Approx | Notes |
|---|---|---|
| **SCD40 / SCD41** | $25-30 | I2C, small, photoacoustic. The one to pick. |
| MH-Z19C | $20-25 | UART, genuine NDIR, physically larger |

---

## Why there is no wall

The original proposal paired the swarm with an intelligent wall that modulated
apertures, light and atmosphere, with the two systems forming a feedback loop.

Cut, and the argument is better for it. With a wall, a designed architectural
element mediates between the plants and the space, and a critic can reasonably
say the agency still belongs to whoever designed the wall. Without it the plants
*are* the architecture.

### The loop still closes

humans breathe -> plants converge on people -> plants compete for and occupy the
good positions -> their occupation changes what is available to the others ->
their clustering changes how people move through the room

That last step is the architectural claim: **non-human actors reorganising human
spatial behaviour.** No wall required.

### Do not let them talk to each other

The tempting solution for swarm behaviour is radio modules and shared state.
Refuse it. Let the plants influence each other **only through the environment** -
by occupying a spot, by shading one another, by crowding the same person.

Intelligence that is genuinely distributed is a stronger version of the claim
than intelligence that is centrally coordinated and described as distributed.

**Honesty note:** this loop closes properly for *light and occupation*. It is
physically weak for CO2 - five small plants will not measurably change a room's
ppm. Any claim about the swarm altering the atmosphere would not survive
measurement. Their effect is spatial, not chemical, and the work should say so.

---

## Inference over instrumentation

**Decision: the swarm infers the presence of people from cheap, unreliable
sensors rather than measuring CO2 with an NDIR instrument.**

This is a position, not a budget compromise, and the work should say so.

A real orchid does not own a $30 photoacoustic instrument. It has cheap, noisy,
evolved heuristics - stomatal conductance, water potential, light quality - and
infers its way to a decent guess. Building the robot the same way is better
biomimicry than giving a plant laboratory-grade instrumentation. Buy the NDIR
sensor and the plant knows something no plant knows.

**The robot infers people, not ppm.** That distinction should be stated openly.
Since indoor CO2 comes from human breath, a reliable people-detector is a usable
CO2 proxy - and the gap between the proxy and the thing itself is where the
interest lies.

### The proxies, and what each is honestly worth

| Sensor | ~Cost | Signal | Honest limit |
|---|---|---|---|
| **MLX90614** IR thermometer | $6 | A person is warmer than a wall | **Short range only.** A person at 3 m fills a fraction of the field of view and averages out against the background. Reliable under a metre. |
| **AHT20** temp + humidity | $2 | Exhaled breath is warm and humid; local RH rises near people | Slow - a minute or more to register. Reports on *here*, not *there*. |
| Sound module | $1 | Talking means people, and talking people breathe hard | Not directional. A television is indistinguishable from a conversation. |
| Four LDRs (already fitted) | - | A shadow crossing a sensor means something passed | Very crude |
| Ultrasonic (already fitted) | - | Something is at 60 cm | Does not say what |

About **$9 per robot, $45 for five**, against roughly $150 for NDIR.

Both the MLX90614 and the AHT20 are I2C and join the bus already reserved on
A4/A5. The sound module must use its **digital** output on the expander, since
every analogue pin is taken by the LDRs - which costs amplitude and keeps
presence. Counting pulses over time is enough to answer "is there conversation
here", which is the only question being asked of it.

### The one fusion that genuinely works

**Ultrasonic says something is close. IR says it is warm. Close plus warm is a
person.** A reliable person-detector under a metre, from two cheap parts.

### The behavioural consequence

**None of these proxies are directional.** They report on *here*, never on
*which way*. So the robot cannot climb a CO2 gradient the way it climbs a light
gradient.

The behaviour therefore has to be **wander, evaluate, and stay where it is
good** - occupy and persist rather than pursue.

This is far more plant-like than a robot driving purposefully at a human. Plants
do not walk towards things; they grow where conditions suit and die back where
they do not. The crude sensing forces a truer behaviour than the expensive
sensing would have. The limitation improved the work.

### Aggregation without communication

Five robots with bad sensors, spread through a room, should in principle average
out into good data. But aggregation normally requires the robots to share what
they know - and distributed intelligence routed through a radio network has a
coordinator, which is precisely the claim this project is trying not to make.

**The resolution: the swarm's physical arrangement is the aggregated data.**

No robot holds the map. Each wanders, evaluates locally with unreliable sensors,
and stays where conditions are good. After an hour, where the plants are standing
*is* the room's CO2 map - readable by anyone who walks in, held in no memory,
computed by nobody.

Five bad sensors, averaged across space and time, rendered as an arrangement of
objects. Aggregation without communication.

### Keep the false positives

These proxies will be wrong. A radiator reads as a person. A sunlit patch reads
warm. The television talks.

Do not engineer this out. A plant that has mistaken a radiator for a human and
settled down hopefully beside it makes the inference visible, and is more honest
than a robot that always gets it right.

---

## The fifth need has no biological basis

A recording, *Plantsia*, plays continuously in the installation. Units carry a
need for it and will settle near the speaker and remain there.

**Light, carbon, water and rest are physiology. Music is a belief.** The model
does not distinguish between them, and that is the point. A unit weighing its
need for music against its need for light is weighing a fact against a wish, and
cannot tell which is which.

The reference is Mort Garson's *Mother Earth's Plantasia*, 1976 - subtitled "warm
earth music for plants and the people who love them" and composed on the premise
that plants respond to it. A plant seeking out music written for plants closes
that loop of projection rather than resolving it.

**Implementation rule: the music need must not be marked in the code as different
from the others**, and must be disabled the same way - by setting a constant to
zero. If it needs special handling, the point has been lost.

### Five ways of knowing

The needs diagram is really about epistemics rather than wiring. Each need is
known by a different means, and the asymmetry is the design.

| Need | Known by | How |
|---|---|---|
| Light | **Measured** | Four LDRs. Direct, and the only directional sense the unit has. |
| Carbon | **Inferred** | Four weak, non-directional signals combined. Never measured. |
| Water | **Delegated** | To a human, who presses the pad. The plant has no water sensor. You are it. |
| Rest | **Internal** | Elapsed motor running time. No sensor at all. |
| Music | **Attributed** | No biological basis. A human belief about what plants want. |

### Telling music from conversation

The sound module reports only loud or quiet. The discriminator is not frequency -
which an ATmega328P cannot afford - but **duty cycle**, counted over roughly
thirty seconds:

| Sound | Reading | Duty |
|---|---|---|
| Continuous | Music | > 80% |
| Intermittent, turn-taking | Conversation, therefore people, therefore carbon | 30-70% |
| Little or none | Empty | < 15% |

Three states from one digital pin and a counter, at no extra cost. The sound
module therefore carries a second duty, as the ultrasonic does for obstacles.

Like every other proxy it is **not directional**. The unit samples loudness as it
moves and returns to where it was louder - the same wander, evaluate and remain
behaviour the carbon need already requires. A second microphone would give crude
left-right discrimination across the chassis width, but the baseline is short and
the result noisy. Not recommended.
