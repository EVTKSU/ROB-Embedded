#ifndef EVT_RC_H
#define EVT_RC_H

#include <Arduino.h>
#include <SBUS.h>

extern uint16_t channels[10]; // Global SBUS channel array


class EVT_RC
{
private:
    static bool sbusFailSafe;
    static bool sbusLostFrame; 
public:
    

EVT_RC();

/**
 * @brief Begin SBUS communication over Serial2
 */
    
 void setupSbus();
/**
 * @brief 
 * 
 * @return Condition denoting whether the SBUS was able to update
 */
bool updateSbusData();
};


#endif // EVT_RC_H