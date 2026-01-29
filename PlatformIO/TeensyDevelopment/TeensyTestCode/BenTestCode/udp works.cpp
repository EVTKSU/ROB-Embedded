// src/main.cpp
#include <Arduino.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>

// ---------- CONFIG ----------
static byte MAC_ADDR[6] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Teensy static IP (must match your PC subnet)
static IPAddress TEENSY_IP(192, 168, 0, 177);
static IPAddress GATEWAY(192, 168, 0, 1);     // can be anything on same subnet if direct-link
static IPAddress SUBNET(255, 255, 255, 0);
static IPAddress DNS_IP(192, 168, 0, 1);      // not used for this test

// Your PC NIC IP
static IPAddress PC_IP(192, 168, 0, 10);

// Ports
static const uint16_t TEENSY_LISTEN_PORT = 5006; // Teensy listens here
static const uint16_t PC_LISTEN_PORT     = 5005; // Python listens here
// ----------------------------

static EthernetUDP Udp;
static uint32_t lastTxMs = 0;

static void printIp(const IPAddress& ip) {
  Serial.print(ip[0]); Serial.print(".");
  Serial.print(ip[1]); Serial.print(".");
  Serial.print(ip[2]); Serial.print(".");
  Serial.print(ip[3]);
}

void setup() {
  Serial.begin(115200);
  delay(1200);

  Serial.println();
  Serial.println("Teensy NativeEthernet UDP test starting");

  // Static network config
  Ethernet.begin(MAC_ADDR, TEENSY_IP, DNS_IP, GATEWAY, SUBNET);
  delay(200);

  Serial.print("Local IP: ");
  printIp(Ethernet.localIP());
  Serial.println();

  Serial.print("Link: ");
  auto ls = Ethernet.linkStatus();
  if (ls == LinkON) Serial.println("ON");
  else if (ls == LinkOFF) Serial.println("OFF");
  else Serial.println("UNKNOWN");

  if (Udp.begin(TEENSY_LISTEN_PORT)) {
    Serial.print("UDP listening on port ");
    Serial.println(TEENSY_LISTEN_PORT);
  } else {
    Serial.println("UDP begin failed");
  }

  lastTxMs = millis();
}

void loop() {
  // 1) Periodic TX to PC
  const uint32_t now = millis();
  if (now - lastTxMs >= 1000) {
    lastTxMs = now;

    char msg[96];
    snprintf(msg, sizeof(msg), "teensy_hello ms=%lu", (unsigned long)now);

    Udp.beginPacket(PC_IP, PC_LISTEN_PORT);
    Udp.write((const uint8_t*)msg, strlen(msg));
    Udp.endPacket();

    Serial.print("TX -> ");
    printIp(PC_IP);
    Serial.print(":");
    Serial.print(PC_LISTEN_PORT);
    Serial.print(" | ");
    Serial.println(msg);
  }

  // 2) RX and echo back
  int packetSize = Udp.parsePacket();
  if (packetSize > 0) {
    IPAddress remote = Udp.remoteIP();
    uint16_t remotePort = Udp.remotePort();

    static char rxbuf[512];
    int n = Udp.read(rxbuf, (int)sizeof(rxbuf) - 1);
    if (n < 0) n = 0;
    rxbuf[n] = '\0';

    Serial.print("RX <- ");
    printIp(remote);
    Serial.print(":");
    Serial.print(remotePort);
    Serial.print(" | ");
    Serial.println(rxbuf);

    // Echo back to sender
    Udp.beginPacket(remote, remotePort);
    Udp.write((const uint8_t*)rxbuf, (size_t)n);
    Udp.endPacket();

    Serial.print("ECHO -> ");
    printIp(remote);
    Serial.print(":");
    Serial.print(remotePort);
    Serial.println();
  }
}
