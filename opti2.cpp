#include <ESP8266WiFi.h>
const char *ssid = "SSID";
const char *password = "PASSWORD";
#define MOISTURE_PIN A0
#define TRIGGER_PIN D3
#define ECHO_PIN D4
#define SERVER_PORT 80
WiFiServer server(SERVER_PORT);
void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  server.begin();
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}
void handleClientRequest(WiFiClient client) {
  String request = client.readStringUntil('\r');
  int moistureLevel = analogRead(MOISTURE_PIN);
  long duration, distance;
  digitalWrite(TRIGGER_PIN, LOW);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.017;
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println();
  client.print("Moisture Level: ");
  client.println(moistureLevel);
  client.print("Distance: ");
  client.print(distance);
  client.println(" cm");
  client.stop();
}
void loop() {
  WiFiClient client = server.available();
  if (client) {
    handleClientRequest(client);
  }
}
