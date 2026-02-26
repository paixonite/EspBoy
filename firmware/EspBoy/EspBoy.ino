#include "EspBoyCore.h"

EspBoyCore systemCore;

void setup() {
    systemCore.begin();
}

void loop() {
    systemCore.update();
}