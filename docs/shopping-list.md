# Shopping list

Grouped by the phase that needs it, so you can buy as you go rather than all at
once. Tick things off as they arrive.

**Prices are rough estimates in AUD and should be checked.** I have not verified
current Jaycar stock or pricing - the two part numbers below are the ones from
the original brief. Treat the totals as a budgeting guide, not a quote.

---

## Already have

- [x] Arduino UNO R3 clone (ATmega328P)
- [x] Funduino motor control shield v1.0 (two L293D, one 74HC595)
- [x] Duinotech 2WD chassis, **XC4472** - two TT gear motors, 65 mm wheels, castor
- [x] LDRs on a breadboard with divider resistors
- [x] Dupont jumpers, breadboard, assorted resistors

> **Check before buying:** how many LDRs do you actually have wired, three or
> four? The tutorial this started from uses four, one at each corner. See
> "Open questions" at the bottom - it changes what else fits.

---

## Phase 2 - obstacles, cliffs and the face

| | Item | Notes | Est. |
|---|---|---|---|
| [ ] | 8x8 LED matrix, MAX7219 | **XC4499**. The face. | ~$20 |
| [ ] | HC-SR04 ultrasonic module | Obstacle distance | ~$8 |
| [ ] | 2x IR reflectance sensor module | TCRT5000 type. **Cliff detection.** Same parts also read boundary tape. | ~$6 ea |
| [ ] | 4xAA battery holder with leads | Into EXT_PWR. Motors only. | ~$4 |
| [ ] | Rechargeable AA cells + charger | **Get these.** A robot like this eats alkalines. | ~$40 |

Cliff sensors are not optional. Stairs will destroy this robot.

---

## Phase 3 - affection

| | Item | Notes | Est. |
|---|---|---|---|
| [ ] | PIR motion sensor module | Detects warm moving bodies, so it distinguishes a person from a chair | ~$10 |
| [ ] | Capacitive touch module | TTP223 type | ~$5 |
| [ ] | Copper tape or aluminium foil | The touch pad on the pot rim | ~$8 |

---

## Phase 5 - the sensor bus

All of these speak I2C, so they share two wires rather than needing a pin each.

| | Item | Notes | Est. |
|---|---|---|---|
| [ ] | AHT20 or SHT31 breakout | Humidity **and** temperature in one part | ~$12 |
| [ ] | BH1750 or VEML7700 breakout | Real lux, so targets can be in actual units | ~$10 |
| [ ] | LTR390 UV breakout | Tells direct sun from a bright wall. What lets the orchid flee sunbeams. | ~$14 |
| [ ] | PCF8574 I2C expander | Hands back the pins the PIR and touch pad currently use | ~$6 |

---

## Phase 6 - the second robot

A complete duplicate. This is the expensive phase.

| | Item | Est. |
|---|---|---|
| [ ] | Arduino UNO R3 clone | ~$30 |
| [ ] | Motor shield v1 clone | ~$15 |
| [ ] | 2WD chassis, XC4472 | ~$25 |
| [ ] | 8x8 matrix, XC4499 | ~$20 |
| [ ] | Second set of every sensor above | ~$90 |
| [ ] | Battery holder and cells | ~$25 |

### The plants themselves

| | Item | Notes |
|---|---|---|
| [ ] | Jade plant, *Crassula ovata* | Plant A. Wants full sun, dry, warm. Very hard to kill. |
| [ ] | Moth orchid, *Phalaenopsis* | Plant B. Wants bright shade, humid, cool nights. |
| [ ] | 2x **plastic** pots | Not terracotta. The TT motors have limited payload and a jade in terracotta can pass a kilogram before soil. |
| [ ] | Orchid bark mix | Orchids are epiphytes. They do not grow in soil. |

---

## Phase 7 and experiments

| | Item | For |
|---|---|---|
| [ ] | HX711 amplifier + small load cell | Thirst by weight. How growers actually check an orchid - they pick it up. |
| [ ] | TCS34725 colour sensor | Reading root colour. Orchid roots go silver when thirsty, green when watered. |
| [ ] | IR LED + TSOP38238 receiver | Virtual wall beacon for doorways, the way a Roomba does it. |

---

## Practical things worth having

| | Item | Why |
|---|---|---|
| [ ] | Black gaffer tape | Boundary lines for the cliff sensors to read |
| [ ] | Long USB-B cable, 3 m | Tethered testing without crawling about |
| [ ] | Velcro straps and zip ties | Mounting the pot and dressing the wiring |
| [ ] | Spare TT gear motors | The plastic gears are the first thing to fail |
| [ ] | Heat shrink and a decent side cutter | Wiring that survives being driven into furniture |

---

## Rough budget

| Stage | Est. |
|---|---|
| Phases 2-3, first robot complete | **~$110** |
| Phase 5 sensors | **~$42** |
| Phase 6, second robot and both plants | **~$230** |
| Experiments and consumables | **~$80** |

Again: estimates, not a quote. Check prices before committing.

---

## Open questions before you buy

**Three LDRs or four?** The code this project started from uses four, one at each
corner, which gives the robot 360-degree awareness - it can tell that light is
*behind* it. Three forward-facing sensors cannot; they only see a gradient
ahead.

Four is arguably the better arrangement, but it costs pin A3, which the current
plan gives to the ultrasonic echo. Worth deciding before Phase 2, because it
changes the pin map. See [`design-notes.md`](design-notes.md).
