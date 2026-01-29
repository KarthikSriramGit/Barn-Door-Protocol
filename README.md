# Barn Door Protocol

*"Sometimes you've gotta know who's coming through the door."*

ESP32 firmware for door and window monitoring. Magnetic reed sensors, Telegram alerts, multi‑zone coverage, quiet hours, NVS config, and optional MQTT, OTA, buzzer, and “away”/“here” polling. Think of it as a little sentinel for your barn—or apartment, lab, or workshop. No suit required; just an ESP32 and a reed switch.

**Target:** ESP32 (ESP-IDF 5.4.x)  
**Hardware:** ESP32 CP2012 USB-C 38‑pin core board + magnetic reed (NC) door/window sensors.

This repo is the project. Sources, `CMakeLists.txt`, and this README live at the root. Build and run from there.

---

## Quick start

1. Drop your Wi‑Fi and Telegram details into `main/src/app_config.c` (or feed them via NVS).
2. Wire a **NC** reed sensor to **GPIO 4** (closed = contact to GND) and an LED to **GPIO 2**.
3. `idf.py set-target esp32 && idf.py build && idf.py -p COMx flash monitor`.
4. Open or close the sensor. When you’re **away**, you get the alert. When you’re **here**, you get state changes. Your call.

---

## What it does

Barn Door Protocol runs on an ESP32, hits Wi‑Fi, syncs time via NTP, and polls magnetic reed sensors on configurable GPIOs. When something opens or closes:

- **LED** — On when any zone is open, off when everything’s sealed. Simple.
- **Telegram** — Alerts to one or more chat IDs. Open/closed, plus optional repeat reminders in **away** mode.
- **Up to 8 zones** — Label them. Front door, garage, that window you never quite trust. Debouncing, quiet hours, NVS-backed config. Optional extras: MQTT, OTA, buzzer, and Telegram “away”/“here” via getUpdates.

---

## Features

- **Multi‑zone** — Up to 8 zones, each with its own GPIO and label (e.g. *front door*, *kitchen window*).
- **Debouncing** — Configurable (default 50 ms) so we don’t cry wolf.
- **Telegram** — Open/closed alerts to multiple users, custom greetings per chat.
- **Away / here** — **Away:** instant open alerts + repeating reminders. **Here:** state-change only.
- **Quiet hours** — Time window (e.g. 22:00–07:00) when we stay quiet.
- **NVS config** — Wi‑Fi, Telegram, zones, intervals, and the rest in NVS; sensible defaults when keys are missing.
- **Watchdog** — Task WDT. Loop stalls? We reset. We don’t leave you hanging.

**Optional (Kconfig, off by default):** heartbeat logs, MQTT publish, HTTPS OTA, buzzer on alert, Telegram getUpdates for “away”/“here”.

---

## FreeRTOS usage

Runs on **ESP-IDF** over **FreeRTOS**. We use it like we mean it:

- **Tasks** — Main logic in `app_main`. Optional **Telegram poll** spawns `tg_poll` for getUpdates. Wi‑Fi, NTP, HTTP run in IDF tasks.
- **`vTaskDelay`** — Main loop yields with ~10 ms between polls (~100 Hz). No busy‑wait. Telegram poll task sleeps the configured interval between calls.
- **Event groups** — Wi‑Fi manager blocks on an event-group bit until we’re connected (or timeout).
- **Timing** — `esp_timer_get_time()` for alert/debounce timing; optional heartbeat uses a periodic `esp_timer`.
- **Task watchdog** — Main task on the TWDT. We feed it every loop. Stall = reset. Keeps things honest.

---

## Hardware

### ESP32 board

**ESP32 CP2012 USB-C core board** (38 pins):

- 38‑pin, narrow, breadboard‑friendly.
- Antenna, RF balun, LNAs, filters, power management—all onboard.
- UART, SPI, I2C, PWM, DAC, ADC. 2.4 GHz Wi‑Fi + BT; STA / AP / STA+AP.

USB via **CP2012** (Type‑C). Use it for flash and serial.

### Magnetic reed switch (sensors)

**NC/NO magnetic reed — door/window alarm style:**

- Dry contacts only. We use them as switches, not power sources.

**Use NC (normally closed).** Magnet near → closed. Door opens, magnet away → open.

**Wiring:**

- One contact side → **GND**. Other → **GPIO** (e.g. 4 for zone 0).
- GPIO pull‑up. **Closed:** contact shorts to GND → **low**. **Open:** contact open → **high.**

### Other

- **LED** — Status: on if any zone open, off when all closed. Default **GPIO 2** (often onboard). Use a series resistor if external.
- **Buzzer** (optional) — Kconfig‑enable, wire to default **GPIO 15**. Fires when door’s open in away mode (respects quiet hours).

### Default GPIOs

| Function       | GPIO | Notes |
|----------------|------|-------|
| Zone 0 (sensor)| 4    | First zone |
| Status LED     | 2    | Onboard on many boards |
| Buzzer         | 15   | Kconfig only |

Extra zones via NVS. Avoid strapping pins (0, 2, 12, 15, etc.) if your board uses them at boot.

---

## Project structure

```
.
├── CMakeLists.txt
├── sdkconfig
├── main/
│   ├── CMakeLists.txt
│   ├── Kconfig              # Barn Door Protocol menu (optional features)
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
│   ├── sensor/              # Multi‑zone + debounce
│   ├── wifi_mgr/            # Wi‑Fi STA + NTP
│   ├── notify/              # Telegram, optional MQTT + Telegram poll
│   └── system/              # WDT, heartbeat, OTA
└── README.md
```

---

## Configuration

### NVS (runtime)

Namespace `door_monitor`. Missing keys → defaults.

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
| `alert_interval_ms`| Repeat‑alert interval when open in away mode (ms) |
| `led_gpio`         | Status LED GPIO |
| `away_mode`        | 1 = away, 0 = here |
| `quiet_en`         | 1 = quiet hours on |
| `quiet_start`      | Quiet start (0–23) |
| `quiet_end`        | Quiet end (0–23) |
| `mqtt_uri`         | MQTT broker URI (if MQTT enabled) |
| `mqtt_topic`       | MQTT topic (if MQTT enabled) |

Use `app_config_load()` / `app_config_save()` (or your own config layer) to read/write.

### Build‑time (Kconfig)

`idf.py menuconfig` → **Door Monitor** (the protocol menu). Enable:

- Heartbeat and interval  
- MQTT  
- OTA and OTA firmware URL  
- Buzzer and buzzer GPIO  
- Telegram poll and poll interval  

### Defaults (no NVS)

- **Wi‑Fi:** SSID `Ace & King of Hearts`, password `PASSWORD` — change before deploy.
- **Telegram:** Token `TELEGRAM_TOKEN`, two chats `9999999999`, greetings *"Hello Mr Stark"* / *"Hello Mrs Suseendran"*.
- **Zones:** 1 zone, GPIO 4, label `"door"`.
- **LED:** GPIO 2. **Debounce:** 50 ms. **Alert interval:** 60 000 ms. **Away:** on. **Quiet hours:** off.

---

## Build and flash

### Requirements

- **ESP‑IDF v5.4.x**
- IDF environment active (`idf.py`, `cmake` on `PATH`). ESP‑IDF 5.4 CMD or PowerShell.

### Build

```bash
idf.py set-target esp32
idf.py build
```

From the **repo root**.

### Flash and monitor

```bash
idf.py -p COMx flash monitor
```

Use your port (`COM3`, `/dev/ttyUSB0`, etc.). CP2012 USB‑C for serial.

### OTA (optional)

1. **Partition table** → *Factory app, two OTA definitions*.
2. **Door Monitor** → *Enable OTA updates*, set **OTA firmware URL** (HTTPS to your `.bin`).
3. Rebuild. On boot we check the URL, update if we can, then restart.

---

## First‑time setup

1. **Build and flash** as above.
2. **Configure Wi‑Fi and Telegram** (`app_config.c` defaults or NVS): `wifi_ssid`, `wifi_pass`, `tg_token`, at least one `tg_chat_*` and `tg_greet_*`.
3. **Telegram:** [@BotFather](https://t.me/BotFather) → create bot, token, add to chat(s), plug in chat ID(s).
4. **Wiring:** NC reed(s) + LED (and optional buzzer) per **Hardware**.
5. Power up. We connect, sync NTP, and watch. Open/close a sensor → alerts.

---

## Optional features (Kconfig)

- **Heartbeat** — Periodic “alive” logs.
- **MQTT** — Publish `open` / `closed` to a topic; `mqtt_uri` and `mqtt_topic` in NVS.
- **OTA** — Boot‑time HTTPS OTA when URL is set; use two‑OTA partition table.
- **Buzzer** — Drive buzzer GPIO when door’s open in away mode (quiet hours respected).
- **Telegram “away” / “here”** — Poll getUpdates, parse “away”/“here”, update mode.

---

## GPIO summary

| Signal    | Default | Configurable via |
|-----------|---------|-------------------|
| Zone 0    | 4       | NVS `zone_0_gpio` |
| Zones 1–7 | —       | NVS `zone_N_gpio` |
| LED       | 2       | NVS `led_gpio` |
| Buzzer    | 15      | Kconfig `DOOR_MONITOR_BUZZER_GPIO` |

---

## License

Use and modify as you like. Stay compliant with Telegram, MQTT, OTA, etc. where it applies.

*Barn Door Protocol.* Your doors. Your rules.
