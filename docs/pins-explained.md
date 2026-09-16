# Pins, explained

For someone who has not done this before. Everything here uses this project's
own hardware as the example.

---

## Digital and analogue

The light sensors are on **analogue** pins because light is not on or off, it is
a quantity. `analogRead()` gives a number from 0 to 1023.

A **digital** pin knows only two things: HIGH, about 5 volts, or LOW, 0 volts.
Which sounds limiting until you notice how many questions are yes or no. Is the
pad being touched. Is there a floor below me. Is it loud in here.

**A0 to A5 are digital pins too.** They are ordinary digital pins that happen to
have a voltage meter attached as well. That is why the test bench can read A4 and
A5 as digital inputs - nothing special is going on, it is simply not using the
extra ability.

---

## The three commands

```cpp
pinMode(7, INPUT);            // I am going to listen on this pin
int state = digitalRead(7);   // gives HIGH or LOW

pinMode(7, OUTPUT);           // I am going to talk on this pin
digitalWrite(7, HIGH);        // push it to 5V
```

`pinMode` sets the pin's *direction* - ear or mouth. Set it before using the pin,
normally in `setup()`.

### Direction can change while running

The ultrasonic sensor depends on this. From `firmware/sensor_test/sensor_test.ino`:

```cpp
pinMode(PIN_ULTRASONIC, OUTPUT);     // become a mouth
digitalWrite(PIN_ULTRASONIC, HIGH);  // shout
delayMicroseconds(10);
digitalWrite(PIN_ULTRASONIC, LOW);

pinMode(PIN_ULTRASONIC, INPUT);      // become an ear
unsigned long us = pulseIn(PIN_ULTRASONIC, HIGH, 30000UL);
```

One pin does both jobs by switching direction mid-operation. That trick is what
makes the pin budget close - see `design-notes.md`.

---

## The thing that will actually bite you

**A digital input with nothing connected reads rubbish.** Not zero. Rubbish. It
picks up mains hum and static and flickers between HIGH and LOW.

We have already seen this. Running Phase 1 with nothing wired gave:

```
# ambient raw: 51 51 50 49
```

and then readings that climbed steadily to 205 while the robot sat still. Four
disconnected pins drifting. Same phenomenon, analogue version.

So this does **not** work:

```
5V ──── button ──── pin 7
```

Pressed, the pin sees 5V. Released, it is connected to nothing and reads
randomly. The button appears to press itself.

### The fix: a pull-up

A resistor that gently holds the pin at a known value when nothing else is
driving it. The chip has them built in:

```cpp
pinMode(7, INPUT_PULLUP);   // held HIGH unless something pulls it down
```

Then wire the button between **pin 7 and GND**.

| | Reads |
|---|---|
| Not pressed | HIGH |
| Pressed | LOW |

That is backwards from what you would expect. `LOW` means pressed. This catches
everybody once.

### Our modules do not need it

The TTP223 touch board and the TCRT5000 cliff board each contain a chip that
actively drives its output HIGH or LOW. They are never floating, so plain `INPUT`
is correct. It is bare buttons and bare switches that need the pull-up.

---

## What a pin cannot do

A pin can supply about **20 mA** safely, 40 mA absolute maximum. An LED is fine.
A motor is not - a TT gear motor wants hundreds of milliamps and would destroy
the chip.

That is the entire reason the motor shield exists. The controller sends a small
signal; the two L293D chips do the heavy work, from the cells.

---

## Getting more pins

This project runs out. Four light sensors take A0 to A3, the ultrasonic takes D2,
the matrix takes D9, D10 and D13, and the shield has D3 to D8, D11 and D12. That
leaves A4 and A5, which are the I2C pins.

A **PCF8574 expander** sits on those two wires and provides eight more digital
pins. You ask it over the bus - "what is pin 3 doing" - and it answers.

It is slower than a real pin: a fraction of a millisecond rather than
microseconds. Fine for a touch pad or a cliff sensor. **Not** fine for the
ultrasonic echo, where the measurement *is* a pulse width timed in microseconds
and a bus round trip destroys it.

That is the rule for deciding what goes on the expander: if the timing is the
measurement, it needs a real pin.

---

## This project's pins

| Pin | Carries | Kind |
|---|---|---|
| A0, A1, A2, A3 | Light sensors, four corners | Analogue in |
| A4, A5 | I2C bus - expander, and later the thermometer and hygrometer | Bus |
| D2 | Ultrasonic, single-pin mode | Digital, direction switches |
| D9, D10, D13 | Matrix data, chip select, clock | Digital out |
| D3-D8, D11, D12 | Motor shield | Unavailable |
| D0, D1 | USB serial | Never use |
| Expander 0-3 | Touch, cliff, sound, spare | Digital in, slow |

D0 and D1 are worth being firm about. They are the USB connection. Wire something
to them and uploading fails in ways that look like a broken board.
