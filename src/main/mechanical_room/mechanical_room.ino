/*
 *  Simple HTTP get webclient test
 */

#include "Esp8266RemoteStation.h"
#include "PressButtonCallback.h"

/* BME280 includes */
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "BME280Reader.h"

uint8_t pin_led = 2;
uint8_t pin_relay = 14;

char *ssid = "ferginzeys secure";
char *password = "h0w3S0undV13w";

String physicalLocation = "mechanical_room";

PressButtonCallback onCallback(pin_relay, 0);
PressButtonCallback offCallback(pin_relay, 1);

unsigned long delayTime;
unsigned long sendEnvDelayTime;

EnvData data;

Esp8266RemoteStation espRemote("mechanical_room2");

Adafruit_BME280 bme; // I2C
BME280Reader reader(&bme);

// END BME280

void setup() {
    Serial.begin(9600);
    // We start by connecting to a WiFi network

    espRemote.initServer(ssid, password);
    espRemote.setPublishEndpoint("cabin.local", "/homeServer/logEnv?envJson=");
    Serial.println("ESP Config "+espRemote.getConfig());

    delay(100);
    pinMode(pin_led, OUTPUT);
    pinMode(pin_relay, OUTPUT);

    espRemote.registerServerUrl("/on", &onCallback);
    espRemote.registerServerUrl("/off", &offCallback);

    // BME
    if (!bme.begin()) {
        // default settings
        // You can also pass in a Wire library object like &Wire2
        // status = bme.begin(0x76, &Wire2)
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x");
        Serial.println(bme.sensorID(), 16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        while (1) delay(10);
    }
    delayTime = 500;
    relayOff();
    // END BME
}

void loop() {
    //delay(delayTime);
    if (espRemote.readyToSendEnv() || espRemote.readyToPrint())
    {
        data = reader.readEnv();
        if (espRemote.readyToSendEnv())
        {
            espRemote.sendEnv(data);
        }
        if (espRemote.readyToPrint())
        {
            espRemote.printEnv(data);
        }
    }
    espRemote.handleClient();

}

void setLedState(uint8_t newState) {
    digitalWrite(pin_led, newState);
    digitalWrite(pin_relay, newState);
    Serial.println("Pin LED: " + String(digitalRead(pin_led)));
    Serial.println("Pin Relay: " + String(digitalRead(pin_relay)));
}

void relayOff() {
    setLedState(1);
}

void relayOn() {
    setLedState(0);
}

