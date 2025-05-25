#include <ESP32Servo.h>

// Create servo objects to control the steering and throttle servos
Servo steeringServo;  
Servo throttleServo;  

// Define input pins
const int thIn = 32;           // Throttle input pin from RC receiver
const int stIn = 33;           // Steering input pin from RC receiver
const int autoSwitchPin = 34;  // Autonomous mode switch input pin (PWM)
const int steeringPin = 25;    // Steering servo control pin
const int throttlePin = 26;    // Throttle servo control pin

// Autonomous mode flag
bool autonomousMode = false;

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);

  // Set input pins
  pinMode(thIn, INPUT);
  pinMode(stIn, INPUT);
  pinMode(autoSwitchPin, INPUT);

  // Attach the servos
  steeringServo.setPeriodHertz(50);               // Standard 50 Hz servo
  steeringServo.attach(steeringPin, 1000, 2000);  // Attach the steering servo
  throttleServo.setPeriodHertz(50);               // Standard 50 Hz servo
  throttleServo.attach(throttlePin, 1000, 2000);  // Attach the throttle servo

  Serial.println("Setup complete. Waiting for RC inputs...");
}

void loop() {
  // Read the PWM signal from the autonomous mode switch
  unsigned long autoSwitchValue = pulseIn(autoSwitchPin, HIGH, 25000);  // Read switch pulse width in microseconds

  // Output the raw switch value to the serial monitor
  Serial.print("Switch Input Value: ");
  Serial.println(autoSwitchValue);

  // Determine autonomous mode based on the switch value
  // Assuming a threshold of 1500 microseconds to determine high or low state
  autonomousMode = (autoSwitchValue > 1500);

  if (autonomousMode) {
    // Autonomous mode
    Serial.println("Autonomous mode active");

    // Preprogrammed throttle value for autonomous mode (fixed value 0-255)
    int throttlePos = 128;  // Example: medium speed throttle value (0-255)

    // Preprogrammed steering value for autonomous mode (adjustable)
    int steeringPos = 90;  // Neutral steering position

    // Continuously check switch to see if we should revert to manual mode
    while (autonomousMode) {
      // Check the switch value at every step
      autoSwitchValue = pulseIn(autoSwitchPin, HIGH, 25000);
      autonomousMode = (autoSwitchValue > 1500);

      if (!autonomousMode) {
        Serial.println("Switching back to manual mode");
        break;
      }

      // Example: set the steering and throttle positions
      steeringPos = 0;
      throttlePos = 35;

      // Write the positions to the servos
      throttleServo.write(throttlePos);
      steeringServo.write(steeringPos);

      Serial.print("Autonomous Throttle Position: ");
      Serial.println(throttlePos);
      Serial.print("Autonomous Steering Position: ");
      Serial.println(steeringPos);

      delay(100);  // Adjust delay for smoothness
    }
  } else {
    // Manual RC control mode
    Serial.println("Manual RC control mode");

    // Read the PWM signals from the RC receiver
    unsigned long thInValue = pulseIn(thIn, HIGH, 25000);  // Read throttle pulse width in microseconds
    unsigned long stInValue = pulseIn(stIn, HIGH, 25000);  // Read steering pulse width in microseconds

    // Map the throttle input to 0-180 degrees, handling reverse and forward
    int throttlePos;
    if (thInValue < 1500) {
      throttlePos = map(thInValue, 1000, 1500, 0, 90);  // Reverse range
    } else {
      throttlePos = map(thInValue, 1500, 2000, 90, 180);  // Forward range
    }

    // Map the steering input from 1000-2000 to 0-180 degrees
    int steeringPos = map(stInValue, 1000, 2000, 0, 180);

    // Print the raw input values and mapped positions to the serial monitor
    Serial.print("Throttle Input Value: ");
    Serial.println(thInValue);
    Serial.print("Mapped Throttle Position: ");
    Serial.println(throttlePos);
    Serial.print("Steering Input Value: ");
    Serial.println(stInValue);
    Serial.print("Mapped Steering Position: ");
    Serial.println(steeringPos);

    // Move the servos to the mapped positions
    throttleServo.write(throttlePos);
    steeringServo.write(steeringPos);

    // Small delay to make the output more readable
    delay(15);  // Adjust delay for smoothness
  }
}
