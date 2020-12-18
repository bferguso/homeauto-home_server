#include "EnvData.h"

EnvData::EnvData()
{
}

EnvData::EnvData(float temp, float humid, float press)
{
   temperature = temp;
   humidity = humid;
   pressure = press;
}

EnvData::EnvData(float temp, float humid, float press, float alt, float heatIdx)
{
   temperature = temp;
   humidity = humid;
   pressure = press;
   altitude = alt;
   heatIndex = heatIdx;
}
