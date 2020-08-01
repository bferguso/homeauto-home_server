/*
 *  Simple HTTP get webclient test
 */

#include <ESP8266WiFi.h>

uint8_t pin_led = 2;
uint8_t pin_range_hood_trigger = 13;

uint8_t analogSensor = A0;


//const char* ssid     = "ferginzeys";
//const char* password = "m0unta1nS1d3!";
const char* ssid     = "ferginzeys secure";
const char* password = "h0w3S0undV13w";

const char* host = "10.0.0.99";
const char* toggleService = "/toggle";
const char* onService = "/on";
const char* offService = "/off";

unsigned long delayTime;
bool isMakeupFanOn = true;
bool rangeHoodIsOn = false;

void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(pin_led, OUTPUT);
  pinMode(pin_range_hood_trigger, INPUT);
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

  Serial.print("Server started!");
  delayTime = 500;
  Serial.println();
  setMakeupActive(false);
}

int value = 0;
unsigned long lastPrinted = 0;

void loop() {
  delay(delayTime);
  ++value;
/*
  if (digitalRead(pin_range_hood_trigger))
  {
     setMakeupActive(true);
  }
  else
  {
     setMakeupActive(false);
  }
*/
  setRangeHoodPin();
}

void setMakeupActive(bool turnOn)
{
  const char* service;
  int newState = 0;

  if ((turnOn && isMakeupFanOn) || (!turnOn && !isMakeupFanOn))
  {
    Serial.println("Noop - no change in state");
    return;
  }
  if (turnOn)
  {
     service = onService;
     newState = 0;
  }
  else
  {
     service = offService;
     newState = 1;
  }

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
  //String service = "/toggle";
  Serial.print("Requesting URL: ");
  Serial.println(service);

  // This will send the request to the server
  client.print(String("GET ") + service + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  digitalWrite(pin_led, newState);
  delay(500);

  Serial.println("Before read...");
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println("");
  Serial.println("After read...");
  Serial.println();
  Serial.println("closing connection");
  isMakeupFanOn = turnOn;
}

float sampleInput()
{
  float low = 10000;
  float high = 0;
  float next = 0;
 for (unsigned int n = 0; n < 2000; n++)
 {
    //next = analogRead(CURRENT_ANALOG_IN);
    next = analogRead(analogSensor);
    if (next > high)
    {
       high = next;
    }
    if (next < low)
    {
      low = next;
    }
 }
 Serial.print(F("Low: "));
 Serial.print(low);
 Serial.print(F(", High: "));
 Serial.println(high);
 return high;
}

bool isRangeHoodOn()
{
   float sample = sampleInput();
   if (sample > 50)
   {
       Serial.println("Range hood is on, sample value is "+String(sample));
       return true;
   }
   return false;
}

void setRangeHoodPin()
{
  bool isOn = isRangeHoodOn();
  if (isOn && !rangeHoodIsOn)
  {
    setMakeupActive(true);
    //digitalWrite(pin_send_trigger, 1);
    rangeHoodIsOn = true;
    Serial.println("Turning fan on");
  }
  else if(!isOn && rangeHoodIsOn)
  {
    setMakeupActive(false);
    //digitalWrite(pin_send_trigger, 0);
    rangeHoodIsOn = false;
    Serial.println("Turning fan off");
  }
/*
  Serial.println("Current Range Hood State "+String(digitalRead(pin_send_trigger)));
  */
}



