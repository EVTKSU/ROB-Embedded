/*
  Teensy CAN Bus Duty Cycle Ramp Example for Flipsky VESC 75300
  ---------------------------------------------------------------
  This sketch demonstrates sending CAN messages to a Flipsky VESC 75300
  to command it to ramp up its motor duty cycle gradually until reaching 18%.
  
  The VESC protocol (as used by Flipsky VESCs) uses a command code to set duty:
    - Command code for "Set Duty Cycle" is assumed to be 0x05.
    - The duty cycle is expressed as a 32-bit signed integer equal to (duty * 1,000,000).
      For example, a duty cycle of 0.18 (18%) becomes: 0.18 * 1,000,000 = 180,000.
  
  The CAN message is built with an 8-byte payload:
    Byte 0: Command code (0x05)
    Bytes 1-4: 32-bit signed integer (big-endian) representing the duty cycle.
    Bytes 5-7: Reserved (set to 0).
  
  The CAN ID is built using a base ID. For the Flipsky VESC 75300,
  we assume a base CAN ID of 0x75300. (Adjust this as needed for your configuration.)
  The final extended CAN ID is set as: base_id OR'ed with the controller_id.
  
  This sketch uses the FlexCAN_T4 library on Teensy (e.g., Teensy 3.6/4.x).
  
  NOTE: Ensure that both the Teensy and the VESC are powered and that the CAN
  transceiver (e.g., SN65HVD232D) is correctly wired to the CAN bus.
  
  Debug print messages are sent via Serial.
*/

#include <FlexCAN_T4.h>

// Define the command code for "Set Duty Cycle"
#define COMM_SET_DUTY 0x05

// Create a FlexCAN_T4 object on CAN1 with increased buffer sizes.
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> CanBus;

// Global variable to track the current duty cycle (fractional value, 0.0 to 1.0)
float currentDuty = 0.0;
// Target duty cycle (18% duty cycle = 0.18)
const float targetDuty = 0.18;
// Ramp step (increase duty by this fraction on each cycle)
const float dutyStep = 0.005;  // Adjust for a smoother or faster ramp
// Delay between ramp steps in milliseconds
const unsigned long rampDelay = 200;

// For our example, we assume a single VESC controller with ID = 1
const uint8_t controller_id = 1;

// Define the base CAN ID for the Flipsky VESC 75300.
// (Adjust this value if your VESC is configured differently.)
const uint32_t base_id = 0x75300;

void setup() {
  // Start Serial communication for debugging.
  Serial.begin(115200);
  while (!Serial) {}  // Wait until the Serial monitor is ready
  Serial.println("Starting Flipsky VESC 75300 CAN Duty Cycle Ramp Example");

  // Initialize CAN pins and CAN bus
  Serial.println("Initializing CAN bus...");
  CanBus.begin();
  // Set baud rate to 250 kbps (adjust if needed)
  CanBus.setBaudRate(250000);
  // Enable FIFO buffering for incoming messages (if needed)
  CanBus.enableFIFO();
  Serial.println("CAN bus initialized.");

  // Print initial state
  Serial.print("Initial duty cycle: ");
  Serial.print(currentDuty * 100.0);
  Serial.println(" %");
}

void loop() {
  // Ramp up the duty cycle gradually until it reaches targetDuty
  if (currentDuty < targetDuty) {
    currentDuty += dutyStep;
    if (currentDuty > targetDuty) {
      currentDuty = targetDuty; // clamp to target if overshoot
    }
    Serial.print("Ramping up duty cycle to: ");
    Serial.print(currentDuty * 100.0, 2);  // print as percentage with 2 decimals
    Serial.println(" %");

    // Send the current duty cycle command via CAN
    sendVescSetDuty(controller_id, currentDuty);
    
    // Wait for a short delay between steps
    delay(rampDelay);
  } else {
    // Once reached target, send the command periodically to hold the duty cycle.
    Serial.print("Holding duty cycle at target: ");
    Serial.print(currentDuty * 100.0, 2);
    Serial.println(" %");
    
    sendVescSetDuty(controller_id, currentDuty);
    delay(1000);  // 1-second interval for test prints
  }
  
  // Optionally, check for any incoming CAN messages (for debugging)
  CAN_message_t rxMsg;
  while (CanBus.read(rxMsg)) {
    Serial.print("Received CAN message with ID: 0x");
    Serial.print(rxMsg.id, HEX);
    Serial.print(" Data: ");
    for (uint8_t i = 0; i < rxMsg.len; i++) {
      Serial.print(rxMsg.buf[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
}

/*
  sendVescSetDuty()
  -----------------
  This function builds and sends a CAN message to set the duty cycle on a VESC.
  
  Parameters:
    - controller_id: The ID of the VESC controller (to be OR'ed with the base CAN ID).
    - duty: Duty cycle as a fractional value (e.g., 0.18 for 18%).
  
  The duty value is converted to a 32-bit integer by multiplying by 1,000,000.
  The resulting value is sent in big-endian format.
*/
void sendVescSetDuty(uint8_t controller_id, float duty) {
  // Convert the floating-point duty cycle to an integer.
  // For example, 0.18 * 1,000,000 = 180,000.
  int32_t duty_int = (int32_t)(duty * 1000000.0);
  
  // Debug print the raw integer value being sent.
  Serial.print("Converted duty (int): ");
  Serial.println(duty_int);
  
  // Prepare an 8-byte payload:
  // Byte 0: Command code for set duty (0x05)
  // Bytes 1-4: 32-bit signed integer (duty_int) in big-endian order.
  // Bytes 5-7: Reserved, set to zero.
  uint8_t payload[8] = {0};
  payload[0] = COMM_SET_DUTY;
  payload[1] = (duty_int >> 24) & 0xFF;
  payload[2] = (duty_int >> 16) & 0xFF;
  payload[3] = (duty_int >> 8)  & 0xFF;
  payload[4] = duty_int & 0xFF;
  // payload[5], payload[6], payload[7] remain 0
  
  // Debug: Print the payload bytes in hex.
  Serial.print("Payload bytes: ");
  for (int i = 0; i < 8; i++) {
    if (payload[i] < 16) Serial.print("0");
    Serial.print(payload[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Construct the CAN message.
  CAN_message_t msg;
  // Build the extended CAN ID by combining base_id and controller_id.
  // (For example, if base_id is 0x75300 and controller_id is 1,
  // then msg.id = 0x75300 | 0x01.)
  msg.id = base_id | controller_id;
  msg.flags.extended = 1; // Use extended (29-bit) identifiers.
  msg.len = 8;
  memcpy(msg.buf, payload, 8);

  // Debug: Print the CAN ID being used.
  Serial.print("Sending CAN message with ID: 0x");
  Serial.println(msg.id, HEX);

  // Send the CAN message.
  if (CanBus.write(msg)) {
    Serial.println("Successfully sent Set Duty command.");
  } else {
    Serial.println("ERROR: Failed to send Set Duty command.");
  }
}
