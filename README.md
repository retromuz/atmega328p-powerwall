# ATMEGA328P-powerwall

This project is intended to be used with below two projects.
1. https://github.com/retromuz/esp32-bms-monitor.git
2. https://github.com/retromuz/esp8266-powerwall.git

This is the brain behind the DC to DC boost converter that charges 14S Li-Ion battery system. It also does MPPT function. Acts as an I²C slave for ESP8266 master.


ATMEGA328P drives boost converter at 125kHz. Duty cycle is varied at real-time to maintain output voltage within 14S battery charge limits. It also adjusts the input voltage (from 48V solar panels) according to MPPT instructions coming from ESP8266 to extract maximum energy during the day.

![PCB](https://github.com/retromuz/atmega328p-powerwall/blob/master/schematics/V2-pcb-2D-view.png?raw=true)
![Schematics](https://github.com/retromuz/atmega328p-powerwall/blob/master/schematics/V2-schematic.png?raw=true)

