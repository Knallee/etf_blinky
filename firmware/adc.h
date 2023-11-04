/*
 * adc.h
 *
 * Created: 10/15/2023 8:43:12 PM
 *  Author: ccederberg
 */ 


#ifndef ADC_H_
#define ADC_H_


#include "f_cpu.h"
#include <avr/io.h>


#define AUDIO_CHANNEL	(1)
#define VBAT_LEVEL		(2)

void adc_init();
void adc_start();
uint16_t adc_get_val();
void adc_interrupt_enable();
void adc_interrupt_disable();
void adc_channel_toggle();
void adc_channel_audio();
void adc_channel_vbat_level();

#endif /* ADC_H_ */