# Door Monitor

ESP32 firmware for a door and window monitoring system. It watches magnetic reed sensors, sends Telegram alerts on open/close, and supports away mode with repeating reminders.

**Target:** ESP32 (ESP-IDF 5.4.x)  
**Hardware:** ESP32 CP2012 USB-C 38‑pin core board + magnetic reed (NC) door/window sensors.

This repository contains only the Door Monitor project. App sources, `CMakeLists.txt`, and this README are at the repo root. You can rename the repo (or clone into) `door_monitor` / `door-monitor` if you prefer that folder name.

---

## Quick start

1. Set Wi‑Fi and Telegram in `main/door_monitor.c` (or update the `#define` defaults).
2. Wire a **NC** reed sensor to **GPIO 4** (sensor closed = contact to GND) and an LED to **GPIO 2**.
3. Run `idf.py set-target esp32 && idf.py build && idf.py -p COMx flash monitor`.
4. Open or close the sensor; you get Telegram alerts when in **away** mode.

---

## Overview

Door Monitor connects to Wi‑Fi, syncs time via NTP, and polls a magnetic reed sensor on GPIO 4. When the door or window opens or closes it:

- Drives a status LED (on = open, off = closed)
- Sends Telegram messages to one or more chat IDs (open/closed alerts)
- Repeats “door open” alerts every 60 seconds while in **away** mode

---

## Features

- **Reed sensor monitoring** — Single zone on GPIO 4 (configurable in code).
- **Telegram notifications** — Open/closed alerts to multiple users, with custom greetings per chat.
- **Away / here mode** — **Away:** instant open alerts plus 60 s repeating reminders. **Here:** only state-change (open/closed) notifications.
- **NTP** — Time sync via `pool.ntp.org` for accurate timestamps.

---

## Hardware

### ESP32 board

**ESP32 CP2012 USB-C core board** (38 pins):

- 38-pin layout, narrower width, breadboard-friendly.
- Integrated antenna, RF balun, power amplifiers, LNAs, filters, and power management.
- Interfaces: UART, SPI, I2C, PWM, DAC, ADC.
- 2.4 GHz Wi‑Fi + Bluetooth dual-mode; STA / AP / STA+AP; standard AT commands.

USB is via **CP2012** (Type‑C). Use this board’s USB port for flashing and serial monitor.

### Magnetic reed switch (sensor)

**Magnetic reed switch — normally open (NO) / normally closed (NC) — door/window alarm:**

- Sold as door/window security sensors, magnetic contact switches, reed switches for alarms, GPS, etc.
- Often rated for DC 5 V / 12 V / 24 V; we use them only as dry contacts (no external voltage on the contact).

**Use NC (normally closed) types.** When the magnet is near the reed, the contact is closed; when the door/window opens and the magnet moves away, the contact opens.

**Wiring:**

- One side of the reed contact → **GND**.
- Other side → **GPIO 4** (or your chosen sensor GPIO).
- GPIO has internal pull-up. **Door closed** (magnet near): contact closed, GPIO pulled to GND → **low**. **Door open**: contact open, GPIO high → **high.**

### Other

- **LED** — Status indicator: on when door open, off when closed. Use a series resistor; default **GPIO 2** (on-board LED on many boards).

### Default GPIOs

| Function   | GPIO | Notes            |
|-----------|------|------------------|
| Sensor    | 4    | Reed switch      |
| Status LED| 2    | On-board on many |

---

## Project structure

```
.
├── CMakeLists.txt
├── main/
│   ├── CMakeLists.txt
│   └── door_monitor.c
├── sdkconfig
└── README.md
```

---

## Configuration

Edit `main/door_monitor.c` and update:

- `WIFI_SSID`, `WIFI_PASS`
- `TELEGRAM_BOT_TOKEN`
- `TELEGRAM_CHAT_ID_1`, `TELEGRAM_CHAT_ID_2` (and greetings in code)
- `SENSOR_GPIO`, `LED_GPIO` if using different pins
- `ALERT_INTERVAL_MS` (default 60 s repeat when open in away mode)
- `away_mode` (true = away, false = here)

---

## Build and flash

### Requirements

- ESP-IDF **v5.4.x**
- ESP-IDF environment active (e.g. ESP-IDF 5.4 CMD or PowerShell; `idf.py` and `cmake` on `PATH`).

### Build

```bash
idf.py set-target esp32
idf.py build
```

### Flash and monitor

```bash
idf.py -p COMx flash monitor
```

Use your board’s port (e.g. `COM3` on Windows, `/dev/ttyUSB0` on Linux). The CP2012 USB-C port is used for serial.

---

## First-time setup

1. **Build and flash** as above.
2. **Configure** Wi‑Fi and Telegram in `main/door_monitor.c`.
3. **Telegram:** Create a bot with [@BotFather](https://t.me/BotFather), get the token, add the bot to your chat(s), use the chat ID(s) in code.
4. **Wiring:** NC reed to GPIO 4 and GND, LED to GPIO 2.
5. Power up. The device connects to Wi‑Fi, syncs NTP, and monitors. Open/close the sensor to trigger alerts.

---

## License

Use and modify as you like. Comply with Telegram’s terms where applicable.
