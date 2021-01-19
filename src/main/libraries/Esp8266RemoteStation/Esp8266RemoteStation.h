/*
  Esp8266RemoteStation.h - Library with standard functions provided by an ESP8266 remote station w/ environment
  sensor.
*/

#ifndef Esp8266RemoteStation_h
#define Esp8266RemoteStation_h

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EnvData.h>
#include <RemoteServerCallback.h>
#include "cppQueue.h"

class Esp8266RemoteStation {
public:
    Esp8266RemoteStation(String physicalLocation);
    void initServer();
    void initServer(char *ssid, char *password);
    void setPublishEndpoint(String host, String url);
    bool setBufferInterval(int bufferInterval);
    void setSendInterval(int sendInterval);
    void registerServerUrl(String url, RemoteServerCallback *callback);
    String getHelpMessage();
    void handleClient();
    void sendEnv(EnvData data);
    void printEnv(EnvData data);
    String sendHttpRequest(String host, int port, String request);
    String* sendHttpPost(String host, int port, String url, String *payload);
    bool readyToSendEnv();
    bool readyToPrint();
    void updateConfig();
    String getConfig();

private:
    const int _maxQueueSize = 12 * 60 + 1; // 1h @ 5s per sample + 1

    // Default time in millis between sending readings to server
    const int _defaultSendInterval = 15000; // 15s

    // Default time in millis between printing readings to Serial
    const int _defaultPrintInterval = 5000; // 5s

    // Default buffer time in millis to buffer readings. 0 === no buffering
    const int _defaultBufferInterval = 0;

    int _httpPort;
    String _envPublishHost = "cabin.local";
    String _envPublishUrl = "/homeServer/logEnv?envJson=";
    String _envPublishBufferedUrl = "/homeServer/logBufferedEnv";
    char *_defaultSsid = "ferginzeys secure";
    char *_defaultPassword = "h0w3S0undV13w";
    String _physicalLocation;
    int _bufferInterval;
    int _sendInterval;
    int _printInterval;
    cppQueue _envQueue = cppQueue(sizeof(EnvData), _maxQueueSize, FIFO, false);
    unsigned long _lastBufferFlush;
    unsigned long _lastEnvSent;
    unsigned long _lastEnvPrinted;
    ESP8266WebServer _server;
    // Get Env data JSON, URL encoded
    String getEnvJsonEncoded(EnvData data);
    String getEnvJson(EnvData data);
    void callbackWrapper(RemoteServerCallback *callback);
    void sendEnvToServer(EnvData data);

    //
    // Reading Buffer functionality
    //
    bool bufferEnabled();
    bool shouldFlushBuffer();
    bool bufferEnvData(EnvData *data);
    void flushBufferedEnvData();
    // Get JSON for all buffered readings
    String getBufferedEnvJson();
};

#endif