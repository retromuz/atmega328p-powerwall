# ATMEGA328P-powerwall

This project is intended to be used with below two projects.
1. https://github.com/retromuz/esp32-bms-monitor.git
2. https://github.com/retromuz/esp8266-powerwall.git

This is the brain behind the DC to DC boost converter that charges 14S Li-Ion battery system. Acts as an I²C slave for ESP8266 master. ESP8266 manages the MPPT function with data provided by ESP32 connected to BMS.


ATMEGA328P drives boost converter at 125kHz. Duty cycle is varied at real-time to maintain output voltage within 14S battery charge limits. It also adjusts the input voltage (from 48V solar panels) according to MPPT instructions coming from ESP8266 to extract maximum energy during the day.

![Powerwall - Web UI - Discharging](https://github.com/retromuz/atmega328p-powerwall/blob/master/photos/powerwall-web-ui-discharging.png?raw=true)
![Modding eBay 1500W boost converter - ATMEGA328P (I²C slave) shown with optocoupler and regulators](https://github.com/retromuz/atmega328p-powerwall/blob/master/photos/mod-boost-converter-atmega328p-shown.png?raw=true)
![Modding eBay 1500W boost converter - ESP8266 I²C master](https://github.com/retromuz/atmega328p-powerwall/blob/master/photos/mod-boost-converter-esp8266-shown.png?raw=true)
![Modding eBay 1500W boost converter - Calibrating before Deployment](https://github.com/retromuz/atmega328p-powerwall/blob/master/photos/mod-boost-converter-output-voltage-calibration.png?raw=true)