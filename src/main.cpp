/*********************************************************************
 *********************************************************************/

#include "main.h"

int max_pwm = 158;
int min_pwm = -1;
volatile int pwm = -1;
int a3 = 0;
int a2 = 0;
int targetOutput = 727; // relates to around 59V
int targetInput = 358; // 321:34V, 340:36V, 360:38V, 378:40V
int loops = 0;


void setup() {
	Serial.begin(115200);
	DDRC &= (1 << PC3); // ADC3 as input
	DDRC &= (1 << PC2); // ADC2 as input
	// PWM setup
	DDRD |= (1 << PD3);
	TCCR2A = 0x23;
	TCCR2B = 0x09; // mode 7, clock prescale by 1
	setPWM();  // 160 clock periods = 10us per cycle
	OCR2B = 0;
	TCNT2 = 0;
}

void loop() {
	int a3 = analogRead(A3);
	int a2 = analogRead(A2);
	_delay_ms(5);
	if (a3 < 5 || a2 < 5) {
		decPWM();
	} else {
		if (a3 > targetOutput || a2 < targetInput) {
			decPWM();
		} else if (a3 < targetOutput && a2 > targetInput) {
			incPWM();
		}
	}

	if (loops % 100 == 0) {
		Serial.printf("A2:%d, A3:%d, PWM:%d\r\n", a2, a3, pwm);
	}
	loops++;
}

void incPWM() {
	pwm = pwm < max_pwm ? pwm + 1 : pwm;
	setPWM();
}

void decPWM() {
	pwm = pwm > min_pwm ? pwm -1 : pwm;
	setPWM();
}

void setPWM() {
	if (pwm >= min_pwm && pwm <= max_pwm) {
		if (pwm == -1) {
			DDRD &= ~(1 << PD3);
		} else {
			DDRD |= (1 << PD3);
			OCR2A = 160 - pwm;
		}
	}
}
