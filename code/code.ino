#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>

// ---------------------
// DHT SETTINGS
// ---------------------
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ---------------------
// NETWORK CONFIG
// ---------------------
byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x7A, 0x27 };

// Pushgateway hostname and port
const char* pushgatewayHost = "192.168.178.52";
const int pushgatewayPort = 9091;

// Pushgateway job URL
const char* pushPath = "/metrics/job/arduino-temperature";

EthernetClient client;

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println("Starting Ethernet...");

  // DHCP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("DHCP failed. Check your network.");
    while (true) { delay(1000); } // halt
  }

  delay(1000);
  Serial.print("My IP: ");
  Serial.println(Ethernet.localIP());

  dht.begin();
}

void loop() {
  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Failed to read from DHT11");
    delay(10000);
    return;
  }

  // Prepare metrics in Prometheus text format
  String body = "";
  body += "arduino_temperature_celsius ";
  body += String(temp, 2);
  body += "\n";

  body += "arduino_humidity_percent ";
  body += String(hum, 2);
  body += "\n";

  Serial.println("Connecting to Pushgateway...");

  if (client.connect(pushgatewayHost, pushgatewayPort)) {
    Serial.println("Connected, sending POST");

    client.print("POST ");
    client.print(pushPath);
    client.println(" HTTP/1.1");

    client.print("Host: ");
    client.println(pushgatewayHost);

    client.println("Content-Type: text/plain");
    client.print("Content-Length: ");
    client.println(body.length());

    client.println();
    client.print(body);

    Serial.println("Data sent:");
    Serial.println(body);
  } else {
    Serial.println("Connection failed");
  }

  // Read and print HTTP response
  delay(2000); // wait a bit for response
  while (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
  client.stop();

  // Send every 30 seconds
  delay(30 * 1000);
}
