#include "src/core/LoverBoyCore.h"

LoverBoyCore systemCore;

void setup() {
    systemCore.begin();
}

void loop() {
    systemCore.update();
}