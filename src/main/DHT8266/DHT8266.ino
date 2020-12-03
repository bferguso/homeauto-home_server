#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor
#include "DHT.h"

#define DHTPIN 12     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

const char* ssid     = "ferginzeys secure";
const char* password = "h0w3S0undV13w";

// For sending data via web client
const char* quote = "%22";
const char* openBrace = "%7b";
const char* closeBrace = "%7d";

String envPublishHost = "192.168.102.178";
String envPublishUrl = "/homeServer/logEnv?envJson=";
String physicalLocation = "bedroom";

ESP8266WebServer server;

unsigned long delayTime;
unsigned long sendEnvDelayTime;

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
DHT dht(DHTPIN, DHTTYPE);

void setup() {

  Serial.begin(9600);
  Serial.println(F("DHT22 test!"));
  dht.begin();

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

  Serial.print("Location: ");
  Serial.println(physicalLocation);

  server.on("/", [](){server.send(200, "text/plain", getHelpMessage());});
  Serial.println("Registered path: /");

  server.on("/help", [](){server.send(200, "text/plain", getHelpMessage());});
  Serial.println("Registered path: /help");

  server.on("/updateConfig", updateConfig);
  Serial.println("Registered path: /updateConfig");

  server.begin();
  Serial.print("Server started!\n");

  Serial.println(getHelpMessage());
  delayTime = 1000;
  sendEnvDelayTime = 5000;
  Serial.println();
}

int value = 0;
unsigned long lastEnvSent = 0;

void loop() {
  server.handleClient();
  // Wait a few seconds between measurements.
  delay(delayTime);
  printTemp();
  if ((millis() - lastEnvSent) > sendEnvDelayTime) {
     sendEnv();
     lastEnvSent = millis();
  }
}

void printTemp()
{
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
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
  server.sendHeader("Access-Control-Allow-Origin", "*");
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
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return "";
  }

 // Compute heat index in Celsius (isFahreheit = false)
 float heatIndex = dht.computeHeatIndex(temperature, humidity, false);

  return openBrace
     +wrap("location")+":"+quote+physicalLocation+quote
     +","+wrap("temp")+":"+String(temperature)
     +","+wrap("humidity")+":"+String(humidity)
     +","+wrap("heatIndex")+":"+String(heatIndex)
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

