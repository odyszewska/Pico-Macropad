# MacroPad Pico 🚀

![Status](https://img.shields.io/badge/Status-Work_in_Progress-orange)
![Platform](https://img.shields.io/badge/Platform-Raspberry_Pi_Pico-red)
![Framework](https://img.shields.io/badge/Framework-Arduino_C%2B%2B-blue)

## 📌 About the Project
**MacroPad Pico** is a custom, programmable 4x4 USB macropad powered by the **Raspberry Pi Pico**. Designed to act as a standard USB Human Interface Device (HID), it requires no additional drivers and works right out of the box. 

The device goes beyond simple key presses. It features an interactive **20x4 LCD screen** with a custom UI and a dedicated mode-switching system. By pressing the 16th button (`S16`), the pad enters "Mode Select" state, turning the remaining 15 buttons into profile selectors. This allows you to pack hundreds of shortcuts, macros, and text snippets into one compact device.

## ✨ Features
* **Native USB HID:** Recognized by PCs natively as "MacroPad Pico" (Manufacturer: AO).
* **4x4 Button Matrix:** Efficiently scanned inputs with built-in state debouncing.
* **15 Dynamic Modes:** Easily switchable via the `S16` button overlay menu.
* **20x4 LCD Interface:** Real-time visual feedback displaying the current mode, active shortcuts, and custom 8x8 pixel icons (e.g., Play/Pause symbols).
* **Arduino Powered:** Written using the Arduino core for RP2040, making it easy to expand and modify.

## 🛠️ Hardware Requirements
* **Microcontroller:** Raspberry Pi Pico (RP2040)
* **Input:** 4x4 Matrix Keypad (16 buttons)
* **Display:** 20x4 LCD with HD44780 controller
* **Connection:** USB Cable

### Pinout Configuration
* **Matrix Rows (Input + Pull-Down):** Pins 4, 5, 6, 7
* **Matrix Cols (Output):** Pins 0, 1, 2, 3
* **LCD (RS, E, D4, D5, D6, D7):** Pins 8, 9, 10, 11, 12, 13

## 🗂️ Button Modes
The pad supports 15 distinct layouts. *Note: Most modes are currently in the UI/Serial testing phase and will be mapped to specific keystrokes soon.*

1. **Dashboard:** Essential shortcuts (Google, AGH, USOS), media controls, and volume.
2. **Numpad:** Standard 0-9 digits and math operators (`+`, `-`, `*`, `/`, `%`).
3. **Text Editing:** Ctrl+A, Delete, Backspace, Arrow navigation, Bold/Italic/Underline.
4. **Navigation:** Home, End, Page Up/Down, Tabs, and Arrow keys.
5. **Quick Texts:** *Work in progress*
6. **Emojis:** *Work in progress*
7. **Useful Sites:** *Work in progress*
8. **Media/Display:** *Work in progress*
9. **Terminal:** Git commands and directory navigation.
10. **Browser:** Tab management, incognito, zoom.
11. **Letters A-M:** With special smart-shift functionality.
12. **Letters N-Z:** With special smart-shift functionality.
13. **Punctuation:** Special characters and brackets.
