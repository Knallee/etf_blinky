/*
 * spi_bit_bang.c
 *
 * Created: 10/13/2023 10:30:29 PM
 *  Author: ccederberg
 */ 

#include "bit_bang_spi.h"

void bit_bang_init() {
	DDR_MOSI |= (1 << PIN_MOSI);
	DDR_SCK |= (1 << PIN_SCK);
	
}

void bit_bang_bit(uint8_t bit){
	if (bit) {
		PORT_SCK &= ~(1 << PIN_SCK);
		PORT_MOSI |= (1 << PIN_MOSI);
		asm volatile("nop\n\t" ::);
		asm volatile("nop\n\t" ::);
		asm volatile("nop\n\t" ::);
		PORT_SCK |= (1 << PIN_SCK);
		asm volatile("nop\n\t" ::);
		asm volatile("nop\n\t" ::);
		asm volatile("nop\n\t" ::);
		PORT_MOSI &= ~(1 << PIN_MOSI);
		PORT_SCK |= (1 << PIN_SCK);
	} else {
		PORT_SCK &= ~(1 << PIN_SCK);
		PORT_MOSI &= ~(1 << PIN_MOSI);
		asm volatile("nop\n\t" ::);
		asm volatile("nop\n\t" ::);
		asm volatile("nop\n\t" ::);
		PORT_SCK |= (1 << PIN_SCK);
		asm volatile("nop\n\t" ::);
		asm volatile("nop\n\t" ::);
		asm volatile("nop\n\t" ::);
		PORT_MOSI &= ~(1 << PIN_MOSI);
		PORT_SCK |= (1 << PIN_SCK);
	}
}


// It takes roughly 96 us to send one byte by calling this function.
void bit_bang_byte(uint8_t byte){
	for (int i = 7; i>=0; i--) {
		uint8_t bit = (((byte & (1 << i)) >> i));
		bit_bang_bit(bit);
	}
}