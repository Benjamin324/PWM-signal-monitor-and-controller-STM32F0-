/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "diag/trace.h"
#include "cmsis/cmsis_device.h"

// ----------------------------------------------------------------------------
//
// STM32F0 empty sample (trace via DEBUG).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the DEBUG output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace-impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

int pot_res = 0; // determine the maximum potentiometer resistance value

void ADC_Init() {
	//enable clock for GPIOA peripheral
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	//configure PA1 as analog input
	GPIOA->MODER |= 0xC;

	RCC->APB2ENR |= RCC_APB2ENR_ADCEN;

	ADC1->CHSELR = ADC_CHSELR_CHSEL1;

	ADC1->CR = ADC_CR_ADEN;
	//wait for ADRDY = 1 before proceeding with the conversion
	while( !(ADC1->ISR &= ADC_ISR_ADRDY) );
}

uint16_t ADC_Read() {
	//start polling
	ADC1->CR |= ADC_CR_ADSTART;
	//continue polling until end of conversion flag comes up
	while( !(ADC1->ISR & ADC_ISR_EOC) );

	return (uint16_t)ADC1->DR;
}

void DAC_Write(uint16_t ADC_value) {
	
}

int
main(int argc, char* argv[])
{
  // At this stage the system clock should have already been configured
  // at high speed.

	ADC_Init();

  while (1)
    {
      //read adc potentiometer voltage through polling
	  uint16_t ADC_value = ADC_Read();

	  //calculate digital value of pot voltage from ADC read
	  float voltage = (float)ADC_value / 4095f * 3.3;

	  //calculate digital value of pot resistance from ADC read
	  float resistance = (float)ADC_value / 4095f * pot_res;
	  fprintf("ADC Voltage: %f\nADC Resistance: %f\n", voltage, resistance); // for testing

	  //write DAC value to PA4 output
	  DAC_Write(ADC_value);
    }
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
