#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 177);
EthernetUDP Udp;

char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

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
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    int len = Udp.read(packetBuffer, sizeof(packetBuffer) - 1);
    if (len > 0) packetBuffer[len] = '\0';

    Serial.print("Received packet: ");
    Serial.println(packetBuffer);
  }
}