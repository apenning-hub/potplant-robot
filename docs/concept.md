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
