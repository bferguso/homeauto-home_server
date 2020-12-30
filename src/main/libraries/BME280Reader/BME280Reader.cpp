#include "BME280Reader.h"
#include "EnvData.h"

#define SEALEVELPRESSURE_HPA (1013.25)

BME280Reader::BME280Reader(Adafruit_BME280 *bme) {
    _bme = bme;
}

EnvData BME280Reader::readEnv() {
    EnvData envData = EnvData();
    envData.temperature = _bme->readTemperature();
    envData.humidity = _bme->readHumidity();
    envData.pressure = _bme->readPressure() / 100.0F;
    envData.altitude = _bme->readAltitude(SEALEVELPRESSURE_HPA);
    return envData;
}
