/*
 * spi_bit_bang.h
 *
 * Created: 10/13/2023 10:31:10 PM
 *  Author: ccederberg
 */ 


#ifndef SPI_BIT_BANG_H_
#define SPI_BIT_BANG_H_

#include "f_cpu.h"
#include <avr/io.h>
#include <util/delay.h>

#define LCD_SPI_CLK_MIN_PERIOD_US	1

#define PORT_MOSI	PORTA
#define PIN_MOSI	(5)
#define DDR_MOSI 	DDRA
#define DDR_SCK		DDRA
#define PORT_SCK	PORTA
#define PIN_SCK		(4)

void bit_bang_init();
void bit_bang_bit(uint8_t bit);
void bit_bang_byte(uint8_t byte);

#endif /* SPI_BIT_BANG_H_ */