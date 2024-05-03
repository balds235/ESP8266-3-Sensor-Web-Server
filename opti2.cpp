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
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload,
                    size_t length) {
  switch (type) {
  case WStype_DISCONNECTED:
    Serial.printf("[%u] Disconnected!\n", num);
    break;
  case WStype_CONNECTED:
    Serial.printf("[%u] Connected from IP: %s\n", num,
                  webSocket.remoteIP(num).toString().c_str());
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
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println();
  client.print("Temperature: ");
  client.print(temperature);
  client.println(" °C");
  client.print("Moisture Level: ");
  client.println(moistureLevel);
  client.print("Distance: ");
  client.print(distance);
  client.println(" cm");
  int numClients =
      webSocket.broadcastTXT("Temperature: " + String(temperature) +
                             " °C, Moisture Level: " + String(moistureLevel) +
                             ", Distance: " + String(distance) + " cm");
  Serial.printf("Broadcasted to %d clients\n", numClients);
  client.stop();
  Serial.println("Client disconnected");
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
  Serial.println(Wifi.localIP())
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  | dht.begin();
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