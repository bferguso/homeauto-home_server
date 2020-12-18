/*
  Esp8266RemoteStation.h - Library with standard functions provided by an ESP8266 remote station w/ environment
  sensor.
*/

#ifndef Esp8266RemoteStation_h
#define Esp8266RemoteStation_h

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EnvData.h>

class Esp8266RemoteStation {
   public:
      Esp8266RemoteStation(String physicalLocation);
      void initServer(char* ssid, char* password);
      void setPublishEndpoint(String host, String url);
      String getHelpMessage();
      void handleClient();
      void sendEnv(EnvData data);
      void printEnv(EnvData data);
      String sendHttpRequest(char *host, int port, String request);
      bool readyToSendEnv();
      bool readyToPrint();
      void updateConfig();
      String getConfig();

   private:
      const char* quote = "%22";
      const char* openBrace = "%7b";
      const char* closeBrace = "%7d";
      int _httpPort;
      String _envPublishHost = "cabin.local";
      String _envPublishUrl = "/homeServer/logEnv?envJson=";
      String _physicalLocation;
      int _sendInterval;
      int _printInterval;
      unsigned long _lastEnvSent;
      unsigned long _lastEnvPrinted;
      ESP8266WebServer _server;
      String wrap(String text);
      String getEnvJson(EnvData data);
};

#endif