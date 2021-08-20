// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _POWERWALL_MAIN_H_
#define _POWERWALL_MAIN_H_
#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>
#include <stdlib.h>
#include <Arduino.h>
#include <Wire.h>

#define F_CPU  8000000L

#define I2C_ADDR 0b00100010
#define I2C_CLOCK 100000
#define I2C_CMD_WRITE_TARGET_INPUT_ADC_VAL 0
#define I2C_CMD_WRITE_TARGET_OUTPUT_ADC_VAL 4
#define I2C_CMD_READ_TARGET_INPUT_ADC_VAL 1
#define I2C_CMD_READ_TARGET_OUTPUT_ADC_VAL 5
#define I2C_CMD_READ_INPUT_ADC_VAL 2
#define I2C_CMD_READ_OUTPUT_ADC_VAL 3
#define I2C_CMD_READ_PWM_VAL 6
#define I2C_CMD_READ_FREQ 8
#define I2C_CMD_READ_OUTPUT_CURRENT_ADC_VAL 10
#define I2C_CMD_READ_INPUT_CURRENT_ESTIMATE 11
#define I2C_CMD_READ_OUTPUT_CURRENT_ESTIMATE 12
#define I2C_CMD_READ_MPPT_STATUS 13
#define I2C_CMD_WRITE_NO_MPPT 14
#define I2C_CMD_DEFAULT 0xff

#define MIN_TARGET_INPUT 419 // 30V
#define MAX_TARGET_INPUT 700 // 50V
#define MAX_INPUT_CURRENT 30000 // 30A
#define MAX_OUTPUT_CURRENT_AT_SATURATION 5000 // 5A
#define CHAGE_STOP_CURRENT_ADC 1000 / 30 // 1000mA
#define CHARGE_STOP_VOLTAGE_ADC 420 // 416 = 58.2V (4.15V / cell)
#define CHARGE_RESTART_VOLTAGE_ADC 410 // 57.4V (4.1V / cell)
#define TARGET_INPUT 670 // 47.89V

#define OUTPUT_CURRENT_ADC_TO_MA_RATIO 30

#define MIN_TARGET_OUTPUT 200
#define MAX_TARGET_OUTPUT 900

#define TIMER_COUNTER_TOP 63 // 8MHz / 64 -> 125kHz PWM frequency
#define MAX_PWM 56 // 90% duty
#define MIN_PWM -1

#define MPPT_STEP 4
#define MPPT_STEP_DELAY_LOOPS 40

#define PRECISION_ANALOG_READ_COUNT 20
#define LOOPS_PER_SECOND 400
#define ESP_RESET_LOOPS_COUNT_THRESHOLD 8000

#define TIMSK     _SFR_IO8(0x37)

//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned int inAdc;
	unsigned int current;
} MPPTEntry;

typedef struct {
	MPPTEntry data[100];
	unsigned int status;
} MPPTData;

void setup();
void loop();
void regulate();
void incPWM();
void decPWM();
void setPWM();
void i2cReceive(int k);
void i2cSend();
int precisionAnalogRead(uint8_t PORT);
void resetESP8266();
void mpptScan(unsigned int statusesToCapture);
void mpptAnalyze(unsigned int statusesToCapture);

#ifdef __cplusplus
} // extern "C"
#endif

//Do not add code below this line
#endif /* _POWERWALL_MAIN_H_ */

