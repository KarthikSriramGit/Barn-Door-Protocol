# Door Monitor

ESP32 firmware for a door and window monitoring system. It watches magnetic reed sensors, sends Telegram alerts on open/close, supports multiple zones, quiet hours, NVS configuration, and optional extras (MQTT, OTA, buzzer, Telegram away/here polling).

**Target:** ESP32 (ESP-IDF 5.4.x)  
**Hardware:** ESP32 CP2012 USB-C 38‑pin core board + magnetic reed (NC) door/window sensors.

This repository contains only the Door Monitor project. App sources, `CMakeLists.txt`, and this README are at the repo root. Build and run from the root directory.

---

## Quick start

1. Set Wi‑Fi and Telegram in `main/src/app_config.c` (or via NVS).
2. Wire a **NC** reed sensor to **GPIO 4** (sensor closed = contact to GND) and an LED to **GPIO 2**.
3. Run `idf.py set-target esp32 && idf.py build && idf.py -p COMx flash monitor`.
4. Open or close the sensor; you get Telegram alerts when in **away** mode.

---

## Overview

Door Monitor runs on an ESP32, connects to Wi‑Fi, syncs time via NTP, and polls magnetic reed sensors on configurable GPIOs. When a door or window opens or closes it:

- Drives a status LED (on = any zone open, off = all closed)
- Sends Telegram messages to one or more chat IDs (open/closed alerts)
- Repeats “door open” alerts at a configurable interval while in **away** mode

It supports up to 8 zones, debouncing, quiet hours, and full NVS-based configuration. Optional build-time features add MQTT, OTA, a local buzzer, and Telegram “away”/“here” reply handling.

---

## Features

- **Multi-zone sensors** — Up to 8 zones, each with its own GPIO and label (e.g. “front door”, “kitchen window”).
- **Debouncing** — Configurable debounce (default 50 ms) per zone to avoid false triggers.
- **Telegram notifications** — Open/closed alerts to multiple users, with custom greetings per chat.
- **Away / here mode** — **Away:** instant open alerts plus repeating reminders. **Here:** only state-change (open/closed) notifications.
- **Quiet hours** — Time window (e.g. 22:00–07:00) during which alerts are suppressed.
- **NVS configuration** — Wi‑Fi, Telegram, zones, intervals, and related settings in NVS; defaults when keys are missing.
- **Watchdog** — Task watchdog to recover from main-loop stalls.

**Optional (Kconfig, off by default):** heartbeat logging, MQTT publish, HTTPS OTA updates, buzzer on alert, Telegram getUpdates polling for “away”/“here” replies.

---

## FreeRTOS usage

The project runs on **ESP-IDF**, which uses **FreeRTOS** as its RTOS. Door Monitor relies on FreeRTOS for concurrency, timing, and synchronization:

- **Tasks** — The main application runs in the default `app_main` task (sensor poll loop, LED updates, notifications). Optionally, **Telegram poll** (Kconfig) spawns a separate task `tg_poll` that periodically calls the Telegram `getUpdates` API to handle “away”/“here” replies. Wi‑Fi, NTP, and HTTP run in ESP-IDF internal tasks.
- **`vTaskDelay`** — The main loop uses `vTaskDelay(pdMS_TO_TICKS(10))` to yield between poll cycles (~100 Hz), avoiding busy‑wait and keeping the system responsive. The Telegram poll task sleeps for the configured poll interval between requests.
- **Event groups** — The Wi‑Fi manager uses a FreeRTOS event group to block until the station is connected (or timeout). The connect callback sets a bit; `wifi_mgr_wait_connected()` waits on that bit before continuing init.
- **Timing** — Millisecond timestamps come from `esp_timer_get_time()` (e.g. alert repeat interval, debounce). The optional **heartbeat** uses a high‑resolution periodic `esp_timer` to log “alive” messages at a fixed interval.
- **Task watchdog** — The app subscribes the main task to the **task watchdog** (TWDT). The main loop calls `system_wdt_feed()` each iteration. If the loop stalls (e.g. deadlock or infinite loop), the watchdog fires and the system resets, improving robustness.

---

## Hardware

### ESP32 board

**ESP32 CP2012 USB-C core board** (38 pins):

- 38-pin layout, narrower width, breadboard-friendly.
- Integrated antenna, RF balun, power amplifiers, LNAs, filters, and power management.
- Interfaces: UART, SPI, I2C, PWM, DAC, ADC.
- 2.4 GHz Wi‑Fi + Bluetooth dual-mode; STA / AP / STA+AP; standard AT commands.

USB is via **CP2012** (Type‑C). Use this board’s USB port for flashing and serial monitor.

### Magnetic reed switch (sensors)

**Magnetic reed switch — normally open (NO) / normally closed (NC) — door/window alarm:**

- Sold as door/window security sensors, magnetic contact switches, reed switches for alarms, GPS, etc.
- Often rated for DC 5 V / 12 V / 24 V; we use them only as dry contacts (no external voltage on the contact).

**Use NC (normally closed) types.** When the magnet is near the reed, the contact is closed; when the door/window opens and the magnet moves away, the contact opens.

**Wiring:**

- One side of the reed contact → **GND**.
- Other side → **GPIO** (e.g. GPIO 4 for zone 0).
- GPIO has internal pull-up. **Door closed** (magnet near): contact closed, GPIO pulled to GND → **low**. **Door open**: contact open, GPIO high → **high.**

### Other

- **LED** — Status indicator: on when any zone is open, off when all closed. Use a series resistor; default GPIO **2** (often the on-board LED).
- **Buzzer** (optional) — If enabled via Kconfig, connect a buzzer to the configured GPIO (default **15**). Driven when the door is open in away mode (and not in quiet hours).

### Default GPIOs

| Function        | GPIO | Notes                          |
|----------------|------|---------------------------------|
| Zone 0 (sensor)| 4    | Default first zone              |
| Status LED     | 2    | On-board LED on many boards     |
| Buzzer         | 15   | Only when buzzer enabled (Kconfig) |

Extra zones use GPIOs set in NVS. Avoid strapping pins (e.g. 0, 2, 12, 15) for critical logic if your board uses them at boot.

---

## Project structure

```
.
├── CMakeLists.txt
├── sdkconfig
├── main/
│   ├── CMakeLists.txt
│   ├── Kconfig              # Door Monitor menu (optional features)
│   ├── include/
│   │   ├── app_config.h
│   │   ├── app_scheduler.h
│   │   └── app_state.h
│   └── src/
│       ├── main.c
│       ├── app_config.c     # NVS load/save, defaults
│       ├── app_scheduler.c  # Quiet hours
│       └── app_state.c      # Away mode, last states
├── components/
│   ├── hal/                 # GPIO, sensor, LED, buzzer
│   ├── sensor/              # Multi-zone + debounce
│   ├── wifi_mgr/            # Wi‑Fi STA + NTP
│   ├── notify/              # Telegram, optional MQTT + Telegram poll
│   └── system/              # WDT, heartbeat, OTA
└── README.md
```

---

## Configuration

### NVS (runtime)

Config lives in NVS namespace `door_monitor`. Missing keys fall back to defaults.

| Key pattern        | Description |
|--------------------|-------------|
| `wifi_ssid`        | Wi‑Fi SSID |
| `wifi_pass`        | Wi‑Fi password |
| `tg_token`         | Telegram bot token |
| `tg_num_chats`     | Number of Telegram chats (1–8) |
| `tg_chat_0` …      | Chat IDs |
| `tg_greet_0` …     | Greetings per chat |
| `zone_count`       | Number of zones (1–8) |
| `zone_N_gpio`      | GPIO for zone N |
| `zone_N_label`     | Label for zone N |
| `debounce_ms`      | Debounce (ms) |
| `alert_interval_ms`| Repeat-alert interval when open in away mode (ms) |
| `led_gpio`         | Status LED GPIO |
| `away_mode`        | 1 = away, 0 = here |
| `quiet_en`         | 1 = quiet hours on |
| `quiet_start`      | Quiet hours start (0–23) |
| `quiet_end`        | Quiet hours end (0–23) |
| `mqtt_uri`         | MQTT broker URI (if MQTT enabled) |
| `mqtt_topic`       | MQTT topic (if MQTT enabled) |

Use `app_config_load()` / `app_config_save()` (or your own config UI) to read/write these.

### Build-time (Kconfig)

Run `idf.py menuconfig`, open **Door Monitor**. You can enable:

- Heartbeat and interval  
- MQTT  
- OTA and OTA firmware URL  
- Buzzer and buzzer GPIO  
- Telegram poll and poll interval  

### Defaults (when NVS unused)

- **Wi‑Fi:** SSID `Ace & King of Hearts`, password `PASSWORD` (change before use).
- **Telegram:** Token `TELEGRAM_TOKEN`, two chats `9999999999`, greetings “Hello Mr Stark” / “Hello Mrs Suseendran”.
- **Zones:** 1 zone, GPIO 4, label “door”.
- **LED:** GPIO 2.
- **Debounce:** 50 ms.
- **Alert interval:** 60 000 ms.
- **Away mode:** on.
- **Quiet hours:** off.

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

Run from the **repo root** (project root).

### Flash and monitor

```bash
idf.py -p COMx flash monitor
```

Use your board’s port (e.g. `COM3` on Windows, `/dev/ttyUSB0` on Linux). The CP2012 USB-C port is used for serial.

### OTA (optional)

1. In menuconfig: **Partition Table** → **Factory app, two OTA definitions**.
2. **Door Monitor** → **Enable OTA updates**, set **OTA firmware URL** (HTTPS to your `.bin`).
3. Rebuild. On boot, the app checks the URL and updates if possible, then restarts.

---

## First-time setup

1. **Build and flash** as above.
2. **Configure Wi‑Fi and Telegram** (edit defaults in `app_config.c` or use NVS):  
   - `wifi_ssid`, `wifi_pass`  
   - `tg_token`  
   - At least one `tg_chat_*` and `tg_greet_*`
3. **Telegram:** Create a bot with [@BotFather](https://t.me/BotFather), get the token, add the bot to your chat(s), use the chat ID(s) in config.
4. **Wiring:** NC reed sensor(s) and LED as in **Hardware** (and optional buzzer if enabled).
5. Power up. The device connects to Wi‑Fi, syncs NTP, and monitors. Open/close a sensor to trigger alerts.

---

## Optional features (Kconfig)

- **Heartbeat** — Periodic “alive” log messages.
- **MQTT** — Publish `open` / `closed` to a topic; set `mqtt_uri` and `mqtt_topic` in NVS.
- **OTA** — Boot-time HTTPS OTA when URL is set; use two-OTA partition table.
- **Buzzer** — Drive buzzer GPIO when door open in away mode (and not in quiet hours).
- **Telegram “away” / “here”** — Poll `getUpdates`, parse “away”/“here” replies, update away mode.

---

## GPIO summary

| Signal   | Default GPIO | Configurable via        |
|----------|--------------|--------------------------|
| Zone 0   | 4            | NVS `zone_0_gpio`        |
| Zones 1–7| —            | NVS `zone_N_gpio`        |
| LED      | 2            | NVS `led_gpio`           |
| Buzzer   | 15           | Kconfig `DOOR_MONITOR_BUZZER_GPIO` |

---

## License

Use and modify as you like. Comply with Telegram, MQTT, OTA, etc. terms where applicable.
