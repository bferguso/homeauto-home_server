/*
 *  Simple HTTP get webclient test
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

/* BME280 includes */
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

uint8_t pin_led = 2;
uint8_t pin_relay = 14;

const char* ssid     = "ferginzeys secure";
const char* password = "h0w3S0undV13w";
//const char* ssid     = "ferginzeys";
//const char* password = "m0unta1nS1d3!";

const char* host = "wifitest.adafruit.com";

ESP8266WebServer server;

// BME280
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

unsigned long delayTime;

Adafruit_BME280 bme; // I2C

// END BME280

void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(pin_led, OUTPUT);
  pinMode(pin_relay, OUTPUT);
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected...");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Netmask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());

  server.on("/", [](){server.send(200, "text/plain", "Hello World!");});
  Serial.println("Registered path: /");

  server.on("/toggle", toggle);
  Serial.println("Registered path: /toggle");

  server.on("/on", ledOn);
  Serial.println("Registered path: /on");

  server.on("/off", ledOff);
  Serial.println("Registered path: /off");

  server.begin();
  Serial.print("Server started!");

  // BME
    unsigned status;

    // default settings
    status = bme.begin();
    // You can also pass in a Wire library object like &Wire2
    // status = bme.begin(0x76, &Wire2)
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x");
        Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        while (1) delay(10);
    }

    Serial.println("-- Default Test --");
    delayTime = 1000;

    Serial.println();
  // END BME
}

int value = 0;
unsigned long lastPrinted = 0;

void loop() {
   server.handleClient();
   if ((millis() - lastPrinted) > delayTime) {
      printEnv();
      lastPrinted = millis();
   }
   
/*
  delay(5000);
  ++value;

  Serial.print("Connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;

  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "/testwifi/index.html";
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(500);

  Serial.println("Before read...");
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println("After read...");
  Serial.println();
  Serial.println("closing connection");
  */
}

void setLedState(uint8_t newState) {
  digitalWrite(pin_led, newState);
  digitalWrite(pin_relay, newState);
  server.send(200, "text/plain", "Toggled to "+String(digitalRead(pin_led))+"\n");
  Serial.println("Pin LED: "+String(digitalRead(pin_led)));
  Serial.println("Pin Relay: "+String(digitalRead(pin_relay)));
}

void toggle() {
  setLedState(!digitalRead(pin_led));
  /*
  digitalWrite(pin_led, !digitalRead(pin_led));
  server.send(200, "text/plain", "Toggled to "+String(digitalRead(pin_led)));
  */
}

void ledOff() {
  setLedState(1);
}

void ledOn() {
  setLedState(0);
}

void printEnv() {
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");

    Serial.print("Pressure = ");

    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");

    Serial.println();
}





