/*********************************************************************
 *********************************************************************/

#include "main.h"

int max_pwm = 60;
int min_pwm = -1;
volatile int pwm = -1;
volatile int targetInput = 560; // 40V
//volatile int targetInput = 210; // temp
volatile int targetOutput = 425; // 59.0V

volatile byte requestedAddr = 255;
volatile int a3 = 0;
volatile int a2 = 0;
volatile int a1 = 0;

volatile int loops = 0;
volatile unsigned int espRstLoops = 0;
volatile byte espResetting = 0;

void setup() {
	Serial.begin(115200);
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

	Wire.setClock(100000);
	Wire.begin(I2C_ADDR);
	Wire.onReceive(i2cReceive);
	Wire.onRequest(i2cSend);
}

void loop() {
	a3 = precisionAnalogRead(A3);
	a2 = precisionAnalogRead(A2);
	a1 = precisionAnalogRead(A1);
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

	PORTB &= ~(1 << PB0);

	if (espResetting && espRstLoops > 1000) {
		PORTD |= (1 << PD7); // reset esp high
		PORTB &= ~(1 << PB1); // turn off LED4
		espResetting = 0;
		espRstLoops = 0;
	} else if (espRstLoops > 20000) {
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
		if (n >= MIN_TARGET_INPUT && n <= MAX_TARGET_INPUT) {
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
