/*
 * attiny44_test.c
 *
 * Created: 8/26/2023 10:06:19 PM
 * Author : ccederberg
 */ 


#include "f_cpu.h"
#include "bit_bang_spi.h"
#include "adc.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile uint16_t adc_val_8bit, delay_count;

volatile uint8_t red_led, green_led, blue_led, led_pos;
volatile int8_t k;

void led_send();

int main(void)
{
	k = 0;
	led_pos = 0;
	
	DDRA |= (1 << PORTA6);
	
	adc_init();
	adc_interrupt_enable();
	sei();
	adc_start();
	
	bit_bang_init();
	
	
	
    while (1) {
		//PORTA |= (1 << PORTA6);
		led_send();
		//PORTA &= ~(1 << PORTA6);
		//_delay_ms(1);
	}

}



ISR(ADC_vect) {
	adc_val_8bit = adc_get_val() >> 2;
	red_led = adc_val_8bit;
	green_led = 0.5 * adc_val_8bit;
	blue_led = 0.1 * adc_val_8bit;
	
	delay_count = adc_val_8bit;
	
	
	
	adc_start();
}

void led_send(){
	
	bit_bang_byte(0);
	bit_bang_byte(0);
	bit_bang_byte(0);
	bit_bang_byte(0);
	

	for (int i = 0; i < 10; i++) {
		
		if (led_pos == i) {
			bit_bang_byte(0xFF);
			bit_bang_byte(255);
			bit_bang_byte(0);
			bit_bang_byte(0);
		} else {
			bit_bang_byte(0xFF);
			bit_bang_byte(0);
			bit_bang_byte(10);
			bit_bang_byte(0);
		}
				
	}
	
	led_pos = led_pos + k;
	
	if (led_pos == 9) k = -1;
	if (led_pos == 0)  k =  1;
	
	
	for (int j = 0; j < delay_count; j++) _delay_ms(.1);
	
	bit_bang_byte(0xFF);
	bit_bang_byte(0xFF);
	bit_bang_byte(0xFF);
	bit_bang_byte(0xFF);
	
}