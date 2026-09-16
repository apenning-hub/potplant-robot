#!/usr/bin/env python3
"""
Sensor test bench for the pot plant robot.

Run this on your computer. It talks to the Arduino over the USB cable and walks
you through checking each piece of hardware.

    python3 tools/bench.py

The board must be running firmware/sensor_test first. Upload that once, from the
Arduino IDE or with arduino-cli, and then leave it alone - everything below can
be changed without reflashing.

Only one dependency:

    python3 -m pip install --user pyserial
"""

import sys
import time

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    sys.exit("pyserial is not installed. Run:\n\n"
             "    python3 -m pip install --user pyserial\n")

BAUD = 115200
CHANNELS = ("A0 front-left", "A1 front-right", "A2 back-right", "A3 back-left")

# Devices this project expects to find on the I2C bus.
KNOWN_I2C = {
    0x20: "PCF8574 expander", 0x21: "PCF8574 expander", 0x22: "PCF8574 expander",
    0x23: "BH1750 lux (or PCF8574)", 0x24: "PCF8574 expander",
    0x25: "PCF8574 expander", 0x26: "PCF8574 expander", 0x27: "PCF8574 expander",
    0x38: "AHT20 temperature/humidity",
    0x5A: "MLX90614 infrared thermometer",
    0x5C: "BH1750 lux",
    0x76: "pressure sensor", 0x77: "pressure sensor",
}

# ---------------------------------------------------------------------------
# Terminal helpers. Plain ANSI, no curses, so it works anywhere.

CLEAR, HOME, HIDE, SHOW = "\033[2J", "\033[H", "\033[?25l", "\033[?25h"
BOLD, DIM, RED, GREEN, RESET = "\033[1m", "\033[2m", "\033[31m", "\033[32m", "\033[0m"


def bar(value, lo=0, hi=1023, width=34):
    """Draw one reading as a bar. Seeing it move beats reading a number."""
    span = max(hi - lo, 1)
    filled = int(round((value - lo) / span * width))
    filled = max(0, min(width, filled))
    return "[" + "#" * filled + "." * (width - filled) + "]"


def rule(char="-", width=62):
    return char * width


# ---------------------------------------------------------------------------

class Bench:
    def __init__(self, port):
        self.ser = serial.Serial(port, BAUD, timeout=1.5)
        time.sleep(2.0)          # the board resets when the port opens
        self.ser.reset_input_buffer()

    def close(self):
        try:
            self.send("S")       # never leave motors running
        except Exception:
            pass
        self.ser.close()

    def send(self, cmd):
        self.ser.write(cmd.encode())
        self.ser.flush()

    def line(self):
        return self.ser.readline().decode(errors="replace").strip()

    def ask(self, cmd, prefix):
        """Send a command and return the fields of the reply we asked for."""
        self.send(cmd)
        for _ in range(8):       # skip anything unrelated, eg a deadman notice
            got = self.line()
            if not got:
                return None
            parts = got.split(",")
            if parts[0] == prefix:
                return parts[1:]
        return None

    # --- individual reads --------------------------------------------------

    def light(self):
        f = self.ask("L", "LDR")
        return [int(x) for x in f] if f and len(f) == 4 else None

    def distance(self):
        f = self.ask("U", "DIST")
        return int(f[0]) if f else None

    def spare(self):
        f = self.ask("P", "DIG")
        return (int(f[0]), int(f[1])) if f and len(f) == 2 else None

    def ram(self):
        f = self.ask("R", "RAM")
        return int(f[0]) if f else None

    def i2c(self):
        self.send("I")
        found = []
        for _ in range(140):
            got = self.line()
            if not got:
                break
            parts = got.split(",")
            if parts[0] != "I2C":
                continue
            if parts[1] == "END":
                break
            found.append(int(parts[1]))
        return found

    def motor(self, n, direction):
        return self.ask(f"M{n}{direction}", "MOT") is not None

    def stop(self):
        self.ask("S", "STOP")


# ---------------------------------------------------------------------------
# Tests

def test_light(b):
    print(f"\n{BOLD}Live light readings{RESET}")
    print("Wave a torch at each sensor in turn. Ctrl-C to stop.\n")
    time.sleep(1.0)
    try:
        print(HIDE, end="")
        while True:
            vals = b.light()
            if vals is None:
                print("\nNo reply from the board.")
                return
            print(HOME + CLEAR, end="")
            print(f"{BOLD}Live light readings{RESET}   Ctrl-C to stop\n")
            for name, v in zip(CHANNELS, vals):
                print(f"  {name:<16} {bar(v)} {v:>4}")
            steer = (vals[1] + vals[2]) - (vals[0] + vals[3])
            frontback = (vals[0] + vals[1]) - (vals[2] + vals[3])
            print(f"\n  {DIM}steer{RESET}  {steer:>5}   "
                  f"{'brighter right' if steer > 0 else 'brighter left' if steer < 0 else 'even'}")
            print(f"  {DIM}front/back{RESET} {frontback:>5}   "
                  f"{'brighter ahead' if frontback > 0 else 'brighter behind' if frontback < 0 else 'even'}")
            time.sleep(0.15)
    except KeyboardInterrupt:
        pass
    finally:
        print(SHOW, end="")
        print("\nStopped.")


def test_range(b, seconds=20):
    """Find what range each sensor actually covers, and suggest tuning values."""
    print(f"\n{BOLD}Range test - {seconds} seconds{RESET}")
    print("Cover each sensor with your hand, then uncover it.")
    print("Shine a torch on each one. Get the extremes.\n")
    input("Press Enter to start.")

    lo = [1023] * 4
    hi = [0] * 4
    start = time.time()
    try:
        print(HIDE, end="")
        while time.time() - start < seconds:
            vals = b.light()
            if vals is None:
                continue
            for i, v in enumerate(vals):
                lo[i] = min(lo[i], v)
                hi[i] = max(hi[i], v)
            left = seconds - (time.time() - start)
            print(HOME + CLEAR, end="")
            print(f"{BOLD}Range test{RESET}   {left:4.1f}s left\n")
            for name, l, h, v in zip(CHANNELS, lo, hi, vals):
                print(f"  {name:<16} {bar(v)} {v:>4}   min {l:>4}  max {h:>4}  span {h-l:>4}")
            time.sleep(0.05)
    except KeyboardInterrupt:
        pass
    finally:
        print(SHOW, end="")

    print("\n" + rule("="))
    print(f"{BOLD}RESULTS{RESET}\n")
    spans = [h - l for l, h in zip(lo, hi)]
    for name, l, h, s in zip(CHANNELS, lo, hi, spans):
        flag = f"   {RED}<-- TOO SMALL, check this one{RESET}" if s < 50 else ""
        print(f"  {name:<16} min {l:>4}   max {h:>4}   span {s:>4}{flag}")

    print()
    if min(spans) < 50:
        print(f"{RED}At least one sensor barely moved.{RESET} Likely causes:")
        print("  - missing its fixed resistor (an LDR alone cannot be read)")
        print("  - a wire in the wrong hole")
        print("  - you did not actually cover that one")
        return

    worst = min(spans)
    floor = int(sum(lo) / 4) + worst // 8
    deadband = max(6, worst // 12) * 2      # doubled: steering sums two per side
    print(f"{GREEN}All four responded.{RESET} Suggested starting values for tuning.h:\n")
    print(f"  const int LIGHT_FLOOR    = {floor};")
    print(f"  const int LIGHT_DEADBAND = {deadband};")
    print(f"\n  {DIM}Deadband is doubled because steering sums two sensors per side.{RESET}")
    print(f"  {DIM}These come from your hardware in your room, not from a guess.{RESET}")


def test_motors(b):
    """Walk through all four channels so you learn which wheel is which."""
    print(f"\n{BOLD}Motor identification{RESET}")
    print(f"{RED}Wheels off the ground.{RESET} Prop the chassis on a book.\n")
    print("Each channel runs forwards, then backwards. Watch which wheel moves.")
    print("Channels 3 and 4 will whine audibly - that is why the robot uses 1 and 2.\n")
    input("Press Enter when the wheels are clear.")

    result = {}
    for n in (1, 2, 3, 4):
        print(f"\n{BOLD}M{n}{RESET}")
        for d, word in (("f", "forwards"), ("b", "backwards")):
            print(f"  {word} ... ", end="", flush=True)
            if not b.motor(n, d):
                print(f"{RED}no reply{RESET}")
                continue
            # The board stops itself after its deadman period, but ask anyway.
            time.sleep(1.2)
            b.stop()
            print("done")
            time.sleep(0.4)
        answer = input(f"  Which wheel moved on M{n}? [l]eft / [r]ight / [n]othing: ").strip().lower()
        result[n] = {"l": "LEFT", "r": "RIGHT"}.get(answer[:1] if answer else "n", "nothing")

    b.stop()
    print("\n" + rule("="))
    print(f"{BOLD}RESULTS{RESET}\n")
    for n, which in result.items():
        print(f"  M{n}  {which}")

    left = [n for n, w in result.items() if w == "LEFT"]
    right = [n for n, w in result.items() if w == "RIGHT"]
    print()
    if len(left) == 1 and len(right) == 1:
        print(f"{GREEN}Put this in config.h:{RESET}\n")
        print(f"  const uint8_t MOTOR_LEFT_CHANNEL  = {left[0]};")
        print(f"  const uint8_t MOTOR_RIGHT_CHANNEL = {right[0]};")
        if set([left[0], right[0]]) != {1, 2}:
            print(f"\n  {RED}Note:{RESET} the robot firmware wants M1 and M2, because")
            print("  channels 3 and 4 whine. Consider moving the wires.")
    else:
        print(f"{RED}Could not work out a left/right pair.{RESET}")
        print("Check the motors are screwed into the terminals and try again.")


def test_ultrasonic(b):
    print(f"\n{BOLD}Ultrasonic, D2, single-pin mode{RESET}")
    print("If every reading says no echo, check that trigger and echo are")
    print("commoned through a 1k resistor to D2. Ctrl-C to stop.\n")
    time.sleep(1.0)
    try:
        print(HIDE, end="")
        while True:
            mm = b.distance()
            print(HOME + CLEAR, end="")
            print(f"{BOLD}Ultrasonic{RESET}   Ctrl-C to stop\n")
            if mm is None:
                print("  no reply from the board")
            elif mm < 0:
                print(f"  {DIM}no echo{RESET} - nothing in range, or not wired")
            else:
                print(f"  {bar(min(mm, 2000), 0, 2000)} {mm:>5} mm")
                if mm < 500:
                    print(f"\n  {GREEN}object close{RESET} - this is the range where")
                    print("  the infrared thermometer can tell you if it is warm")
            time.sleep(0.2)
    except KeyboardInterrupt:
        pass
    finally:
        print(SHOW, end="")
        print("\nStopped.")


def test_i2c(b):
    print(f"\n{BOLD}Scanning the I2C bus{RESET} on A4 (data) and A5 (clock)...\n")
    found = b.i2c()
    if not found:
        print("  Nothing found.")
        print(f"  {DIM}Expected, if no I2C parts are fitted yet.{RESET}")
        print("  If something is fitted: check power, ground, and that data")
        print("  and clock are not swapped.")
        return
    for addr in found:
        name = KNOWN_I2C.get(addr, "unrecognised")
        print(f"  0x{addr:02X}   {name}")
    print(f"\n  {len(found)} device(s).")


def test_spare(b):
    print(f"\n{BOLD}A4 and A5 as digital inputs{RESET}")
    print("For checking a touch pad or cliff sensor before the expander is")
    print("fitted. Ctrl-C to stop.\n")
    time.sleep(1.0)
    try:
        print(HIDE, end="")
        while True:
            v = b.spare()
            print(HOME + CLEAR, end="")
            print(f"{BOLD}Spare pins{RESET}   Ctrl-C to stop\n")
            if v is None:
                print("  no reply")
            else:
                print(f"  A4 = {v[0]}     A5 = {v[1]}")
            time.sleep(0.2)
    except KeyboardInterrupt:
        pass
    finally:
        print(SHOW, end="")
        print("\nStopped.")


# ---------------------------------------------------------------------------

def find_port():
    ports = [p for p in list_ports.comports()
             if "usbmodem" in p.device or "usbserial" in p.device or "wchusb" in p.device]
    if not ports:
        return None
    if len(ports) == 1:
        return ports[0].device
    print("More than one board found:")
    for i, p in enumerate(ports):
        print(f"  {i}  {p.device}  {p.description}")
    return ports[int(input("Which one? "))].device


MENU = f"""
{rule('=')}
  {BOLD}SENSOR TEST BENCH{RESET}
{rule('=')}
  1  live light readings
  2  range test        {DIM}gives you numbers for tuning.h{RESET}
  3  motor identification {DIM}which wheel is on which channel{RESET}
  4  ultrasonic distance
  5  scan the I2C bus
  6  read A4/A5 as digital
  7  free memory on the board
  q  quit
{rule('=')}"""


def main():
    port = sys.argv[1] if len(sys.argv) > 1 else find_port()
    if not port:
        sys.exit("No Arduino found. Plug it in, or pass the port:\n\n"
                 "    python3 tools/bench.py /dev/cu.usbmodem1234\n")

    print(f"Connecting to {port} ...")
    try:
        b = Bench(port)
    except serial.SerialException as e:
        sys.exit(f"Could not open {port}: {e}")

    ver = b.ask("V", "VER")
    if not ver:
        b.close()
        sys.exit("The board did not answer.\n"
                 "Is firmware/sensor_test uploaded? The robot firmware will not work here.")
    print(f"Connected: {ver[0]} v{ver[1]}")

    tests = {"1": test_light, "2": test_range, "3": test_motors,
             "4": test_ultrasonic, "5": test_i2c, "6": test_spare}
    try:
        while True:
            print(MENU)
            choice = input("  > ").strip().lower()
            if choice in ("q", "quit", "exit"):
                break
            if choice == "7":
                print(f"\n  {b.ram()} bytes free of 2048")
            elif choice in tests:
                tests[choice](b)
            else:
                print("  Not a choice. Pick a number, or q.")
    except (KeyboardInterrupt, EOFError):
        pass
    finally:
        print("\nStopping motors and closing.")
        b.close()


if __name__ == "__main__":
    main()
