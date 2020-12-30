/*
 * Library for BME280 sensor which reads the sensor and returns data in an EnvData object
*/

#ifndef BME280Reader_h
#define BME280Reader_h

/* BME280 includes */
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <EnvData.h>

class BME280Reader {
public:
    BME280Reader(Adafruit_BME280 *bme);

    EnvData readEnv();

private:
    Adafruit_BME280 *_bme;
};

#endif;