/*
 *  Simple HTTP get webclient test
 */

#include "Esp8266RemoteStation.h"
#include "DHTReader.h"
#include "DHT.h"

uint8_t pin_led = 2;
uint8_t pin_range_hood_trigger = 13;

uint8_t analogSensor = A0;


char *ssid = "ferginzeys secure";
char *password = "h0w3S0undV13w";

int makeupFanHttpPort = 80;
char *makeupFanHost = "192.168.102.124";
const char *onService = "/on";
const char *offService = "/off";

unsigned long delayTime;
bool isMakeupFanOn = true;
bool rangeHoodIsOn = false;

DHT dht(12, DHT22);
DHTReader reader(&dht);
Esp8266RemoteStation espRemote("range_hood");

void setup() {
    Serial.begin(9600);
    dht.begin();

    espRemote.initServer(ssid, password);
    espRemote.setPublishEndpoint("cabin.local", "/homeServer/logEnv?envJson=");
    Serial.println("ESP Config "+espRemote.getConfig());

    delay(100);
    pinMode(pin_led, OUTPUT);
    pinMode(pin_range_hood_trigger, INPUT);
    delayTime = 500;
    //setMakeupActive(false);
}

EnvData data;

void loop() {
    delay(delayTime);
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
/*
  if (digitalRead(pin_range_hood_trigger))
  {
     setMakeupActive(true);
  }
  else
  {
     setMakeupActive(false);
  }
    //setRangeHoodPin();
*/
}

void setMakeupActive(bool turnOn) {
    const char *service;
    int newState = 0;

    if ((turnOn && isMakeupFanOn) || (!turnOn && !isMakeupFanOn)) {
        Serial.println("Noop - no change in state");
        return;
    }
    if (turnOn) {
        service = onService;
        newState = 0;
    } else {
        service = offService;
        newState = 1;
    }

    Serial.print("setMakeupActive Connecting to ");
    Serial.println(makeupFanHost);

    // Use WiFiClient class to create TCP connections
    WiFiClient client;

    char* host = makeupFanHost;
    int port = makeupFanHttpPort;

    if (!client.connect(makeupFanHost, makeupFanHttpPort)) {
        Serial.println("setMakeupActive connection failed");
        return;
    }


    String request = String("GET ") + service + " HTTP/1.1\r\n" +
    "Host: " + makeupFanHost + "\r\n" +
    "Connection: close\r\n\r\n";

    String response = espRemote.sendHttpRequest(makeupFanHost, makeupFanHttpPort, request);
    // Need to check for failure
    if (1 == 1)
    {
        digitalWrite(pin_led, newState);
    }



    Serial.println("Before read...");
    // Read all the lines of the reply from server and print them to Serial
    while (client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }
    Serial.println("");
    Serial.println("After read...");
    Serial.println();
    Serial.println("closing connection");
    isMakeupFanOn = turnOn;
}

float sampleInput() {
    float low = 10000;
    float high = 0;
    float next = 0;
    for (unsigned int n = 0; n < 2000; n++) {
        //next = analogRead(CURRENT_ANALOG_IN);
        next = analogRead(analogSensor);
        if (next > high) {
            high = next;
        }
        if (next < low) {
            low = next;
        }
    }
    Serial.print(F("Low: "));
    Serial.print(low);
    Serial.print(F(", High: "));
    Serial.println(high);
    return high;
}

bool isRangeHoodOn() {
    float sample = sampleInput();
    if (sample > 50) {
        Serial.println("Range hood is on, sample value is " + String(sample));
        return true;
    }
    return false;
}

void setRangeHoodPin() {
    bool isOn = isRangeHoodOn();
    if (isOn && !rangeHoodIsOn) {
        setMakeupActive(true);
        //digitalWrite(pin_send_trigger, 1);
        rangeHoodIsOn = true;
        Serial.println("Turning fan on");
    } else if (!isOn && rangeHoodIsOn) {
        setMakeupActive(false);
        //digitalWrite(pin_send_trigger, 0);
        rangeHoodIsOn = false;
        Serial.println("Turning fan off");
    }
/*
  Serial.println("Current Range Hood State "+String(digitalRead(pin_send_trigger)));
  */
}



