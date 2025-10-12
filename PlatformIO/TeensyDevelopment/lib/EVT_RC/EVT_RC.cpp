#include "EVT_RC.hpp"


SBUS sbus(Serial2); // Create SBUS instance on Serial2
uint16_t channels[10] = {0};


EVT_RC::EVT_RC(){
    static bool sbusFailSafe = false;
    static bool sbusLostFrame = false;
}

void EVT_RC::setupSbus() {
    Serial2.begin(100000, SERIAL_8E2);
    sbus.begin();
    delay(500);
}

bool EVT_RC::updateSbusData() {
    return sbus.read(channels, &sbusFailSafe, &sbusLostFrame);
}






