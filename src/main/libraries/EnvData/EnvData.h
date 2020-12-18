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
      float temperature;
      float humidity;
      float pressure;
      float altitude;
      float heatIndex;
};

#endif