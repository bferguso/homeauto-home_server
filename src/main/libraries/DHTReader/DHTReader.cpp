#include "DHTReader.h"
#include "EnvData.h"

DHTReader::DHTReader(DHT* dht)
{
   _dht = dht;
}

EnvData DHTReader::readEnv()
{
   EnvData envData = EnvData();
   envData.temperature = _dht->readTemperature();
   envData.humidity = _dht->readHumidity();
   if (!isnan(envData.temperature) && !isnan(envData.humidity))
   {
      envData.heatIndex = _dht->computeHeatIndex(envData.temperature, envData.humidity, false);
   }
   else
   {
   Serial.println("Could not read from DHT");
   }
   return envData;
}
