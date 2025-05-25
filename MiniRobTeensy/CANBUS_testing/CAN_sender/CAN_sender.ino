#include <FlexCAN_T4.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;


void setup() {
 Serial.begin(9600);
 while (!Serial);

 can1.begin();
 can1.setBaudRate(500000); 
}

void loop() {
 CAN_message_t msgTransmit, msgReceive;
 
// adress of receiving device
 msgTransmit.id = 0x82; 
 // this is the length (bytes) of can message
 msgTransmit.len = 4; 
 //individual can data bytes
 msgTransmit.buf[0] = 0x01;
 msgTransmit.buf[1] = 0x02;
 msgTransmit.buf[2] = 0x03;
 msgTransmit.buf[3] = 0x04; 
 // writes message over can
 can1.write(msgTransmit);
 Serial.println("Sent Message");

delay(1000);
}