/*********************************************************************
 *********************************************************************/

#include "main.h"

volatile int pwm = MIN_PWM;
volatile unsigned int targetInput = MAX_TARGET_INPUT;
volatile unsigned int targetOutput = CHARGE_STOP_VOLTAGE_ADC;

volatile byte requestedAddr = I2C_ADDR_DEFAULT;
volatile unsigned int a3 = 0; // output voltage adc
volatile unsigned int a2 = 0; // input voltage adc
volatile unsigned int a1 = 0; // output current adc

volatile unsigned long input_current = 0;
volatile unsigned long output_current = 0;

volatile unsigned int loops = 0;
volatile unsigned int espRstLoops = 0;
volatile byte espResetting = 0;

volatile bool stopCharging = false;

void setup() {
	DDRC &= (1 << PC3); // ADC3 as input (output voltage measurement)
	DDRC &= (1 << PC2); // ADC2 as input (input voltage measurement)
	DDRC &= (1 << PC1); // ADC2 as input (output current sense)

	DDRD |= (1 << PD3); // PWM output
	TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
	TCCR2B = _BV(WGM22) | _BV(CS20); // CS21:clk/8 32.25kHz, CS20:no prescaling 250kHz
	setPWM();  // 160 clock periods = 10us per cycle
	OCR2A = 63;

	DDRD |= (1 << PD7); // ESP reset as output
	PORTD |= (1 << PD7); // turn on so that esp8266 does not reset.

	DDRB |= (1 << PB0); // LED 3
	DDRB |= (1 << PB1); // LED 4

	Wire.setClock(I2C_CLOCK);
	Wire.begin(I2C_ADDR);
	Wire.onReceive(i2cReceive);
	Wire.onRequest(i2cSend);
}

void loop() {
	unsigned int a3x = precisionAnalogRead(A3); // output voltage adc
	a2 = precisionAnalogRead(A2); // input voltage adc
	a1 = precisionAnalogRead(A1); // output current adc
	a3 = a3x - (a1 * 7 / 410);

	output_current = (unsigned long) (a1 * OUTPUT_CURRENT_ADC_TO_MA_RATIO); // mA
	unsigned long output_power = (unsigned long) (output_current
			* ((58.8 * a3) / 420)); // mW
	input_current = output_power / (a2 * 40 / 560); // mA

	// TODO: Output and input voltage drift compensation due to the resistance of the conductors

	if (stopCharging) {
		targetInput = MAX_TARGET_INPUT;
	}

	if (a2 < MIN_TARGET_INPUT || input_current > MAX_INPUT_CURRENT) {
		decPWM();
	} else {
		if (a3 > targetOutput || a2 < targetInput || ((a3 > CHARGE_STOP_VOLTAGE_ADC - 2) &&  output_current > MAX_OUTPUT_CURRENT_AT_SATURATION)
				|| ((a3 > CHARGE_STOP_VOLTAGE_ADC - 1) &&  output_current > MAX_OUTPUT_CURRENT_AT_SATURATION / 2)) {
			decPWM();
		} else if (a3 < targetOutput && a2 > targetInput) {
			incPWM();
		}
	}

	if (a3 > CHARGE_STOP_VOLTAGE_ADC && a1 < CHAGE_STOP_CURRENT_ADC) {
		stopCharging = true;
	}

	if (stopCharging && (a3 < CHARGE_RESTART_VOLTAGE_ADC)) {
		stopCharging = false;
		targetInput = TARGET_INPUT;
	}

	PORTB &= ~(1 << PB0);

	if (espResetting && espRstLoops > 4) {
		PORTD |= (1 << PD7); // reset esp high
		PORTB &= ~(1 << PB1); // turn off LED4
		espResetting = 0;
		espRstLoops = 0;
	} else if (espRstLoops > ESP_RESET_LOOPS_COUNT_THRESHOLD) {
		espRstLoops = 0;
		resetESP8266();
	}

	loops++;
	espRstLoops++;
}

void resetESP8266() {
	if (!espResetting) {
		PORTB |= (1 << PB1); // turn on LED4
		PORTD &= ~(1 << PD7); // reset esp low
		espResetting = 1;
	}
}

int precisionAnalogRead(uint8_t PORT) {
	uint8_t x = 0;
	int v = 0;
	while (x++ < PRECISION_ANALOG_READ_COUNT) {
		v = v + analogRead(PORT);
	}
	return v / PRECISION_ANALOG_READ_COUNT;
}

void i2cReceive(int k) {
	espRstLoops = 0;

	PORTB |= (1 << PB0);

	byte addr = Wire.read();

	if (I2C_ADDR_WRITE_TARGET_INPUT_ADC_VAL == addr) {
		int n = ((byte) Wire.read()) | ((byte) Wire.read()) << 8;
		if (!stopCharging && n >= MIN_TARGET_INPUT && n <= MAX_TARGET_INPUT) {
			targetInput = n;
		}
	} else if (I2C_ADDR_WRITE_TARGET_OUTPUT_ADC_VAL == addr) {
		int n = ((byte) Wire.read()) | ((byte) Wire.read()) << 8;
		if (n >= MIN_TARGET_OUTPUT && n <= MAX_TARGET_OUTPUT) {
			targetOutput = n;
		}
	} else {
		requestedAddr = addr;
	}
}

void i2cSend() {

	if (I2C_ADDR_READ_TARGET_INPUT_ADC_VAL == requestedAddr) {
		byte arr[2];
		arr[0] = targetInput & 0xff;
		arr[1] = (targetInput & 0xff00) >> 8;
		Wire.write(arr, 2);
	} else if (I2C_ADDR_READ_TARGET_OUTPUT_ADC_VAL == requestedAddr) {
		byte arr[2];
		arr[0] = targetOutput & 0xff;
		arr[1] = (targetOutput & 0xff00) >> 8;
		Wire.write(arr, 2);
	} else if (I2C_ADDR_READ_INPUT_ADC_VAL == requestedAddr) {
		byte arr[2];
		arr[0] = a2 & 0xff;
		arr[1] = (a2 & 0xff00) >> 8;
		Wire.write(arr, 2);
	} else if (I2C_ADDR_READ_OUTPUT_ADC_VAL == requestedAddr) {
		byte arr[2];
		arr[0] = a3 & 0xff;
		arr[1] = (a3 & 0xff00) >> 8;
		Wire.write(arr, 2);
	} else if (I2C_ADDR_READ_OUTPUT_CURRENT_ADC_VAL == requestedAddr) {
		byte arr[2];
		arr[0] = a1 & 0xff;
		arr[1] = (a1 & 0xff00) >> 8;
		Wire.write(arr, 2);
	} else if (I2C_ADDR_READ_OUTPUT_CURRENT_ESTIMATE == requestedAddr) {
		byte arr[3];
		arr[0] = output_current & 0xff;
		arr[1] = (output_current & 0xff00) >> 8;
		arr[2] = (output_current & 0xff0000) >> 16;
		Wire.write(arr, 3);
	} else if (I2C_ADDR_READ_INPUT_CURRENT_ESTIMATE == requestedAddr) {
		byte arr[3];
		arr[0] = input_current & 0xff;
		arr[1] = (input_current & 0xff00) >> 8;
		arr[2] = (input_current & 0xff0000) >> 16;
		Wire.write(arr, 3);
	} else if (I2C_ADDR_READ_MPPT_STATUS == requestedAddr) {
		Wire.write(0);
	} else if (I2C_ADDR_READ_PWM_VAL == requestedAddr) {
		Wire.write(pwm);
	} else if (I2C_ADDR_READ_FREQ == requestedAddr) {
		Wire.write(TCCR2B == (_BV(WGM22) | _BV(CS20)) ? 0 : 1);
	}
	requestedAddr = I2C_ADDR_DEFAULT;
}

void incPWM() {
	pwm = pwm < MAX_PWM ? pwm + 1 : pwm;
	setPWM();
}

void decPWM() {
	pwm = pwm > MIN_PWM ? pwm - 1 : pwm;
	setPWM();
}

void setPWM() {
	if (pwm >= MIN_PWM && pwm <= MAX_PWM) {
		if (pwm == -1) {
			DDRD &= ~(1 << PD3); // turn off output to set 0 duty cycle
		} else {
			DDRD |= (1 << PD3);
			OCR2B = pwm;
		}
	}
}
