# ATMEGA328P-powerwall

This project is intended to be used with below two projects.
1. https://github.com/retromuz/esp32-bms-monitor.git
2. https://github.com/retromuz/esp8266-powerwall.git

This is the brain behind the DC to DC boost converter that charges 14S Li-Ion battery system. It also does MPPT function. Acts as an I²C slave for ESP8266 master.


ATMEGA328P drives boost converter at 125kHz. Duty cycle is varied at real-time to maintain output voltage within 14S battery charge limits. It also adjusts the input voltage (from 48V solar panels) in order to extract maximum energy during the day.

V2 board adds additional functionality on top of function V1 board. These are:
1. Input surge protection (from transient spikes due to lightning etc)
2. Better voltage regulation
3. Better filtering
4. Even more efficient conversion due to higher current carrying capacity of power traces
5. Removed unwanted DACs from design
6. Output current sensing
7. Atmega328p on-board MPPT (works even without ESP8266)

![PCB](https://github.com/retromuz/atmega328p-powerwall/blob/master/schematics/V2-pcb-2D-view.png?raw=true)
![Schematics](https://github.com/retromuz/atmega328p-powerwall/blob/master/schematics/V2-schematic.png?raw=true)

