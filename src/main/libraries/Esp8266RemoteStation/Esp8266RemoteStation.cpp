#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EnvData.h>
#include <Esp8266RemoteStation.h>
#include <JsonEncoder.h>

Esp8266RemoteStation::Esp8266RemoteStation(String physicalLocation) {
    _lastEnvSent = 0;
    _lastEnvPrinted = 0;
    _lastBufferFlush = 0;
    _httpPort = 80;
    _sendInterval = _defaultSendInterval;
    _printInterval = _defaultPrintInterval;
    _bufferInterval = _defaultBufferInterval;
    _physicalLocation = physicalLocation;
    Serial.begin(9600);
}

void Esp8266RemoteStation::initServer() {
    this->initServer(this->_defaultSsid, this->_defaultPassword);
}

void Esp8266RemoteStation::initServer(char *ssid, char *password) {
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

    _server.on("/", [this]() { _server.send(200, "text/plain", getHelpMessage()); });
    Serial.println("Registered path: /");

    _server.on("/help", [this]() { _server.send(200, "text/plain", getHelpMessage()); });
    Serial.println("Registered path: /help");

    _server.on("/updateConfig", std::bind(&Esp8266RemoteStation::updateConfig, this));
    Serial.println("Registered path: /updateConfig");

    _server.on("/getConfig", [this]() { _server.send(200, "text/plain", getConfig()); });
    Serial.println("Registered path: /getConfig");

    _server.begin();
    Serial.print("Server started!\n");

    Serial.println(getHelpMessage());
    Serial.println();
}

String Esp8266RemoteStation::getConfig() {
    return JsonEncoder::openBrace
           + JsonEncoder::wrapValue("location", _physicalLocation)
           + JsonEncoder::append(JsonEncoder::wrapValue("sendInterval", _sendInterval))
           + JsonEncoder::append(JsonEncoder::wrapValue("printInterval", _printInterval))
           + JsonEncoder::append(JsonEncoder::wrapValue("bufferInterval", _bufferInterval))
           + JsonEncoder::append(JsonEncoder::wrapValue("httpPort", _httpPort))
           + JsonEncoder::append(JsonEncoder::wrapValue("envPublishHost", _envPublishHost))
           + JsonEncoder::append(JsonEncoder::wrapValue("envPublishUrl", _envPublishUrl))
           + JsonEncoder::append(JsonEncoder::wrapValue("ip", String(WiFi.localIP().toString())))
           + JsonEncoder::append(JsonEncoder::wrapValue("netmask", String(WiFi.subnetMask().toString())))
           + JsonEncoder::append(JsonEncoder::wrapValue("gateway", String(WiFi.gatewayIP().toString())))
           + JsonEncoder::closeBrace;
}

void Esp8266RemoteStation::setPublishEndpoint(String host, String url)
{
    _envPublishHost = host;
    _envPublishUrl = url;
}

void Esp8266RemoteStation::registerServerUrl(String url, RemoteServerCallback *callback)
{
    _server.on(url, std::bind(&Esp8266RemoteStation::callbackWrapper, this, callback));
    Serial.println("Registered path: "+url);
}

void Esp8266RemoteStation::callbackWrapper(RemoteServerCallback *callback) {
    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.send(200, "application/json", callback->execute());
}

void Esp8266RemoteStation::updateConfig() {
    int status = 200;
    Serial.println(_server.args());
    String messages = "";
    bool didUpdate = false;
    for (short i = 0; i < _server.args(); i++) {
        Serial.print(_server.argName(i));
        Serial.print(" : ");
        Serial.println(_server.arg(i));
    }
    if (_server.hasArg("envPublishHost")) {
        _envPublishHost = _server.arg("envPublishHost");
        messages = messages + JsonEncoder::quoteString("Set envPublishHost to " + _envPublishHost);
        didUpdate = true;
    }
    if (_server.hasArg("envPublishUrl")) {
        _envPublishUrl = _server.arg("envPublishUrl");
        if (didUpdate) { messages = messages + JsonEncoder::valueSeparator; }
        messages = messages + JsonEncoder::quoteString("Set envPublishUrl to " + _envPublishUrl);
        didUpdate = true;
    }
    if (_server.hasArg("physicalLocation")) {
        _physicalLocation = _server.arg("physicalLocation");
        if (didUpdate) { messages = messages + JsonEncoder::valueSeparator; }
        messages = messages + JsonEncoder::quoteString("Set physicalLocation to " + _physicalLocation);
        didUpdate = true;
    }
    if (_server.hasArg("sendInterval")) {
        setSendInterval(_server.arg("sendInterval").toInt());
        if (didUpdate) { messages = messages + JsonEncoder::valueSeparator; }
        messages = messages + JsonEncoder::quoteString("Set sendInterval to " + String(_sendInterval));
        didUpdate = true;
    }
    if (_server.hasArg("printInterval")) {
        _printInterval = _server.arg("printInterval").toInt();
        if (didUpdate) { messages = messages + JsonEncoder::valueSeparator; }
        messages = messages + JsonEncoder::quoteString("Set printInterval to " + String(_printInterval));
        didUpdate = true;
    }
    if (_server.hasArg("bufferInterval")) {
        if (didUpdate) { messages = messages + JsonEncoder::valueSeparator; }
        if (setBufferInterval(_server.arg("bufferInterval").toInt()))
        {
            messages = messages + JsonEncoder::quoteString("Set bufferInterval to "+String(_bufferInterval));
        }
        else
        {
            status = 400;
            messages = messages + JsonEncoder::quoteString("Unable to set bufferInterval to "+_server.arg("bufferInterval"));
        }
        didUpdate = true;
    }
    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.send(status, "application/json", "{"+JsonEncoder::quoteString("messages")+":["+messages+"]}");
}

String Esp8266RemoteStation::getHelpMessage() {
    return String("Usage: \n") +
           "/getConfig : Get current configuration variables. Variables include:\n" +
           "/updateConfig : Update config variables. Variables include:\n" +
           "                physicalLocation : Description of location sent to server\n" +
           "                envPublishHost   : Host name to publish to\n" +
           "                envPublishUrl    : URL to publish to\n" +
           "                sendInterval     : Number of milliseconds between publishing environment\n" +
           "                printInterval    : Number of milliseconds between printing environment to Serial\n" +
           "                bufferInterval   : Number of milliseconds to buffer readings before sending to server\n" +
           "                eg: /updateConfig?physicalLocation=mech_rm&envPublishHost=192.168.102.110\n";
}

void Esp8266RemoteStation::sendEnv(EnvData data) {
    // If we haven't hit the interval time yet bypass send
    if (!readyToSendEnv()) {
        return;
    }

    // Now update the last timestamp we tried to send
    _lastEnvSent = millis();
    if (!bufferEnabled())
    {
        Serial.println("Buffer not enabled.");
        sendEnvToServer(data);
    }
    else
    {
        // First flush the buffer if it's full
        if (shouldFlushBuffer())
        {
            flushBufferedEnvData();
        }

        // Then try to buffer the data
        if (!bufferEnvData(&data))
        {
            sendEnvToServer(data);
        }
    }
}

void Esp8266RemoteStation::sendEnvToServer(EnvData data)
{
    Serial.print("Trying to send to ");
    Serial.println(_envPublishHost);
    Serial.println(_envPublishUrl);
    String url = _envPublishUrl + getEnvJsonEncoded(data);
    String response = sendHttpRequest(_envPublishHost, 80, url);
    Serial.println(response);
}

void Esp8266RemoteStation::setSendInterval(int sendInterval)
{
    _sendInterval = sendInterval;
}

/*
 * EnvData Buffering logic
 */
bool Esp8266RemoteStation::setBufferInterval(int bufferInterval)
{
    Serial.println("Checking if buffer should be flushed");
    if (!_envQueue.isEmpty())
    {
        Serial.println("Trying to flush buffer");
        flushBufferedEnvData();
    }

    // Create a new queue if we have to increase the size
    if (((int)(bufferInterval/_sendInterval))+1 <= _maxQueueSize )
    {
        _bufferInterval = bufferInterval;
        Serial.println("New buffer interval set to: "+String(_bufferInterval));
        return true;
    }
    else
    {
        Serial.println("New buffer interval is too large for max buffer size: "+
        String((int)(bufferInterval/_sendInterval)+1)+" > "+String(_maxQueueSize));
        return false;
    }
}

bool Esp8266RemoteStation::bufferEnabled()
{
    return _bufferInterval > 1000 && _bufferInterval > _sendInterval;
}

bool Esp8266RemoteStation::shouldFlushBuffer()
{
    return bufferEnabled() && (_envQueue.isFull() || (millis() - _lastBufferFlush) > _bufferInterval);
}

bool Esp8266RemoteStation::bufferEnvData(EnvData *data)
{

    if (shouldFlushBuffer())
    {
        Serial.println("Should be flushing, not buffering");
        return false;
    }
    if (!_envQueue.isInitialized())
    {
        Serial.println("Queue is not initialized");
        return false;
    }
    Serial.println("Current buffer size "+String(_envQueue.getCount()));
    data->millisOffset = millis();
    _envQueue.push(data);
    Serial.println("New buffer size "+String(_envQueue.getCount()));
    return true;
}

void Esp8266RemoteStation::flushBufferedEnvData()
{
    if (_bufferInterval == 0 || _envQueue.isEmpty())
    {
        Serial.println("Not trying to flush buffer");
        return;
    }
    Serial.println("Trying to flush buffer");
    String data = getBufferedEnvJson();
    if (data)
    {
        String *response = sendHttpPost(_envPublishHost, 80, _envPublishBufferedUrl, &data);
    }

    _lastBufferFlush = millis();
}

String Esp8266RemoteStation::sendHttpRequest(String host, int port, String request) { // Use WiFiClient class to create TCP connections
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
        response += line;
    }
    Serial.println("");
    Serial.println("After read...");
    Serial.println();
    Serial.println("closing connection");
    return response;
}

String* Esp8266RemoteStation::sendHttpPost(String host, int port, String url, String* payload) {
    // Use WiFiClient class to create TCP connections
    WiFiClient client;

    if (!client.connect(host, port)) {
        Serial.println("connection failed");
        return false;
    }
    Serial.println("Connected!");

    HTTPClient http;
    Serial.println("Connecting to "+host+url);
    http.begin("http://"+host+url);
    http.addHeader("Content-Type", "application/json");
    auto httpCode = http.POST(*payload);
    Serial.println(httpCode);
    delay(500);
    String responseString = http.getString();
    Serial.println(responseString);
    Serial.println("");
    Serial.println("After POST...");
    Serial.println();
    Serial.println("closing connection");
    return &responseString;
}

void Esp8266RemoteStation::handleClient() {
    _server.handleClient();
}

bool Esp8266RemoteStation::readyToSendEnv() {
    return ((millis() - _lastEnvSent) > _sendInterval);
}

bool Esp8266RemoteStation::readyToPrint() {
    return ((millis() - _lastEnvPrinted) > _printInterval);
}

void Esp8266RemoteStation::printEnv(EnvData data) {
    // If we haven't hit the interval time yet bypass send
    if (!readyToPrint()) {
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
    if (data.pressure) {
        Serial.print(F(" Pressure: "));
        Serial.print(data.pressure);
    }
    if (data.heatIndex) {
        Serial.print(F(" Heat index: "));
        Serial.print(data.heatIndex);
        Serial.print(F("°C"));
    }
    Serial.println();
}

String Esp8266RemoteStation::getEnvJsonEncoded(EnvData data) {
    return JsonEncoder::encodedOpenBrace
           + JsonEncoder::encodeValue("location", _physicalLocation)
           + JsonEncoder::append(JsonEncoder::encodeValue("temp", data.temperature))
           + JsonEncoder::append(JsonEncoder::encodeValue("humidity", data.humidity))
           + JsonEncoder::append(JsonEncoder::encodeValue("pressure", data.pressure))
           + JsonEncoder::append(JsonEncoder::encodeValue("heatIndex", data.heatIndex))
           + JsonEncoder::append(JsonEncoder::encodeValue("millisOffset", data.millisOffset))
           + JsonEncoder::encodedCloseBrace;
}

String Esp8266RemoteStation::getEnvJson(EnvData data) {
    return JsonEncoder::openBrace
           + JsonEncoder::wrapValue("location", _physicalLocation)
           + JsonEncoder::append(JsonEncoder::wrapValue("temp", data.temperature))
           + JsonEncoder::append(JsonEncoder::wrapValue("humidity", data.humidity))
           + JsonEncoder::append(JsonEncoder::wrapValue("pressure", data.pressure))
           + JsonEncoder::append(JsonEncoder::wrapValue("heatIndex", data.heatIndex))
           + JsonEncoder::append(JsonEncoder::wrapValue("millisOffset", data.millisOffset))
           + JsonEncoder::closeBrace;
}

String Esp8266RemoteStation::getBufferedEnvJson()
{
    String payload;
    EnvData data;
    String jsonEntry;

    if (_bufferInterval == 0 || _envQueue.isEmpty())
    {
        return F("");
    }

    payload = JsonEncoder::openBrace;
    payload += JsonEncoder::wrapValue("batch", true);
    payload += JsonEncoder::valueSeparator;
    payload += JsonEncoder::wrapValue("referenceMillis", millis());
    payload += JsonEncoder::valueSeparator;
    payload += JsonEncoder::quoteString("envReadings");
    payload += F(":");
    payload += F("[");
    bool first = true;
    Serial.println("Size of queue: "+String(_envQueue.getCount()));
    while (_envQueue.pop(&data))
    {
        if (! first )
        {
            payload += JsonEncoder::valueSeparator;
        }
        jsonEntry = getEnvJson(data);
        payload += jsonEntry;
        first = false;
    }
    payload += F("]");
    payload += JsonEncoder::closeBrace;
    return payload;
}
