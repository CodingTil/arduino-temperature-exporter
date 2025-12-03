# Arduino Temperature Logger
Records temperature measurements from the DHT11 sensor and forwards them to Prometheus.

## Getting Started
Install the `arduino-cli`, along with the dependencies:

```sh
arduino-cli config init

arduino-cli core update-index
arduino-cli core install arduino:avr

arduino-cli lib install "Ethernet"
arduino-cli lib install "DHT sensor library"
```

Connect the board to your computer and check it is recognized:
```sh
arduino-cli board list
```

## Compile and Upload the Sketch
```sh
arduino-cli compile --fqbn arduino:avr:uno code

arduino-cli upload \
  --port /dev/ttyACM0 \
  --fqbn arduino:avr:uno \
  my_prometheus_dht
```

To monitor the output:
```sh
arduino-cli monitor -p /dev/ttyACM0 -c 9600
```

# Requirements for Prometheus
Setup a [pushgateway](https://github.com/prometheus/pushgateway). In `code/code.ino` adapt the `pushgatewayHost` variable.

Add the pushgateway to the scrapeconfig of Prometheus (e.g., in `/etc/prometheus/prometheus.yml`):
```yaml
scrape_configs:
  - job_name: "pushgateway"
    static_configs:
      - targets: ["localhost:9091"] # Adapt if necessary
```
