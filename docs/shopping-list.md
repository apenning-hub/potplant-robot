# Shopping list

Five robots. Everything bought online in bulk - Jaycar charges roughly three to
five times these prices for generic breakout modules, and nothing here is needed
this week except what you already have.

**Prices are rough estimates in AUD and have not been checked.** Treat the totals
as a budgeting guide, not a quote.

---

## Already have

- [x] Five Arduino UNO boards
- [x] One Funduino motor control shield v1.0 (two L293D, one 74HC595)
- [x] One Duinotech 2WD chassis, **XC4472** - TT gear motors, 65 mm wheels, castor
- [x] LDRs, resistors, breadboard, Dupont jumpers

> **Unconfirmed:** how many motor shields and chassis you have. If it is one of
> each, add four of each below.

---

## Per robot - the core build

| | Item | Why | Est. |
|---|---|---|---|
| [ ] | 4x LDR + 4x 10k resistor | Light, one at each corner | ~$2 |
| [ ] | MAX7219 8x8 matrix | The face | ~$6 |
| [ ] | HC-SR04 ultrasonic | Obstacles, and "something is close" | ~$3 |
| [ ] | TTP223 touch module | A pat on the pot rim | ~$1 |
| [ ] | TCRT5000 reflectance module | Cliff detection. **Not optional - stairs.** | ~$1 |
| [ ] | PCF8574 I2C expander | Keeps A4/A5 free for the sensor bus | ~$1 |
| [ ] | 4xAA holder with leads | Motor power, into EXT_PWR | ~$1.50 |
| | **Per robot** | | **~$16** |
| | **Five robots** | | **~$80** |

## Per robot - inferring people

Deferred until the swarm moves on light alone, but ordered at the same time
since shipping takes weeks. See "Inference over instrumentation" in
[`concept.md`](concept.md).

| | Item | Why | Est. |
|---|---|---|---|
| [ ] | MLX90614 IR thermometer | Warm body at close range | ~$6 |
| [ ] | AHT20 temp + humidity | Exhaled breath is warm and damp | ~$2 |
| [ ] | Sound module (digital out) | Conversation means people | ~$1 |
| | **Five robots** | | **~$45** |

Both I2C parts join the bus on A4/A5. The sound module uses its digital output on
the expander, because every analogue pin is taken by the LDRs.

## If you need bodies

| | Item | Est. each |
|---|---|---|
| [ ] | 2WD chassis kit with motors and wheels | ~$10 |
| [ ] | Motor shield v1 clone | ~$3 |

## Power

| | Item | Notes | Est. |
|---|---|---|---|
| [ ] | 20x rechargeable AA + charger | Five robots, four cells each. **Nearly as much as the sensors.** | ~$70 |

## The plants

| | Item | Notes |
|---|---|---|
| [ ] | Five plants, mixed | Different species make different temperaments legible |
| [ ] | Five **plastic** pots | Not terracotta. The TT motors have limited payload. |

Worth including one CAM plant - an orchid or a succulent. CAM plants take in CO2
at **night**, so it would hunt people after dark and light during the day, on an
inverted schedule. Real biology, visibly different behaviour, no code change.

---

## Deferred, deliberately

| Item | Est. | Why not yet |
|---|---|---|
| SCD40 NDIR CO2 sensor | $30 ea | See `concept.md` - inference is the better position, and $150 is the largest single line in the project |
| Load cell + HX711 | $5 ea | Only earns its place for a plant in bark |
| Colour sensor | $4 ea | Root-colour reading. Interesting, hardest to execute. |
| Radio modules | $2 ea | **Rejected on principle.** The swarm's arrangement is the shared data. |

---

## Rough budget

| | Est. |
|---|---|
| Core build, five robots | ~$80 |
| People-inference sensors, five robots | ~$45 |
| Four more chassis and shields, if needed | ~$52 |
| Batteries and charger | ~$70 |
| Plants and pots | ~$60 |
| **Total** | **~$307** |

Against roughly $500 for the two-robot version with NDIR sensors. Five plants,
cheaper, and a better argument.

---

## Avoid

**MQ-135**, sold everywhere as a CO2 sensor for about $3. It measures tin-oxide
resistance and responds to alcohol, smoke and VOCs. The CO2 figure is inferred
from a relationship that does not hold indoors. For work that will be examined,
a sensor that responds to hand sanitiser is a liability. **CCS811** and **SGP30**
report "eCO2" - estimated, not measured - and are at least honest about it.
