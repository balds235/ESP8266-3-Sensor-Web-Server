#include <ESP8266WiFi.h>
const char *ssid = "SSID";
const char *password = "PASSWORD";
#define MOISTURE_PIN A0 // Analog pin connected to the moisture sensor
#define TRIGGER_PIN D3  // Digital pin connected to the HC-SR04 trigger
#define ECHO_PIN D4     // Digital pin connected to the HC-SR04 echo
#define SERVER_PORT 80
WiFiServer server(SERVER_PORT);
void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  server.begin();
  Serial.println("TCP server started");
  dht.begin();
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}
void handleClientRequest(WiFiClient client) {
  String request = client.readStringUntil('\r');
  Serial.println("Received request: " + request);
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
  client.print("Moisture Level: ");
  client.println(moistureLevel);
  client.print("Distance: ");
  client.print(distance);
  client.println(" cm");
  client.stop();
  Serial.println("Client disconnected");
}
void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected");
    handleClientRequest(client);
  }
}