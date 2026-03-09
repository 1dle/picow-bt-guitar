
# Pico W XInput BLE Gamepad

This project implements a general-purpose XInput-compatible Bluetooth LE gamepad for the Raspberry Pi Pico W. It can be used as a base for custom controllers, adapters, or embedded projects.

## Features
- XInput-compatible BLE HID gamepad for PC
- Easily customizable GPIO-to-button/axis mapping
- Example: Guitar Hero controller adapter (see below)

## Project Structure
- `src/` — Core code for a generic XInput BLE gamepad (see `src/main.c` for a minimal template)
- `examples/guitar_controller.c` — Full implementation for a Guitar Hero controller (original project logic)
- `src/ble_hid_device.c`, `src/ble_hid.h`, `src/descriptor/XboxDescriptors.h` — BLE and XInput HID logic

## Building

Build with CMake as usual for Pico SDK projects. The default firmware target is:

- `xinput_ble_gamepad` — Minimal generic gamepad (template)

To build the Guitar Hero controller example, use `examples/guitar_controller.c` as a reference or add it as a target in your own CMakeLists.txt if needed.

## Example: Guitar Hero Controller


The `examples/guitar_controller.c` file contains the original implementation for using a Guitar Hero controller with the Pico W. See the code for pinout and mapping details.

---

### Memo: Original Project Idea

This project was originally created as a Bluetooth adapter for Guitar Hero controllers on the Raspberry Pi Pico W, allowing them to work with PC via XInput over BLE. The codebase has since been generalized to support any XInput BLE gamepad use case, but the original guitar controller logic is preserved as an example.

The original Guitar Hero controller project can be found at [this commit](https://github.com/1dle/xinput-ble-gamepad/tree/6303c595f6e54ccd3da0a82c1c512357c12cd132).
### Guitar Pinout

| Pico GPIO | Button |
| - | - |
| GP0 - GP4 | Frets |
| GP6 - GP9 | DPAD |
| GP18 | LED on DPAD |
| GP19, GP20 | Start, select |
| GP21, GP22  | Strums |
| GP27_A1 | Whammy potentiometer |

## Creating Your Own Controller

Copy and modify `src/main.c` or use it as a template for your own GPIO/button/axis mapping. See the code comments for guidance.

## Credits

- Inspired by [esp32-ble-guitar](https://github.com/1dle/esp32-ble-guitar)