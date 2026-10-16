# RGB Cube ESPHome firmware

Native ESPHome firmware for an 8×8×8 RGB LED cube driven by chained TLC59711
controllers. The firmware uses ESP-IDF, exposes native Home Assistant entities,
and includes a small ESPHome web interface.

## Build

```sh
mise install
mise exec -- esphome compile esphome.yaml
```

## Upload

For the first USB upload:

```sh
mise exec -- esphome run esphome.yaml --device /dev/cu.usbserial-0001
```

Subsequent uploads can use USB or OTA:

```sh
mise exec -- esphome run esphome.yaml
```

## Wi-Fi setup

No Wi-Fi credentials are compiled into the firmware. Configure Wi-Fi using
Improv over USB or connect to the open `RGB Cube Setup` access point and visit
`http://192.168.4.1/`. Once connected, the device is available at
`http://rgb-cube.local/` and should be discovered by Home Assistant.

## Home Assistant entities

- Cube Switch
- Cube Mode (`Loop` or `Solo`)
- Cube Effect
- Cube Effect Timeout
- Cube Brightness
- Previous and next effect buttons
- Cube Restart

## Source layout

- `esphome.yaml` — ESPHome configuration and Home Assistant entities.
- `src/` — the small ESPHome lifecycle and persistence adapter.
- `lib/tlc59711/` — shared ESP-IDF TLC59711 SPI driver.
- `lib/cube_framebuffer/` — framebuffer, color, and drawing code; depends on
  `tlc59711`.
- `lib/ics43434/` — ICS-43434 ESP-IDF I²S acquisition and ESP-DSP FFT
  processing.
- `lib/cube_effects/` — all ESP-IDF effect implementations, including
  `mic_spectrum`; depends on `cube_framebuffer` and `ics43434`.

There is only one copy of each effect. ESPHome compiles the original ESP-IDF
implementations directly from `lib/cube_effects/effect_functions/`.
The microphone spectrum keeps the original ESP-IDF rendering and FFT behavior,
with only its I²S acquisition migrated to the current ESP-IDF channel API.
