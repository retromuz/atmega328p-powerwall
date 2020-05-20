/*********************************************************************
 *********************************************************************/

#include "main.h"

int max_pwm = 60;
int min_pwm = -1;
volatile int pwm = -1;
volatile int targetInput = 540; // 38.8125V
volatile int targetOutput = 560; // 59.2V

volatile byte requestedAddr = 255;
volatile int a3 = 0;
volatile int a2 = 0;

int loops = 0;

void setup() {
	Serial.begin(115200);
	DDRC &= (1 << PC3); // ADC3 as input
	DDRC &= (1 << PC2); // ADC2 as input

	DDRD |= (1 << PD3);
	TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
	TCCR2B = _BV(WGM22) | _BV(CS20); // CS21:clk/8 32.25kHz, CS20:no prescaling 250kHz
	setPWM();  // 160 clock periods = 10us per cycle
	OCR2A = 63;

	Wire.setClock(100000);
	Wire.begin(I2C_ADDR);
	Wire.onReceive(i2cReceive);
	Wire.onRequest(i2cSend);
}

void loop() {
	_delay_ms(10);
	a3 = precisionAnalogRead(A3);
	a2 = precisionAnalogRead(A2);
	if (a2 < 142) {
		decPWM();
	} else {
		if (a3 > targetOutput || a2 < targetInput) {
			decPWM();
		} else if (a3 < targetOutput && a2 > targetInput) {
			incPWM();
		}
	}

	if (loops % 100 == 0) {
		Serial.printf("targetInput: %d, A2(In):%d, A3(Out):%d, PWM:%d\r\n",
				targetInput, a2, a3, pwm);
	}
	loops++;
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

	byte addr = Wire.read();

	if (I2C_ADDR_WRITE_TARGET_INPUT_ADC_VAL == addr) {
		int n = ((byte) Wire.read()) | ((byte) Wire.read()) << 8;
		if (n >= MIN_TARGET_INPUT && n <= MAX_TARGET_INPUT) {
			targetInput = n;
		}
	} else if (I2C_ADDR_WRITE_TARGET_OUTPUT_ADC_VAL == addr) {
		int n = ((byte) Wire.read()) | ((byte) Wire.read()) << 8;
		if (n >= MIN_TARGET_OUTPUT && n <= MAX_TARGET_OUTPUT) {
			targetOutput = n;
		}
	} else if (I2C_ADDR_WRITE_FREQ == addr) {
		int n = ((byte) Wire.read()) | ((byte) Wire.read()) << 8;
		if (n == 0) {
			TCCR2B = _BV(WGM22) | _BV(CS20);
		} else if (n == 1) {
			TCCR2B = _BV(WGM22) | _BV(CS21);
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
	} else if (I2C_ADDR_READ_PWM_VAL == requestedAddr) {
		Wire.write(pwm);
	} else if (I2C_ADDR_READ_FREQ == requestedAddr) {
		Wire.write(TCCR2B == (_BV(WGM22) | _BV(CS20)) ? 0 : 1);
	}
	requestedAddr = 0xff;
}

void incPWM() {
	pwm = pwm < max_pwm ? pwm + 1 : pwm;
	setPWM();
}

void decPWM() {
	pwm = pwm > min_pwm ? pwm - 1 : pwm;
	setPWM();
}

void setPWM() {
	if (pwm >= min_pwm && pwm <= max_pwm) {
		if (pwm == -1) {
			DDRD &= ~(1 << PD3); // turn off output to set 0 duty cycle
		} else {
			DDRD |= (1 << PD3);
			OCR2B = pwm;
		}
	}
}
