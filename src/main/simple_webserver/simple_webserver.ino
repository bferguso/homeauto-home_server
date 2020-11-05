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

const char* quote = "%22";
const char* openBrace = "%7b";
const char* closeBrace = "%7d";

const char* host = "wifitest.adafruit.com";
String envPublishHost = "192.168.102.128";
String envPublishUrl = "/homeServer/logEnv?envJson=";
String physicalLocation = "mechanical";

ESP8266WebServer server;

// BME280
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

unsigned long delayTime;
unsigned long sendEnvDelayTime;

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

  server.on("/", [](){server.send(200, "text/plain", getHelpMessage());});
  Serial.println("Registered path: /");

  server.on("/help", [](){server.send(200, "text/plain", getHelpMessage());});
  Serial.println("Registered path: /help");

  server.on("/toggle", relayToggle);
  Serial.println("Registered path: /toggle");

  server.on("/on", relayOn);
  Serial.println("Registered path: /on");

  server.on("/off", relayOff);
  Serial.println("Registered path: /off");

  server.on("/updateConfig", updateConfig);
  Serial.println("Registered path: /updateConfig");

  server.begin();
  Serial.print("Server started!\n");

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

    Serial.println(getHelpMessage());
    delayTime = 1000;
    sendEnvDelayTime = 5000;

    relayOff();
    Serial.println();
  // END BME
}

int value = 0;
unsigned long lastPrinted = 0;
unsigned long lastEnvSent = 0;

void loop() {
   server.handleClient();
   if ((millis() - lastPrinted) > delayTime) {
      printEnv();
      lastPrinted = millis();
   }

   if ((millis() - lastEnvSent) > sendEnvDelayTime) {
      sendEnv();
      lastEnvSent = millis();
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

void relayToggle() {
  setLedState(!digitalRead(pin_led));
  /*
  digitalWrite(pin_led, !digitalRead(pin_led));
  server.send(200, "text/plain", "Toggled to "+String(digitalRead(pin_led)));
  */
}

void relayOff() {
  setLedState(1);
}

void relayOn() {
  setLedState(0);
}

void updateConfig() {
  Serial.println(server.args());
  String message = "";
  for (short i = 0; i < server.args(); i++)
  {
     Serial.print(server.argName(i));
     Serial.print(" : ");
     Serial.println(server.arg(i));
  }
  if (server.hasArg("envPublishHost"))
  {
      envPublishHost = server.arg("envPublishHost");
      message = message+"Set envPublishHost to "+envPublishHost+"\n";
  }
  if (server.hasArg("envPublishUrl"))
  {
      envPublishUrl = server.arg("envPublishUrl");
      message = message+"Set envPublishUrl to "+envPublishUrl+"\n";
  }
  if (server.hasArg("physicalLocation"))
  {
      physicalLocation = server.arg("physicalLocation");
      message = message+"Set physicalLocation to "+physicalLocation+"\n";
  }
  server.send(200, "text/plain", message);
}

void sendEnv() {
  // Use WiFiClient class to create TCP connections
  WiFiClient client;

  Serial.print("Trying to send to ");
  Serial.println(envPublishHost);
  Serial.println(envPublishUrl);
  const int httpPort = 80;
  if (!client.connect(envPublishHost, httpPort)) {
    Serial.println("Connection failed :(");
    return;
  }
  else
  {
    Serial.println("Connected!");
  }
  String url = envPublishUrl + getEnvJson();
  Serial.println("Sending url "+url);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + envPublishHost + "\r\n" +
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
}

String getEnvJson() {
return openBrace
   +wrap("location")+":"+quote+physicalLocation+quote
   +","+wrap("temp")+":"+String(bme.readTemperature())
   +","+wrap("humidity")+":"+String(bme.readHumidity())
   +","+wrap("pressure")+":"+String(bme.readPressure()/100.0F)
   +","+wrap("alt")+":"+String(bme.readAltitude(SEALEVELPRESSURE_HPA))
   +closeBrace;
}
String wrap(String text)
{
   return quote+text+quote;
}

String getHelpMessage() {
  return String("Usage: \n")+
  "/toggle       : Toggles relay on or off\n"+
  "/on           : Turns relay on\n"+
  "/off          : Turns relay off\n"+
  "/updateConfig : Update config variables. Variables include:\n"+
  "                physicalLocation : Description of location sent to server\n"+
  "                envPublishHost   : Host name to publish to\n"+
  "                envPublishUrl    : URL to publish to\n"+
  "                eg: /updateConfig?physicalLocation=mech_rm&envPublishHost=192.168.102.110\n";

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





