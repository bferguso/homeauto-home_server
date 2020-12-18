#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EnvData.h>
#include <Esp8266RemoteStation.h>



Esp8266RemoteStation::Esp8266RemoteStation(String physicalLocation)
{
  _lastEnvSent = 0;
  _lastEnvPrinted = 0;
  _httpPort = 80;
  _sendInterval = 15000;
  _printInterval = 5000;
  _physicalLocation = physicalLocation;
  Serial.begin(9600);
}

void Esp8266RemoteStation::initServer(char* ssid, char* password)
{
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected...");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Netmask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());

  _server.on("/", [this](){_server.send(200, "text/plain", getHelpMessage());});
  Serial.println("Registered path: /");

  _server.on("/help", [this](){_server.send(200, "text/plain", getHelpMessage());});
  Serial.println("Registered path: /help");

  _server.on("/updateConfig", std::bind(&Esp8266RemoteStation::updateConfig, this));
  Serial.println("Registered path: /updateConfig");

  _server.on("/getConfig", [this](){_server.send(200, "text/plain", getConfig());});
  Serial.println("Registered path: /getConfig");

  _server.begin();
  Serial.print("Server started!\n");

  Serial.println(getHelpMessage());
  Serial.println();
}

String Esp8266RemoteStation::getConfig()
{
  return openBrace
     +wrap("location")+":"+wrap(_physicalLocation)
     +","+wrap("sendInterval")+":"+String(_sendInterval)
     +","+wrap("printInterval")+":"+String(_printInterval)
     +","+wrap("httpPort")+":"+String(_httpPort)
     +","+wrap("envPublishHost")+":"+String(_envPublishHost)
     +","+wrap("envPublishUrl")+":"+String(_envPublishUrl)
     +","+wrap("ip")+":"+String(WiFi.localIP().toString())
     +","+wrap("netmask")+":"+String(WiFi.subnetMask().toString())
     +","+wrap("gateway")+":"+String(WiFi.gatewayIP().toString())
     +closeBrace;
}

void Esp8266RemoteStation::setPublishEndpoint(String host, String url)
{
   _envPublishHost = host;
   _envPublishUrl = url;
}

void Esp8266RemoteStation::updateConfig()
{
  Serial.println(_server.args());
  String message = "";
  for (short i = 0; i < _server.args(); i++)
  {
     Serial.print(_server.argName(i));
     Serial.print(" : ");
     Serial.println(_server.arg(i));
  }
  if (_server.hasArg("envPublishHost"))
  {
      _envPublishHost = _server.arg("envPublishHost");
      message = message+"Set envPublishHost to "+_envPublishHost+"\n";
  }
  if (_server.hasArg("envPublishUrl"))
  {
      _envPublishUrl = _server.arg("envPublishUrl");
      message = message+"Set envPublishUrl to "+_envPublishUrl+"\n";
  }
  if (_server.hasArg("physicalLocation"))
  {
      _physicalLocation = _server.arg("physicalLocation");
      message = message+"Set physicalLocation to "+_physicalLocation+"\n";
  }
  if (_server.hasArg("sendInterval"))
  {
      _sendInterval = _server.arg("sendInterval").toInt();
      message = message+"Set sendInterval to "+_sendInterval+"\n";
  }
  if (_server.hasArg("printInterval"))
  {
      _printInterval = _server.arg("printInterval").toInt();
      message = message+"Set sendInterval to "+_printInterval+"\n";
  }
  _server.sendHeader("Access-Control-Allow-Origin", "*");
  _server.send(200, "text/plain", message);
}

String Esp8266RemoteStation::getHelpMessage()
{
  return String("Usage: \n")+
  "/getConfig : Get current configuration variables. Variables include:\n"+
  "/updateConfig : Update config variables. Variables include:\n"+
  "                physicalLocation : Description of location sent to server\n"+
  "                envPublishHost   : Host name to publish to\n"+
  "                envPublishUrl    : URL to publish to\n"+
  "                sendInterval     : Number of milliseconds between publishing environment\n"+
  "                printInterval    : Number of milliseconds between printing environment to Serial\n"+
  "                eg: /updateConfig?physicalLocation=mech_rm&envPublishHost=192.168.102.110\n";

}

void Esp8266RemoteStation::sendEnv(EnvData data)
{
    // If we haven't hit the interval time yet bypass send
    if ((millis() - _lastEnvSent) < _sendInterval) {
       return;
    }

    // Now update the last timestamp we tried to send
    _lastEnvSent = millis();

    // Use WiFiClient class to create TCP connections
    WiFiClient client;

    Serial.print("Trying to send to ");
    Serial.println(_envPublishHost);
    Serial.println(_envPublishUrl);
    if (!client.connect(_envPublishHost, _httpPort)) {
      Serial.println("Connection failed :(");
      return;
    }
    else
    {
      Serial.println("Connected!");
    }
    String url = _envPublishUrl + getEnvJson(data);
    Serial.println("Sending url "+url);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + _envPublishHost + "\r\n" +
                 "Connection: close\r\n\r\n");
   delay(500);

   Serial.println("Before read...");
    // Read all the lines of the reply from server and print them to Serial
   while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
   }
   Serial.println("After read...");
   Serial.println();
   Serial.println("closing connection");
}

String Esp8266RemoteStation::sendHttpRequest(char *host, int port, String request)
{
    // Use WiFiClient class to create TCP connections
    WiFiClient client;

    if (!client.connect(host, port)) {
        Serial.println("connection failed");
        return String("");
    }

    // This will send the request to the server
    client.print(String("GET ") + request + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    delay(500);

    Serial.println("Before read...");
    String response = String("");
    // Read all the lines of the reply from server and print them to Serial
    while (client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
        response = response + line;
    }
    Serial.println("");
    Serial.println("After read...");
    Serial.println();
    Serial.println("closing connection");
    return response;
}

void Esp8266RemoteStation::handleClient()
{
   _server.handleClient();
}

bool Esp8266RemoteStation::readyToSendEnv()
{
   return ((millis() - _lastEnvSent) > _sendInterval);
}

bool Esp8266RemoteStation::readyToPrint()
{
   return ((millis() - _lastEnvPrinted) > _printInterval);
}

void Esp8266RemoteStation::printEnv(EnvData data)
{
    // If we haven't hit the interval time yet bypass send
    if ((millis() - _lastEnvPrinted) < _printInterval) {
       return;
    }

    // Now update the last timestamp we tried to send
    _lastEnvPrinted = millis();
  Serial.print(F("Temperature: "));
  Serial.print(data.temperature);
  Serial.print(F("°C"));
  Serial.print(F(" Humidity: "));
  Serial.print(data.humidity);
  Serial.print(F("%"));
  if (data.pressure)
  {
      Serial.print(F(" Pressure: "));
      Serial.print(data.pressure);
  }
  if (data.heatIndex)
  {
      Serial.print(F(" Heat index: "));
      Serial.print(data.heatIndex);
      Serial.print(F("°C"));
  }
  Serial.println();
}

String Esp8266RemoteStation::getEnvJson(EnvData data)
{
  return openBrace
     +wrap("location")+":"+wrap(_physicalLocation)
     +","+wrap("temp")+":"+String(data.temperature)
     +","+wrap("humidity")+":"+String(data.humidity)
     +","+wrap("heatIndex")+":"+String(data.heatIndex)
     +closeBrace;
}

String Esp8266RemoteStation::wrap(String text)
{
   return quote+text+quote;
}
