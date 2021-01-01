
// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor
#include "Esp8266RemoteStation.h"
#include "DHTReader.h"
#include "DHT.h"

#define DHTPIN 12     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

char *ssid = "ferginzeys secure";
char *password = "h0w3S0undV13w";

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHT22);
DHTReader reader(&dht);
Esp8266RemoteStation espRemote("bedroom");

EnvData data;

void setup() {

    Serial.begin(9600);
    dht.begin();

    espRemote.initServer(ssid, password);
    //espRemote.setPublishEndpoint("cabin.local", "/homeServer/logEnv?envJson=");
    Serial.println("ESP Config " + espRemote.getConfig());
}

void loop() {
    if (espRemote.readyToSendEnv() || espRemote.readyToPrint()) {
        data = reader.readEnv();
        if (espRemote.readyToSendEnv()) {
            espRemote.sendEnv(data);
        }
        if (espRemote.readyToPrint()) {
            espRemote.printEnv(data);
        }
    }
    espRemote.handleClient();
}
