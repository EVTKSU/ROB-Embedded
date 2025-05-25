#include <FlexCAN_T4.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;

int count = 0;

void setup() {
 Serial.begin(9600);
 while (!Serial);

 can1.begin();
 can1.setBaudRate(500000); 

}

void loop() {
CAN_message_t msgTransmit, msgReceive;
 
// adress of receiving device
// this iterates the transmit id each loop to brute force the intended id
  count ++;
  msgTransmit.id = count;
  Serial.print("CAN Message #: ");
  Serial.println(count);
  Serial.print("Transmit id: 0x");
  Serial.println(count, HEX);



  
 // this is the length (bytes) of can message
 msgTransmit.len = 4; 

 //example hex values (may set vesc motor current amps to 5)
 msgTransmit.buf[0] = 0x00;
 msgTransmit.buf[1] = 0x00; 
 msgTransmit.buf[2] = 0x13; 
 msgTransmit.buf[3] = 0x88;
 
 // writes message over can
 can1.write(msgTransmit);
 Serial.println("----------------------- teensy transmited data");
 
 // reads can
 can1.read(msgReceive);

// msgRecieve.id is the adress for the intended recipient
// this is set by sender (see msgTransmit.id) 
// this is checking to handle messages with this (teensy) device adress
 if(msgReceive.id == 0x871 || msgReceive.id == 0x872){
  Serial.println("======================================================================================");
  Serial.println("This message is for me! \nData: ");
  // print message
  for (int byte : msgReceive.buf){
    Serial.print(byte);
  }
  Serial.println();
  Serial.println("======================================================================================");
 }else if (msgReceive.id != 0){
    Serial.print("Unexpected message: ");
    Serial.print(msgReceive.id,HEX);

    Serial.println();
 }

 // this code works with no delay (goes really fuckin fast)
 // delay of 1 - 10 minimum recommended to keep can2usb software from crashing / freezing
 delay(200);
}