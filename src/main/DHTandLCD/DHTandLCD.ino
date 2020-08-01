#include <LiquidCrystal.h>
#include "EmonLib.h"
// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include "DHT.h"

#define DHTPIN 2     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

#define CURRENT_ANALOG_IN A1
int analogSensor = A1;

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
LiquidCrystal lcd(12, 11, 6, 5, 4, 3);


// Energy Monitor section
EnergyMonitor emon1;

uint8_t pin_send_trigger = 7;
bool rangeHoodIsOn = false;

void setup() {

  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));
  lcd.begin(16, 2);
  lcd.print("Found you!");
  dht.begin();
  emon1.current(analogSensor, 20.0);
  pinMode(pin_send_trigger, OUTPUT);
}

void loop() {
  // Wait a few seconds between measurements.
  delay(2000);
  printTemp();
  //delay(2000);
  //sampleInput();
  //printVoltage();
  setRangeHoodPin();
}

void setRangeHoodPin()
{
  bool isOn = isRangeHoodOn();
  if (isOn && !rangeHoodIsOn)
  {
    digitalWrite(pin_send_trigger, 1);
    rangeHoodIsOn = true;
    Serial.println("Turning fan on");
  }
  else if(!isOn && rangeHoodIsOn)
  {
    digitalWrite(pin_send_trigger, 0);
    rangeHoodIsOn = false;
    Serial.println("Turning fan off");
  }
/*
  Serial.println("Current Range Hood State "+String(digitalRead(pin_send_trigger)));
  */
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

void printVoltage()
{
  double irms = emon1.calcIrms(1480);
  emon1.serialprint();
  lcd.setCursor(0,0);
  lcd.print("Current ");
  lcd.print(irms*115.0);
  lcd.setCursor(0,1);
  lcd.print("This is not a test.");
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
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.print(t);
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("RelH: ");
  lcd.print(h);
  lcd.print("%");
}
