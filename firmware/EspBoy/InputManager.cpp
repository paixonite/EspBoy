#include "InputManager.h"

static const uint8_t BUTTON_PINS[BTN_COUNT] = {
    BTN_UP,     // BTN_ID_UP
    BTN_DOWN,   // BTN_ID_DOWN
    BTN_LEFT,   // BTN_ID_LEFT
    BTN_RIGHT,  // BTN_ID_RIGHT
    BTN_A,      // BTN_ID_A
    BTN_B,      // BTN_ID_B
    BTN_START,  // BTN_ID_START
    BTN_SELECT  // BTN_ID_SELECT
};

InputManager::InputManager() {
    for (uint8_t i = 0; i < BTN_COUNT; i++) {
        buttons[i].pin             = BUTTON_PINS[i];
        buttons[i].stableState     = false;
        buttons[i].justPressed     = false;
        buttons[i].justReleased    = false;
        buttons[i].consumed        = false;
        buttons[i].pressStartTime  = 0;
        buttons[i].lastHighTime    = 0;
        buttons[i].lastReleaseTime = 0;
        buttons[i].lastRepeatTime  = 0;
    }
}

void InputManager::begin() {
    unsigned long now = millis();
    for (uint8_t i = 0; i < BTN_COUNT; i++) {
        pinMode(buttons[i].pin, INPUT_PULLDOWN);
        bool raw = (digitalRead(buttons[i].pin) == HIGH);
        buttons[i].stableState     = raw;
        buttons[i].justPressed     = false;
        buttons[i].justReleased    = false;
        // If a button is held during boot, mark it consumed until released
        buttons[i].consumed        = raw;
        buttons[i].pressStartTime  = now;
        buttons[i].lastHighTime    = now;
        buttons[i].lastReleaseTime = 0;
        buttons[i].lastRepeatTime  = 0;
    }
}

void InputManager::update() {
    unsigned long now = millis();

    for (uint8_t i = 0; i < BTN_COUNT; i++) {
        ButtonState& b = buttons[i];
        bool raw = (digitalRead(b.pin) == HIGH);

        b.justPressed  = false;
        b.justReleased = false;

        if (!b.stableState) {
            // Currently RELEASED: trigger immediately on rising edge if past post-release guard
            if (raw && (b.lastReleaseTime == 0 || (now - b.lastReleaseTime >= POST_RELEASE_GUARD_MS))) {
                b.stableState    = true;
                b.justPressed    = true;
                b.consumed       = false;
                b.pressStartTime = now;
                b.lastHighTime   = now;
                b.lastRepeatTime = 0;
            }
        } else {
            // Currently PRESSED: keep state HIGH until pin stays continuously LOW for RELEASE_DEBOUNCE_MS
            if (raw) {
                b.lastHighTime = now;
            } else if (now - b.lastHighTime >= RELEASE_DEBOUNCE_MS) {
                b.stableState     = false;
                b.justReleased    = !b.consumed;
                b.consumed        = false;
                b.lastReleaseTime = now;
                b.lastRepeatTime  = 0;
            }
        }
    }
}

bool InputManager::wasPressed(ButtonId btn) const {
    if (btn >= BTN_COUNT) return false;
    return buttons[btn].justPressed && !buttons[btn].consumed;
}

bool InputManager::isHeld(ButtonId btn) const {
    if (btn >= BTN_COUNT) return false;
    return buttons[btn].stableState && !buttons[btn].consumed;
}

bool InputManager::wasReleased(ButtonId btn) const {
    if (btn >= BTN_COUNT) return false;
    return buttons[btn].justReleased;
}

bool InputManager::isRepeated(ButtonId btn, unsigned long initialDelayMs, unsigned long repeatMs) {
    if (btn >= BTN_COUNT) return false;
    ButtonState& b = buttons[btn];

    if (!b.stableState || b.consumed) return false;

    if (b.justPressed) {
        b.lastRepeatTime = 0;
        return true;
    }

    unsigned long now = millis();
    if (b.lastRepeatTime == 0) {
        if (now - b.pressStartTime >= initialDelayMs) {
            b.lastRepeatTime = now;
            return true;
        }
    } else {
        if (now - b.lastRepeatTime >= repeatMs) {
            b.lastRepeatTime = now;
            return true;
        }
    }

    return false;
}

bool InputManager::anyPressed() const {
    for (uint8_t i = 0; i < BTN_COUNT; i++) {
        if (buttons[i].justPressed && !buttons[i].consumed) {
            return true;
        }
    }
    return false;
}

bool InputManager::anyHeld() const {
    for (uint8_t i = 0; i < BTN_COUNT; i++) {
        if (buttons[i].stableState && !buttons[i].consumed) {
            return true;
        }
    }
    return false;
}

bool InputManager::anyPhysicalActivity() const {
    for (uint8_t i = 0; i < BTN_COUNT; i++) {
        if (buttons[i].stableState || buttons[i].justPressed) {
            return true;
        }
    }
    return false;
}

void InputManager::consumeAll() {
    unsigned long now = millis();
    for (uint8_t i = 0; i < BTN_COUNT; i++) {
        ButtonState& b = buttons[i];
        b.justPressed    = false;
        b.justReleased   = false;
        b.lastRepeatTime = 0;

        if (b.stableState || digitalRead(b.pin) == HIGH) {
            b.stableState  = true;
            b.lastHighTime = now;
            b.consumed     = true;
        }
    }
}
