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
#include "cmsis/cmsis_device.h" //Library to manipulate the MCU


// See "system/include/cmsis/stm32f051x8.h" for register/bit definitions.
// See "system/src/cmsis/vectors_stm32f051x8.c" for handler declarations.



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

void mySPI_config(void);//PB13 and 15 AF enable function
void myGPIOB_Init(void);//GPIOB init
SPI_HandleTypeDef SPI_Handle;


void SystemClock48MHz( void )
{
//
// Disable the PLL
//
    RCC->CR &= ~(RCC_CR_PLLON);
//
// Wait for the PLL to unlock
//
    while (( RCC->CR & RCC_CR_PLLRDY ) != 0 );
//
// Configure the PLL for 48-MHz system clock
//
    RCC->CFGR = 0x00280000;
//
// Enable the PLL
//
    RCC->CR |= RCC_CR_PLLON;
//
// Wait for the PLL to lock
//
    while (( RCC->CR & RCC_CR_PLLRDY ) != RCC_CR_PLLRDY );
//
// Switch the processor to the PLL clock source
//
    RCC->CFGR = ( RCC->CFGR & (~RCC_CFGR_SW_Msk)) | RCC_CFGR_SW_PLL;
//
// Update the system with the new clock frequency
//
    SystemCoreClockUpdate();

}


int
main(int argc, char* argv[])
{
	SystemClock48MHz();
  // At this stage the system clock should have already been configured
  // at high speed.
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;  /* Enable SYSCFG clock */
	myGPIOB_Init();
	mySPI_config();




  // Infinite loop
  while (1)
    {


    }
}

/*Author:Benjamin Amadi
 Purpose:GPIOB Initialization
 I/O: GPIOB(AF)
 Registers manipulated:RCC,GPIOB
 */
void myGPIOB_Init(){
	/*Enable clock for GPIOB pins*/
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
}

/*Author:Benjamin Amadi
 Purpose:SPI2 output
 I/O: SPI2->
 Registers manipulated:RCC,GPIOB
 */

void mySPI_config(){
	//trace_printf("hi"); // Add your code here.
	/*Enable clock for SPI2 peripheral*/
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

	/*Configure PB13 and 15 as Alt Func*/
	GPIOB->MODER = 0x88000000;
	/* Ensure push-pull mode selected for PC8 and PC9 */
		GPIOC->OTYPER &= ~(GPIO_OTYPER_OT_13 | GPIO_OTYPER_OT_15);
		/* Ensure high-speed mode for PC8 and PC9 */
		GPIOC->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR13 | GPIO_OSPEEDER_OSPEEDR15);
		/* Ensure no pull-up/pull-down for PC8 and PC9 */
		GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPDR13 | GPIO_PUPDR_PUPDR15);

	/*Configure SPI2 CR as shown in lecture*/
    SPI_Handle.Instance = SPI2;

    SPI_Handle.Init.Direction = SPI_DIRECTION_1LINE;
    SPI_Handle.Init.Mode = SPI_MODE_MASTER;
    SPI_Handle.Init.DataSize = SPI_DATASIZE_8BIT;
    SPI_Handle.Init.CLKPolarity = SPI_POLARITY_LOW;
    SPI_Handle.Init.CLKPhase = SPI_PHASE_1EDGE;
    SPI_Handle.Init.NSS = SPI_NSS_SOFT;
    SPI_Handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    SPI_Handle.Init.FirstBit = SPI_FIRSTBIT_MSB;
    SPI_Handle.Init.CRCPolynomial = 7;

//
// Initialize the SPI interface
//
    HAL_SPI_Init( &SPI_Handle );

//
// Enable the SPI
//
    __HAL_SPI_ENABLE( &SPI_Handle );
  //  trace_printf("hi"); // Add your code here.

}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
