/*
  EnvData.h - DTO class for environmental data
*/
#ifndef ENV_DATA_H
#define ENV_DATA_H

class EnvData {
   public:
      EnvData();
      EnvData(float temp, float humid, float press);
      EnvData(float temp, float humid, float press, float alt, float heatIdx);
      unsigned long millisOffset = 0;
      float temperature = 0.0;
      float humidity = 0.0;
      float pressure = 0.0;
      float altitude = 0.0;
      float heatIndex = 0.0;
};

#endif