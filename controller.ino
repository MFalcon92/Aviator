/*
 _____ _             _           _      _
|_   _| |__   ___   / \__   ___ (_) __ _| |_ ___  _ __
  | | | '_ \ / _ \ / _ \ \ / / | |/ _` | __/ _ \| '__|
  | | | | | |  __// ___ \ V /  | | (_| | || (_) | |
  |_| |_| |_|\___/_/   \_\_/   |_|\__,_|\__\___/|_|

THE AVIATOR — Handheld Controller
===================================
Microcontroller : Arduino Nano
Radio           : nRF24L01 (2.4GHz, 100m+ range)

Inputs:
  A0  — KY-023 joystick VRy  (analog, 0–1023, up/down drone altitude)
  D2  — ARM/LAND toggle      (momentary push, active-LOW)
  D3  — GRAB button          (momentary push, active-LOW)
  D4  — RELEASE button       (momentary push, active-LOW)
  D5  — LASER button         (momentary, drives laser pointer via transistor)
  D6  — POWER OFF button     (momentary push, active-LOW)
  D7  — Laser transistor base (drives 5mW laser module)

nRF24L01 wiring (SPI):
  nRF VCC  → Arduino 3.3V   ← MUST be 3.3V, not 5V
  nRF GND  → Arduino GND
  nRF CE   → D9
  nRF CSN  → D10
  nRF SCK  → D13
  nRF MOSI → D11
  nRF MISO → D12

KY-023 wiring:
  VRy  → A0
  GND  → GND
  +5V  → 5V   (KY-023 works at 5V; output is 0–5V but we only use
                relative values 0–1023 so full range is fine here)
  VRx  → not connected
  SW   → not connected

Power:
  9V battery → Arduino Vin pin (onboard regulator steps to 5V and 3.3V)
  nRF24L01 draws up to 12mA — power from Arduino 3.3V pin, not a GPIO

Packet format (7 bytes sent every 20ms):
  [0] joystick Y high byte  (bits 9–8 of 0–1023 value)
  [1] joystick Y low byte   (bits 7–0)
  [2] button bitmask:
        bit 0 = GRAB
        bit 1 = RELEASE
        bit 2 = ARM/LAND
        bit 3 = LASER  (informational only — laser driven locally)
        bit 4 = POWER OFF
  [3–6] reserved, set to 0

Libraries required (install via Arduino Library Manager):
  RF24 by TMRh20
*/

#include <SPI.h>
#include <RF24.h>

// ── Pin definitions ─────────────────────────────────────────────────────────
const int PIN_JOY_Y      = A0;
const int PIN_BTN_ARM    = 2;
const int PIN_BTN_GRAB   = 3;
const int PIN_BTN_RELEASE= 4;
const int PIN_BTN_LASER  = 5;
const int PIN_BTN_PWROFF = 6;
const int PIN_LASER_OUT  = 7;   // drives NPN transistor base for laser
const int PIN_NRF_CE     = 9;
const int PIN_NRF_CSN    = 10;

// ── Radio ───────────────────────────────────────────────────────────────────
RF24 radio(PIN_NRF_CE, PIN_NRF_CSN);
const byte ADDRESS[6] = "AVTR1";   // must match NRF_ADDRESS in the_aviator.py
const int  CHANNEL    = 76;        // must match NRF_CHANNEL in the_aviator.py

// ── Packet buffer ───────────────────────────────────────────────────────────
uint8_t packet[7];

// ── Button debounce ─────────────────────────────────────────────────────────
// Each button has a last-state tracker to detect edges cleanly.
// We send the instantaneous state each packet — the Pi does edge detection.
struct Button {
  int     pin;
  bool    last_state;
  unsigned long last_ms;
  const int DEBOUNCE_MS = 30;

  void begin(int p) {
    pin = p;
    pinMode(pin, INPUT_PULLUP);
    last_state = HIGH;
    last_ms    = 0;
  }

  bool pressed() {
    bool reading = digitalRead(pin);
    if (reading != last_state) {
      last_ms    = millis();
      last_state = reading;
    }
    if ((millis() - last_ms) > DEBOUNCE_MS) {
      return (last_state == LOW);   // active-LOW: LOW = pressed
    }
    return false;
  }
};

Button btnArm, btnGrab, btnRelease, btnLaser, btnPwrOff;

// ── Send interval ────────────────────────────────────────────────────────────
const unsigned long SEND_INTERVAL_MS = 20;   // 50Hz packet rate
unsigned long last_send_ms = 0;


void setup() {
  Serial.begin(115200);

  // Buttons
  btnArm.begin(PIN_BTN_ARM);
  btnGrab.begin(PIN_BTN_GRAB);
  btnRelease.begin(PIN_BTN_RELEASE);
  btnLaser.begin(PIN_BTN_LASER);
  btnPwrOff.begin(PIN_BTN_PWROFF);

  // Laser output
  pinMode(PIN_LASER_OUT, OUTPUT);
  digitalWrite(PIN_LASER_OUT, LOW);

  // nRF24L01
  if (!radio.begin()) {
    Serial.println("[CTRL] nRF24L01 not found — check wiring!");
    while (1) {}   // halt
  }
  radio.setPALevel(RF24_PA_LOW);       // increase to RF24_PA_HIGH for more range
  radio.setDataRate(RF24_250KBPS);     // 250kbps for maximum range
  radio.setChannel(CHANNEL);
  radio.openWritingPipe(ADDRESS);
  radio.stopListening();               // controller transmits only

  Serial.println("[CTRL] THE AVIATOR controller ready.");
}


void loop() {
  unsigned long now = millis();
  if (now - last_send_ms < SEND_INTERVAL_MS) {
    return;   // wait for next send window
  }
  last_send_ms = now;

  // ── Read joystick Y ──────────────────────────────────────────────────────
  int joy_y = analogRead(PIN_JOY_Y);   // 0–1023

  // ── Read buttons ─────────────────────────────────────────────────────────
  bool arm     = btnArm.pressed();
  bool grab    = btnGrab.pressed();
  bool release = btnRelease.pressed();
  bool laser   = btnLaser.pressed();
  bool pwroff  = btnPwrOff.pressed();

  // ── Drive laser locally ───────────────────────────────────────────────────
  // The laser pointer is powered directly from this controller —
  // no need to go through the drone for this.
  digitalWrite(PIN_LASER_OUT, laser ? HIGH : LOW);

  // ── Build packet ──────────────────────────────────────────────────────────
  packet[0] = (joy_y >> 8) & 0x03;   // high 2 bits
  packet[1] =  joy_y & 0xFF;         // low 8 bits
  packet[2] = (grab    ? 0x01 : 0)
            | (release ? 0x02 : 0)
            | (arm     ? 0x04 : 0)
            | (laser   ? 0x08 : 0)
            | (pwroff  ? 0x10 : 0);
  packet[3] = 0;
  packet[4] = 0;
  packet[5] = 0;
  packet[6] = 0;

  // ── Transmit ──────────────────────────────────────────────────────────────
  bool ok = radio.write(packet, sizeof(packet));

  // Debug output (comment out for production to save CPU)
  Serial.print("[TX] joy=");
  Serial.print(joy_y);
  Serial.print("  btn=0b");
  Serial.print(packet[2], BIN);
  Serial.print(ok ? "  OK" : "  FAIL");
  Serial.println();
}
