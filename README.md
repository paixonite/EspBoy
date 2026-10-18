# 🎮 EspBoy ![Status](https://img.shields.io/badge/status-in%20development-yellow) ![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue) ![Framework](https://img.shields.io/badge/framework-Arduino-cyan)

[Versão em Português](./README-pt.md)

![EspBoy logo](EspBoy_logo.png)

A portable game console, inspired by the iconic "Game Boy", built from scratch using a LILYGO ESP32-S3, basic electronic components, and lots of programming! This is a personal educational project, originally developed using the materials and maker space at Colégio Técnico da UFMG (Coltec) and constantly evolving during my Computer Science undergraduate studies (DCC/UFMG).

## 🕹️ About the Project

The goal of EspBoy is to build a robust and modular hardware and software platform, developing practical skills in:

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

The EspBoy software was designed to be modular, scalable, and responsive.

- **Included Games:** Currently features the classics Crossy Road, Flappy Bird, Frogger, Genius, Snake, and Tetris, running smoothly at ~60 FPS.
- **Persistent Highscore System:** The console has built-in Flash memory that saves the Top 3 for each game, including a classic arcade-style 3-initial input screen.
- **Dynamic Settings Menu:** A dedicated system UI allows adjusting volume, changing screen brightness, and resetting records.
- **Object-Oriented Architecture (OOP):** The system is orchestrated by a Core (`EspBoyCore`) that delegates tasks to specific managers (`MenuManager`, `BatteryManager`). New games inherit from a `Game` superclass that provides the standard state machine and template methods.
- **Non-Blocking Audio:** A sound system that plays background melodies and sound effects via a state machine, without freezing the game's physics or logic.
- **Responsive Controls:** Button reading utilizes edge detection, debounce techniques, and input leak prevention.

## 📂 Code Structure

The project follows an organization that separates firmware responsibilities. 

```
EspBoy/
|
|-- [ Entry Point ]
|-- EspBoy.ino            # Only initializes the console's operating system.
|-- pins.h                # Centralizes the ESP32 hardware mapping.
|
|-- [ Operating System (Managers) ]
|-- EspBoyCore.h/.cpp     # Global state orchestrator.
|-- MenuManager.h/.cpp    # Manages Splash Screen, Game Carousel, and Settings.
|-- BatteryManager.h/.cpp # Math logic and ADC for battery reading.
|-- ConfigManager.h/.cpp  # Centralized NVS configuration persistence.
|
|-- [ Game Engine ]
|-- Game.h/.cpp           # Base abstract class with Template Methods.
|
|-- [ Audio System ]
|-- SoundManager.h/.cpp   # Non-blocking audio queue manager.
|-- audio_assets.h        # Frequency library and melody arrays.
|
|-- [ Implemented Games ]
|-- CrossyRoadGame.h/.cpp # Isolated Crossy Road logic.
|-- FlappyBirdGame.h/.cpp # Isolated Flappy Bird logic.
|-- FroggerGame.h/.cpp    # Isolated Frogger logic.
|-- GeniusGame.h/.cpp     # Isolated Genius logic.
|-- SnakeGame.h/.cpp      # Isolated Snake logic.
|-- TetrisGame.h/.cpp     # Isolated Tetris logic.
```

## 🚀 How to Compile and Use

### Option A: Command Line / Automated Script (Recommended)

1. **One-Touch Build & Flash** (PowerShell on Windows):
   ```powershell
   powershell -ExecutionPolicy Bypass -File .\build_and_upload.ps1
   ```
   *(Optional: pass `-Port COMx` to specify your COM port).*

2. **Compile Only with `arduino-cli`** (WSL / Linux):
   ```bash
   arduino-cli compile --fqbn esp32:esp32:lilygo_t_display_s3 firmware/EspBoy
   ```

### Option B: Arduino IDE

1. **Board Support:** Install **ESP32** board support in the Arduino IDE Boards Manager.
2. **Libraries:** Install the **TFT_eSPI** library by Bodmer.
3. **Display Configuration:** In `TFT_eSPI/User_Setup_Select.h`, uncomment `#include <User_Setups/Setup206_LilyGo_T_Display_S3.h>`.
4. **Compilation & Upload:**
   - Open `firmware/EspBoy/EspBoy.ino`.
   - Select the **LilyGo T-Display-S3** board and the corresponding port.
   - Click **Upload**.

## 👤 Author

- **Vítor Paixão** - [https://github.com/paixonite](https://github.com/paixonite)
