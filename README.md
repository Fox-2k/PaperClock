# PaperClock

Paper Clock is a project about a clock printed in 3d which use an ePaper screen to display time, temperature and humidity.
Designed to be low energy as much as possible.

# Hardware requirements

*Project was designed to work with this hardware. You may make it work with equivalent but pay attention to voltage and dimension, especially for 3D printed frame !*

- [1 Waveshare 7.5inch E-Ink Display Hat for Raspberry Pi 880×528](https://www.amazon.fr/gp/product/B08QCJFJK5)
- [1 Arduino BLE sense 3.3V](https://store.arduino.cc/products/arduino-nano-33-ble-sense)
- [1 18650 battery coupler with 3.3V output](https://www.amazon.fr/gp/product/B086W7326Q) or equivalent
- 1 18650 battery
- 1 breadboard with straps (to try and dev) or [the PaperClock PCB (to make it definitively real)](https://oshwlab.com/Fox2k/e-paper-clock)
- 1 3D printer to print the 2 pieces frame (will be shared soon)

# Software requirement

- [Arduino IDE v1.8+](https://www.arduino.cc/en/software)
    - Add this package from boards manager : Arduino Mbed OS Nano Boards
    - Add this package from library manager : Arduino_HTS221 (for temperature and humidity sensors of the Nano 33 BLE Sense)

# Ressources

- [WaveShare E-Ink Display documentation](https://www.waveshare.com/wiki/7.5inch_HD_e-Paper_HAT_(B))
