
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
// NETWORK CONFIG (DHCP)
// ---------------------
byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x7A, 0x27 };

// HTTP server port (Prometheus will scrape this)
EthernetServer server(9100);

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println("Starting Ethernet (DHCP)...");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("DHCP failed. Check cable or router.");
    while (1) delay(1000);
  }

  Serial.print("My IP: ");
  Serial.println(Ethernet.localIP());

  dht.begin();
  server.begin();
  Serial.println("Exporter running on port 9100");
}

void loop() {
  EthernetClient client = server.available();
  if (!client) return;

  // wait for data
  while (client.connected() && !client.available()) {
    delay(1);
  }

  String req = client.readStringUntil('\r');
  client.read(); // read '\n'

  // Only react to GET /metrics
  if (req.startsWith("GET /metrics")) {
    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();

    // Handle DHT failure
    if (isnan(temp) || isnan(hum)) {
      temp = -1;
      hum  = -1;
    }

    // Build metrics
    String body;
    body += "arduino_temperature_celsius ";
    body += String(temp, 2);
    body += "\n";

    body += "arduino_humidity_percent ";
    body += String(hum, 2);
    body += "\n";

    // Send HTTP 200 + metrics
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain; version=0.0.4");
    client.print("Content-Length: ");
    client.println(body.length());
    client.println();
    client.print(body);

    Serial.println("Served /metrics:");
    Serial.println(body);
  } else {
    // Respond 404 to anything else
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Length: 0");
    client.println();
  }

  delay(1);
  client.stop();
}
