# Project libraries

- `tlc59711` provides the ESP-IDF SPI driver for the chained LED controllers.
- `cube_framebuffer` provides framebuffer, color, and drawing operations and
  depends on `tlc59711`.
- `ics43434` provides native ESP-IDF I²S acquisition and ESP-DSP FFT
  processing for the ICS-43434 microphone.
- `cube_effects` contains all canonical effects originally implemented for the
  ESP-IDF firmware and depends on `cube_framebuffer` and `ics43434`.

ESPHome copies and compiles these libraries directly from `esphome.yaml`.
Each library also contains an ESP-IDF `CMakeLists.txt` documenting its direct
dependencies.
