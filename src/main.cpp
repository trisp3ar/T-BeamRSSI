// T-Beam, display RSSI

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include "SSD1306.h"
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

using namespace std;

// LoRa Defs
#define SCK     5       // GPIO5  -- SX1278's SCK
#define MISO    19      // GPIO19 -- SX1278's MISnO
#define MOSI    27      // GPIO27 -- SX1278's MOSI
#define SS      18      // GPIO18 -- SX1278's CS
#define RST     14      // GPIO14 -- SX1278's RESET
#define DI0     26      // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND    866E6   // set Frequency

// Display Defs
SSD1306 display(0x3c, 21, 22);

const byte switchPin = 38;
byte oldSwitchState = HIGH;
int toggle;

byte localAddress;
byte destinationAddress;
long lastSendTime = 0;
int interval = 2000;
int count = 0;
int old_received = 0;
int ping = 1;

String rssi = "";
String SNR = "";
//string received_string = "";

void receiveMessage(int packetSize) {
    if (packetSize == 0) return;

    int recipient = LoRa.read();
    byte sender = LoRa.read();
    byte incomingLength = LoRa.read();

    String incoming = "";

    while (LoRa.available()) {
        incoming += (char)LoRa.read();
    }

    if (incomingLength != incoming.length()) {
        Serial.println("Error: Message length does not match length");
        return;
    }

    if (recipient != localAddress) {
        Serial.println("Error: Recipient address does not match local address");
        return;
    }

    Serial.print("Received data " + incoming);
    Serial.print(" from 0x" + String(sender, HEX));
    Serial.println(" to 0x" + String(recipient, HEX));
    rssi = "RSSI:   " + String(LoRa.packetRssi()) + " dBm";
    SNR = "SNR:     " + String(LoRa.packetSnr()) + " dB";
}

void sendMessage(String outgoing) {
    LoRa.beginPacket();
    LoRa.write(destinationAddress);
    LoRa.write(localAddress);
    LoRa.write(outgoing.length());
    LoRa.print(outgoing);
    LoRa.endPacket();
}

string LoRaLink(int received) {
    if (received == old_received) return "Missing Link";
    else return "LoRa Link";
    old_received = received;
}

void setup() {
    delay(1500);
    Serial.begin(9600);
    Serial.println("Start LoRa duplex");
    Serial.print("Local address: ");
    Serial.println(String(localAddress, HEX));
    Serial.print("Destination address: ");
    Serial.print(String(destinationAddress, HEX));

    // LoRa init
    SPI.begin(SCK,MISO,MOSI,SS);
        LoRa.setPins(SS,RST,DI0);
        if (!LoRa.begin(866E6)) {
        Serial.println("LoRa init failed. Check your connections.");
        while (true) {}
    }
    LoRa.setTxPower(14);                // EU: 25 mW ERP; NA: 20 dBm/100 mW ERP
    LoRa.setSignalBandwidth(31.25E3);   // 31.25E3 - fast, 125E3 - slow

    // Button init
    pinMode (switchPin, INPUT_PULLUP);

    // display init
    display.init();
    display.flipScreenVertically();  
    display.setFont(ArialMT_Plain_10);
}

void loop() {
    byte switchState = digitalRead (switchPin);

    // has it changed since last time?
    if (switchState != oldSwitchState)
    {
        oldSwitchState =  switchState;  // remember for next time 
        toggle ++;                      // toggle the variable
        delay (10);                     // debounce
        if (toggle >= 4) toggle = 0;
    }                                   // end of state change

    if (toggle == 0 || toggle == 1){
        // Sender 0xAA
        localAddress = 0xAA;
        destinationAddress = 0xBB;
    }
    if (toggle == 2 || toggle == 3){
        // Sender 0xBB
        localAddress = 0xBB;
        destinationAddress = 0xAA;
    }

    if (millis() - lastSendTime > interval)
    {
        //sendMessage(String(ping));
        sendMessage("Ping");

        Serial.print(" from source 0x" + String(localAddress, HEX));
        Serial.println(" to destination 0x" + String(destinationAddress, HEX));
        Serial.println(LoRa.packetRssi()) ;
        Serial.println(LoRa.packetSnr());

        lastSendTime = millis();
        interval = random(2000) + 100;
        if (ping < 255) ping++;
        else ping = 1;
    }

    // Displayausgabe
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    //display.drawString(0, 0, LoRaLink(incoming));
    display.drawString(0, 16, rssi);
    display.drawString(0, 32, SNR);
    display.drawString(0, 48, "Local:    " + String(localAddress) /*String(toggle)*/);
    display.display();

    receiveMessage(LoRa.parsePacket());
}