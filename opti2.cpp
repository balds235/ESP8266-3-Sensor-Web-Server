#include <DHT.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>

const char *ssid = "YOUR_SSID";
const char *password = "YOUR_PASSWORD";
#define DHTPIN D2
#define DHTTYPE DHT11
#define MOISTURE_PIN A0
#define TRIGGER_PIN D3
#define ECHO_PIN D4
#define SERVER_PORT 80

WebSocketsServer webSocket = WebSocketsServer(SERVER_PORT);
DHT dht(DHTPIN, DHTTYPE);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      Serial.printf("[%u] Connected from IP: %s\n", num, webSocket.remoteIP(num).toString().c_str());
      Serial.print("My IP address: ");
      Serial.println(WiFi.localIP());
      break;
  }
}

void handleClientRequest(WiFiClient client) {
  float temperature = dht.readTemperature();
  int moistureLevel = analogRead(MOISTURE_PIN);
  long duration, distance;
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;
  String response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: keep-alive\r\n\r\n";
  response += "<!DOCTYPE HTML>\r\n<html>\r\n";
  response += "<h1>Temperature: ";
  response += temperature;
  response += " °C</h1>\r\n";
  response += "<h2>Moisture Level: ";
  response += moistureLevel;
  response += "</h2>\r\n";
  response += "<h2>Distance: ";
  response += distance;
  response += " cm</h2>\r\n";
  response += "</html>\r\n";
  
  int numClients = webSocket.broadcastTXT(response);
  Serial.printf("Broadcasted to %d clients\n", numClients);
}

void setup() {
  Serial.begin(9600);
  delay(10);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());
  
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  dht.begin();
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  webSocket.loop();
  WiFiClient client = webSocket.available();
  
  if (client) {
    handleClientRequest(client);
  }
}