// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _POWERWALL_MAIN_H_
#define _POWERWALL_MAIN_H_
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <Arduino.h>

#define F_CPU  16000000L

//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif

void setup();
void loop();
void incPWM();
void decPWM();
void setPWM();

#ifdef __cplusplus
} // extern "C"
#endif

//Do not add code below this line
#endif /* _POWERWALL_MAIN_H_ */

