/*
  Esp8266RemoteStation.h - Library with standard functions provided by an ESP8266 remote station w/ environment
  sensor.
*/

#ifndef DHTReader_h
#define DHTReader_h

#include "DHT.h"
#include <EnvData.h>

class DHTReader {
   public:
      DHTReader(DHT* dht);
      EnvData readEnv();

   private:
      DHT* _dht;
};
#endif;