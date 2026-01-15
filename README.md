# esphome-vaillant-x6  

ESPHome component for Vaillant heating boilers with the X6 interface.  


## Overview  

This ESPHome component allows you to read various operational parameters from Vaillant heating boilers equipped with the **X6 interface**. The component communicates with the boiler via UART and can be integrated into Home Assistant.  


## Features  

Continuously reads the sensor values from the boiler and sends them to Home Assistant.
Supported sensors can be found further below. Custom sensors can be added via the yaml configuration.


## Installation  

Add the `vaillant_x6` and a `uart` component to your ESPHome configuration.

```yaml
esphome:
  name: vaillantx6-hko123-web

esp32:
  board: esp32-c6-devkitc-1
  framework:
    type: esp-idf

external_components:
  - source: github://hko123/esphome-vaillant-x6
    components: [ vaillant_x6 ]
    refresh: 0s

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
  # Aktiviert das Captive Portal für einfache WiFi-Konfiguration
  ap:
    ssid: "Vaillant-X6-Fallback"
    password: "vaillant123"

# Captive Portal für WiFi-Konfiguration
captive_portal:

# Web Server aktivieren - Hauptkomponente für die Weboberfläche
web_server:
  port: 80
  version: 3
  #auth:
  #  username: !secret web_username
  #  password: !secret web_password
  local: true
  log: false
  ota: False
  include_internal: true

# API für Home Assistant Integration
#api:
#  encryption:
#    key: !secret api_encryption_key

# OTA Updates über Web-Interface
#ota:
#  - platform: esphome
#    password: !secret ota_password

# Logger
logger:
  level: INFO

mqtt:
  broker: !secret mqtt_host
  username: !secret mqtt_username
  password: !secret mqtt_password
  id: mqtt_vaillant

uart:
  id: my_uart
  tx_pin: GPIO06
  rx_pin: GPIO07
  baud_rate: 9600

vaillant_x6:
  uart_id: my_uart

  # Built-in Sensoren
  flow_temperature_sensor:
    name: Vorlauftemperatur
    icon: mdi:thermometer
    poll_interval: 10
    accuracy_decimals: 1
    unit_of_measurement: °C

  flow_target_temperature_sensor:
    name: Vorlauf Solltemperatur
    icon: mdi:thermometer-alert
    poll_interval: 60
    accuracy_decimals: 1
    unit_of_measurement: °C

  room_thermostat_flow_target_temperature_sensor:
    name: Raumthermostat Solltemperatur
    icon: mdi:thermometer-alert
    poll_interval: 60
    accuracy_decimals: 1
    unit_of_measurement: °C

  return_flow_temperature_sensor:
    name: Ruecklauftemperatur
    icon: mdi:thermometer
    poll_interval: 10
    accuracy_decimals: 1
    unit_of_measurement: °C

  outside_temperature_sensor:
    name: Aussentemperatur
    icon: mdi:home-thermometer
    poll_interval: 60
    accuracy_decimals: 1
    unit_of_measurement: °C

  tank_temperature_sensor:
    name: Speichertemperatur
    icon: mdi:thermometer-water
    poll_interval: 30
    accuracy_decimals: 1
    unit_of_measurement: °C

  tank_target_temperature_sensor:
    name: Speicher Solltemperatur
    icon: mdi:thermometer-alert
    poll_interval: 60
    accuracy_decimals: 1
    unit_of_measurement: °C

  hot_water_temperature_sensor:
    name: Warmwassertemperatur
    icon: mdi:thermometer-water
    poll_interval: 30
    accuracy_decimals: 1
    unit_of_measurement: °C

  hot_water_target_temperature_sensor:
    name: Warmwasser Solltemperatur
    icon: mdi:thermometer-alert
    poll_interval: 60
    accuracy_decimals: 1
    unit_of_measurement: °C
    
  circulating_pump_sensor:
    name: Umwaelzpumpe
    icon: mdi:pump
    poll_interval: 10

  burner_sensor:
    name: Brenner
    icon: mdi:fire
    poll_interval: 10

  gas_valve_sensor:
    name: Gasventil
    icon: mdi:valve
    poll_interval: 10

  # Custom Sensoren
  binary_sensors:
    - name: Pumpe Zielstatus
      icon: mdi:pump-off
      response_type: Status01
      command_byte: 0xa1
      response_length: 1
      poll_interval: 10

    - name: Zuendung
      icon: mdi:lightning-bolt
      response_type: Status0f
      command_byte: 0x49
      response_length: 1
      poll_interval: 10

  sensors:
    - name: Verbleibende Brennersperrzeit
      icon: mdi:timer-lock
      unit_of_measurement: min
      accuracy_decimals: 1
      device_class: duration
      response_type: AnalogueValue1Byte
      command_byte: 0x38
      response_length: 1
      poll_interval: 10

# Zusätzliche System-Sensoren für die Weboberfläche
sensor:
  - platform: uptime
    name: Betriebszeit
    icon: mdi:clock-outline
    update_interval: 60s

  - platform: wifi_signal
    name: WiFi Signal
    icon: mdi:wifi
    update_interval: 60s

text_sensor:
  - platform: wifi_info
    ip_address:
      name: IP Adresse
      icon: mdi:ip-network
    ssid:
      name: SSID
      icon: mdi:wifi
    bssid:
      name: BSSID
      icon: mdi:wifi-strength-2
    mac_address:
      name: MAC Adresse
      icon: mdi:network-outline

  - platform: version
    name: ESPHome Version
    icon: mdi:information-outline

# Status LED (optional)
status_led:
  pin:
    number: GPIO08
    inverted: false

# Restart Button über Weboberfläche
button:
  - platform: restart
    name: Neustart
    icon: mdi:restartbytes, only the first 2 response bytes will be interpreted
      command_byte: 0xb7
      response_length: 3
      poll_interval: 60                   # 60s, the default
```

You can also choose other GPIO pins for TX and RX on the ESP.


## Vaillant X6 Interface  

The **X6 interface** is a service port found on some older Vaillant boilers (for example on Vaillant ecoTEC classic VC 196/2 - C). It provides a simple 5V-UART communication interface for retrieving operational data.

<p align="center">
  <img src="./doc/vaillant-board.jpg" alt="Vaillant board"/>
</p>


### Connection  

To safely connect an ESP device to the boiler's X6 interface, a **galvanic isolation** is recommended to avoid electrical damage to both the ESP but more importantly to the circuit board of the boiler. This can be achieved using optocouplers. Also note, that the ESP uses 3,3V and the X6 interface operates on 5V. **Connecting the ESP directly to the X6 interface will damage your ESP immediately!**

<p align="center">
  <img src="./doc/schematic.png" alt="Schematic"/>
</p>

Simple optocouplers like the 6N139 invert the signal, therefore an inverter is necessary. In this schematic this is achieved with an IRLML5103 PNP Mosfet.

### X6 Port

```
+------------+
--- 24V      |
--- GND      +--+
--- TX          |
--- RX          |
--- 5V       +--+
---          |
+------------+
```
(When looking at the X6 port of the Vaillant circuit board from above.)


## Acknowledgments

Many insights for this project were taken from https://old.ethersex.de/index.php/Vaillant_X6_Schnittstelle. Without this valuable information, this project would not have been possible. A big thank you to the contributors of that documentation! 🙌 A copy of the website can be found [here](./doc/Vaillant%20X6%20Schnittstelle%20–%20Ethersex_Wiki.pdf).
