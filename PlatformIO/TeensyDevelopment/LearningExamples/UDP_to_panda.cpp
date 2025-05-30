#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>

// teensy ip stuffs
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 177);
EthernetUDP Udp;

// panda ip stuffs
static IPAddress telemetryDestIP(192, 168, 0, 10);  // Panda IP
static const uint16_t TELEMETRY_DEST_PORT = 5005;  // Matches receiver

char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 1000;  // Send every 1 second

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Starting NativeEthernet UDP Receiver...");

  Ethernet.begin(mac, ip);
  delay(1000);  // Allow time for hardware setup

  if (Ethernet.linkStatus() == LinkON) {
    Serial.println("Ethernet cable is connected.");
  } else {
    Serial.println("Ethernet cable is NOT connected.");
  }

  Udp.begin(8888);
  Serial.println("UDP listener started on port 8888");
}

void loop() {
  // --- Receive incoming UDP (optional) ---
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    int len = Udp.read(packetBuffer, sizeof(packetBuffer) - 1);
    if (len > 0) packetBuffer[len] = '\0';

    Serial.print("Received packet: ");
    Serial.println(packetBuffer);
  }

  // --- Send test telemetry string every second ---
  if (millis() - lastSendTime >= sendInterval) {
    lastSendTime = millis();

    const char* testTelemetry = "1,AUTO,540.2,0.34,49.1,47.9,13.2,12.5,1300,990";
    
    Udp.beginPacket(telemetryDestIP, TELEMETRY_DEST_PORT);
    Udp.write(testTelemetry);
    Udp.endPacket();

    Serial.println("Sent test telemetry");
  }
}
