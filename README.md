# 🎮 LoverBoy ![Status](https://img.shields.io/badge/status-in%20development-yellow) ![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue) ![Framework](https://img.shields.io/badge/framework-Arduino-cyan)

> *A passion project.*

[Versão em Português](./README-pt.md)

![LoverBoy logo](LoverBoy_logo.png)

A portable game console, inspired by the iconic "Game Boy", built from scratch using a LILYGO ESP32-S3, basic electronic components, and lots of programming! This is a personal educational project, originally developed using the materials and maker space at Colégio Técnico da UFMG (Coltec) and constantly evolving during my Computer Science undergraduate studies (DCC/UFMG).

## 🕹️ About the Project

The goal of LoverBoy is to build a robust and modular hardware and software platform, developing practical skills in:

- **Hardware Development:** Circuit assembly, soldering, and component integration.
- **Low-Level Programming:** Direct interaction with GPIOs, timers (PWM), and microcontroller peripherals.
- **Software Architecture:** Creating an operating system organized by responsibilities, with abstract classes dictating the game lifecycle via the Template Method pattern.
- **Power Management:** Implementing a rechargeable battery system for true portability.

## ⚙️ Hardware Used

- **Development Board:** LILYGO T-Display ESP32-S3 (with integrated 1.9" 170x320 TFT display).
- **Inputs:** 8 push buttons (D-Pad, Start, Select, A, B).
- **Audio:** 1 Passive buzzer for playing melodies and sound effects.
- **Power:** 3.7V rechargeable LiPo battery, managed by a TP4056 USB-C charging module and a slide switch.

## ✨ Firmware Features

The LoverBoy software was designed to be modular, scalable, and responsive.

- **Included Games:** Currently features the classics Crossy Road, Flappy Bird, Frogger, Genius, Snake, and Tetris, running smoothly at ~60 FPS.
- **Persistent Highscore System:** The console has built-in Flash memory that saves the Top 3 for each game, including a classic arcade-style 3-initial input screen.
- **In-Game Tutorials:** Every game features a dedicated tutorial screen accessible from a 3-option title carousel (Play, Tutorial, Highscores), detailing controls and objectives in both English and Brazilian Portuguese.
- **Dynamic Settings Menu:** A dedicated system UI allows adjusting volume, changing screen brightness, and resetting records.
- **Multi-language UI:** English (default) and Brazilian Portuguese, selectable in Settings. Every on-screen string lives in one table (`i18n_strings.h`), and accented letters are rendered on the retro pixel font by `TextRenderer`.
- **Object-Oriented Architecture (OOP):** The system is orchestrated by a Core (`LoverBoyCore`) that delegates tasks to specific managers (`InputManager`, `MenuManager`, `BatteryManager`, `ConfigManager`, `SoundManager`). New games inherit from a `Game` superclass that provides the standard state machine and template methods.
- **Non-Blocking Audio:** A sound system that plays background melodies and sound effects via a state machine, without freezing the game's physics or logic.
- **Responsive Controls (`InputManager`):** Centralized button reading with asymmetric zero-latency press detection, continuous-LOW release qualification (preventing double clicks), DAS/ARR auto-repeat, and state-transition consumption.

## 📂 Code Structure

The project follows an organization that separates firmware responsibilities. 

```
firmware/LoverBoy/
|
|-- LoverBoy.ino                      # Entry point: initializes system and runs loop
|-- src/
    |-- core/                       # Core system managers & hardware abstraction
    |   |-- LoverBoyCore.h/.cpp       # Global state orchestrator (Menu vs Game)
    |   |-- InputManager.h/.cpp     # Button debounce, edge detection, and auto-repeat
    |   |-- BatteryManager.h/.cpp   # ADC read & piecewise battery percentage calculation
    |   |-- ConfigManager.h/.cpp    # Centralized NVS configuration persistence
    |   |-- SoundManager.h/.cpp     # Non-blocking audio queue & melody player
    |   |-- pins.h                  # Hardware pin mappings for ESP32-S3 & display
    |   |-- audio_assets.h          # Frequency library and game melody arrays
    |
    |-- games/                      # Game engine & playable games
    |   |-- Game.h/.cpp             # Base abstract class with Template Method lifecycle
    |   |-- CrossyRoadGame.h/.cpp   # Crossy Road implementation
    |   |-- FlappyBirdGame.h/.cpp   # Flappy Bird implementation
    |   |-- FroggerGame.h/.cpp      # Frogger implementation
    |   |-- GeniusGame.h/.cpp       # Simon/Genius implementation
    |   |-- SnakeGame.h/.cpp        # Snake implementation
    |   |-- TetrisGame.h/.cpp       # Tetris implementation
    |
    |-- ui/                         # User interface & visual presentation
    |   |-- MenuManager.h/.cpp      # Splash screen, game carousel, settings, & HUD
    |   |-- TextRenderer.h/.cpp     # UTF-8 text renderer with accent overlays
    |   |-- Utf8Text.h/.cpp         # UTF-8 parsing and accent decomposition
    |
    |-- localization/               # Internationalization & translations
        |-- Localization.h/.cpp     # Language selector and string lookup (tr())
        |-- i18n_strings.h          # Central string table for all supported languages
```

## 🚀 How to Compile and Upload

### Option 1: Native Linux (`arduino-cli`)
Ideal for standalone Linux environments.

1. **Compile**:
   ```bash
   arduino-cli compile --fqbn esp32:esp32:lilygo_t_display_s3 firmware/LoverBoy
   ```

2. **Upload**:
   ```bash
   arduino-cli upload -p /dev/ttyACM0 --fqbn esp32:esp32:lilygo_t_display_s3 firmware/LoverBoy
   ```
   *(Adjust `-p` with your device port).*

### Option 2: Native Windows (Arduino IDE)
Ideal for Windows users who prefer a graphical interface.

1. **Board Support:** Install **ESP32** board support in the Arduino IDE Boards Manager.
2. **Libraries:** Install the **TFT_eSPI** library by Bodmer.
3. **Display Configuration:** In `TFT_eSPI/User_Setup_Select.h`, uncomment `#include <User_Setups/Setup206_LilyGo_T_Display_S3.h>`.
4. **Compile & Upload:**
   - Open `firmware/LoverBoy/LoverBoy.ino`.
   - Select the **LilyGo T-Display-S3** board and the corresponding `COMx` port.
   - Click **Upload**.

### Option 3: WSL Terminal (`build_and_upload.sh`)
Ideal for developers working directly inside the WSL (Linux) terminal on Windows.

1. **One-Command Build & Flash** (inside WSL terminal):
   ```bash
   ./build_and_upload.sh -p COM7
   ```
   *Customizable parameters & flags:*
   - `-p, --port <PORT>`: Target serial port on Windows host (default: `COM7`).
   - `-b, --baud <BAUD>`: Upload baud rate (default: `921600`).
   - `-d, --dir <DIR>`: Custom firmware path (defaults to `firmware/LoverBoy`).
   - `-c, --cli <PATH>`: Custom `arduino-cli` binary in WSL (auto-detected if omitted).
   - `-e, --esptool <PATH>`: Custom `esptool.exe` path on Windows host (auto-detected if omitted).

## 👤 Author

- **Vítor Paixão** - [https://github.com/paixonite](https://github.com/paixonite)
