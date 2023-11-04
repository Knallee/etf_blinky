/*
 * adc.c
 *
 * Created: 10/15/2023 8:42:59 PM
 *  Author: ccederberg
 */ 

#include "adc.h"


void adc_init() {
	
	// Setting the adc channel to audio output from op-amp. REF is VCC.
	ADMUX |= (1 << MUX0);
	
	// Setting adc clock frequency to 125 kHz, 200 kHz is maximum.
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1); // prescaler to 64
	
	// Eabling the ADC
	ADCSRA |= (1 << ADEN);
}

void adc_start() {
	ADCSRA |= (1 << ADSC);
}

uint16_t adc_get_val() {
	return ADC;
}

void adc_interrupt_enable() {
	ADCSRA |= (1 << ADIE);
}

void adc_interrupt_disable() {
	ADCSRA &= ~(1 << ADIE);
}

void adc_channel_toggle() {
	if ((ADMUX & 0b00111111) == 1) {
		ADMUX &= ~(1 << MUX0);
		ADMUX |= (1 << MUX1);
	} else if ((ADMUX & 0b00111111) == 2) {
		ADMUX &= ~(1 << MUX1);
		ADMUX |= (1 << MUX0);
	}
}

void adc_channel_audio(){
	ADMUX &= ~((1 << MUX5) | (1 << MUX4) | (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0));
	ADMUX |= (1 << MUX0);
}

void adc_channel_vbat_level(){
	ADMUX &= ~(1 << MUX1);
	ADMUX |= (1 << MUX1);
}