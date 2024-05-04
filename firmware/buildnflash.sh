#!/bin/sh

set -ex

avr-gcc -mmcu=attiny44a test.c bit_bang_spi.c adc.c -O2 -o test.elf
avr-objcopy -j .text -j .data -O ihex test.elf test.hex
avrdude -c atmelice_isp -p t44a -U flash:w:test.hex
