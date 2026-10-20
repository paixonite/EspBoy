#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <Arduino.h>
#include "pins.h"

enum ButtonId : uint8_t {
    BTN_ID_UP = 0,
    BTN_ID_DOWN,
    BTN_ID_LEFT,
    BTN_ID_RIGHT,
    BTN_ID_A,
    BTN_ID_B,
    BTN_ID_START,
    BTN_ID_SELECT,
    BTN_COUNT
};

class InputManager {
public:
    // Tuned hardware debounce parameters (ms)
    // Immediate (0ms) rising-edge response for arcade responsiveness, paired with
    // continuous-LOW release qualification so contact bounce on press or release
    // can never trigger a double click.
    static const unsigned long RELEASE_DEBOUNCE_MS   = 35;
    static const unsigned long POST_RELEASE_GUARD_MS = 25;

    InputManager();

    void begin();
    void update();

    // --- Core Queries (automatically respect `consumed` state) ---
    bool wasPressed(ButtonId btn) const;   // True for 1 tick on debounced rising edge (LOW -> HIGH)
    bool isHeld(ButtonId btn) const;       // True while stably pressed and not consumed
    bool wasReleased(ButtonId btn) const;  // True for 1 tick on debounced falling edge (HIGH -> LOW)

    // --- Auto-Repeat (DAS / ARR) ---
    // Returns true on initial press AND every `repeatMs` after `initialDelayMs` of continuous hold
    bool isRepeated(ButtonId btn, unsigned long initialDelayMs = 220, unsigned long repeatMs = 120);

    // --- Aggregate Queries ---
    bool anyPressed() const;          // True if any unconsumed button was just pressed this tick
    bool anyHeld() const;             // True if any unconsumed button is currently held
    bool anyPhysicalActivity() const; // True if any button is physically pressed/held (for sleep/wake)

    // --- State Transition Protection ---
    // Marks all currently active buttons as consumed until physically released,
    // preventing button bleed across screen/state transitions without artificial lag.
    void consumeAll();

private:
    struct ButtonState {
        uint8_t pin;
        bool stableState;               // Debounced physical state (true = HIGH)
        bool justPressed;               // Rising edge flag for current tick
        bool justReleased;              // Falling edge flag for current tick
        bool consumed;                  // True if press belongs to a previous screen/state
        unsigned long pressStartTime;   // millis() when current press began
        unsigned long lastHighTime;     // millis() when raw HIGH was last observed
        unsigned long lastReleaseTime;  // millis() when release was confirmed
        unsigned long lastRepeatTime;   // millis() when isRepeated() last fired
    };

    ButtonState buttons[BTN_COUNT];
};

#endif // INPUTMANAGER_H
