#include "EVT_RC.hpp"


SBUS sbus(Serial2); // Create SBUS instance on Serial2
uint16_t channels[10] = {0};


bool EVT_RC::sbusFailSafe = false;
bool EVT_RC::sbusLostFrame = false;
EVT_RC::EVT_RC(){
    
}

void EVT_RC::setupSbus() {
    Serial2.begin(100000, SERIAL_8E2);
    sbus.begin();
    delay(500);
}

bool EVT_RC::updateSbusData() {
    return sbus.read(channels, &sbusFailSafe, &sbusLostFrame);
}






