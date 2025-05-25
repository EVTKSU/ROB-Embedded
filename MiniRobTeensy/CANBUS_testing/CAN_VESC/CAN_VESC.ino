#include <FlexCAN_T4.h>
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for serial port to connect (if needed)

  can1.begin();
  can1.setBaudRate(500000); 
}

void loop() {
  CAN_message_t msgTransmit, msgReceive;
  
  // Prepare the transmit message: CAN_PACKET_SET_CURRENT for controller with node id 114.
  // Example: frame_id = (packet_id << 8) | vesc_id
  // Here, 0x372 indicates packet id 0x3 and node id 0x72.
  msgTransmit.id  = 0x372; 
  msgTransmit.len = 4; // 4-byte payload
  
  // Set current value: for example, 125 means 12.5A when divided by 10.
  int32_t currentValue = 125;  
  // fills first 4 bytes of buf wth hex version of currentValue (split up)
  memcpy(msgTransmit.buf, &currentValue, 4); // Fill all 4 bytes
  
  // Transmit the message.
  can1.write(msgTransmit);
  
  // Wait for a received message (with a timeout)
  unsigned long startTime = millis();
  bool messageReceived = false;
  while ((millis() - startTime) < 100) { // wait up to 100 ms
    // The read() method returns true if a message is available.
    if (can1.read(msgReceive)) {
      messageReceived = true;
      break;
    }
  }
  
  if (messageReceived) {
    // Check if the received message is the one you expect.
    // For example, if the VESC is supposed to respond with id 0x872:
    if (msgReceive.id == 0x871) {
      // Combine the first 4 bytes into a 32-bit signed integer (FET temperature).
      int32_t tempInt = ((int32_t)msgReceive.buf[0]) |
                        (((int32_t)msgReceive.buf[1]) << 8) |
                        (((int32_t)msgReceive.buf[2]) << 16) |
                        (((int32_t)msgReceive.buf[3]) << 24);
      
      // Convert the raw value to degrees Celsius (assuming a scaling factor of 10).
      float temperature = tempInt / 10.0;
      
      Serial.print("FET Temp: ");
      Serial.print(temperature);
      Serial.println(" Â°C");
    }
    else {
      Serial.print("Unexpected message ID: 0x");
      Serial.println(msgReceive.id, HEX);
    }
  } 
  else {
    //Serial.println("No message received");
  }

  delay(100);
}
