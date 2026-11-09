#include <stdio.h>
#include "diag/trace.h"
#include <string.h>

#include "cmsis/cmsis_device.h"


// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

#define BAR_GRAPH_OFFSET  ( 512 + 32 )

//
// Clock prescaler for TIM2 timer: no prescaling
//
#define myTIM2_PRESCALER ((uint16_t)0x0000)

//
// Maximum possible setting for overflow
//
#define myTIM2_PERIOD ((uint32_t)0xFFFFFFFF)

//
// Global variable keeping track of elapsed timer pulses
//
volatile uint32_t
    timerPulses = 0;

//
// Global variable indicating if TIM2 has been started
//
volatile uint32_t
    timerTriggered = 0;


void SelectExternalSource( void );
uint16_t GetCurrentSignalSource( void );

void IRQ_Config( void );
void TIM2_Config();

unsigned int
    Frequency = 0;




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
// Configure the PLL for a 48MHz system clock
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


//
// At this stage the system clock should have already been configured
// at high speed.
//
    trace_printf("System clock: %uHz\n", SystemCoreClock);
    trace_printf("Standard Peripheral Library version %d.%d.%d\n",
        __STM32F0_DEVICE_VERSION_MAIN,
        __STM32F0_DEVICE_VERSION_SUB1,
        __STM32F0_DEVICE_VERSION_SUB2 );

//
// Initialize external interrupt
//

    IRQ_Config();
    TIM2_Config();      // Initialize TIM2

    while (1)
    {
    }
}


void IRQ_Config(void)
{
//
// Enable clocks for GPIOA, GPIOB and SYSCFG
//
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;

    GPIOA->MODER &= 0xFFFFFFFC;     // Set PA0 as an input with no pull-up/down
    GPIOA->PUPDR &= 0xFFFFFFFC;

    GPIOB->MODER &= 0xFFFFFFCF;     // Set PB2 as an input with no pull-up/down
    GPIOB->PUPDR &= 0xFFFFFFCF;

    SelectExternalSource( );
}

void SelectExternalSource()
{

//
// Disable both frequency measurement inputs
//
    EXTI->IMR &= ~((uint32_t)0x0000000C);
    EXTI->RTSR &= ~((uint32_t)0x0000000C);

//
// Set priority of IRQ6 to max
//
    NVIC->IP[1] |= ((uint32_t)0x00FF0000 );


//
// Enable IRQ Channel for EXTI2(IRQ6)
//
    NVIC->ISER[0] = (uint32_t)0X00000060;

//
// Specify IRQ Priority for EXTI1 = IRQ6
//
    NVIC->IP[1] = ((uint32_t)0xFF00FFFF);


//
// Configure EXTI2 line for function generator signal
//
    EXTI->IMR |= ((uint32_t)0x00000004);    // Interrupt enabled
    EXTI->RTSR |= ((uint32_t)0x00000004);   // Rising-edge trigger

//
// Select PB2 as the interrupt source
//
    SYSCFG->EXTICR[0] = 0x00000100;
}

void TIM2_Config()
{
//
// Enable clock for TIM2 peripheral
//
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

//
// Configure TIM2: buffer auto-reload, count up, stop on overflow,
// enable update events, interrupt on overflow only
//
    TIM2->CR1 = ((uint16_t)0x008C);

//
// Set clock prescaler value
//
    TIM2->PSC = myTIM2_PRESCALER;

//
// Set auto-reloaded delay
//
    TIM2->ARR = myTIM2_PERIOD;

//
// Update timer registers
//
    TIM2->EGR = ((uint16_t)0x0001);

//
// Assign TIM2 interrupt priority = 0 in NVIC
//
    NVIC_SetPriority(TIM2_IRQn, 0);

//
// Same as: NVIC->IP[3] = ((uint32_t)0x00FFFFFF);
//
// Enable TIM2 interrupts in NVIC
//
    NVIC_EnableIRQ(TIM2_IRQn);

//
// Same as: NVIC->ISER[0] = ((uint32_t)0x00000800)
//

//
// Enable update interrupt generation
//
    TIM2->DIER |= TIM_DIER_UIE;
}


//
//  Interrupt handler for function generator or 555 timer
//

void EXTI2_3_IRQHandler()
{
    double
        sigFrequency = 0;

    unsigned int
        int_mask;

//
// Check if EXTI2 interrupt pending flag is indeed set
//
    if ((EXTI->PR &  EXTI_PR_PR2 ) != 0)
    {

//
// Stop timer
//
        TIM2->CR1 &= ~(TIM_CR1_CEN);

//
// First edge reset the count and start the timer
//
        if (timerTriggered == 0)
        {
            timerPulses = 0;
            timerTriggered = 1;

            TIM2->CNT = 0x00000000;             // Clear count register
            TIM2->CR1 |= TIM_CR1_CEN;           // Start timer
        }

//
// Second edge stop the counter and compute the period and frequency
//
        else
        {
            timerPulses = TIM2->CNT;
            int_mask = EXTI->IMR &  EXTI_IMR_MR2;
            EXTI->IMR &= ~EXTI_IMR_MR2;              // Mask EXTI1 interrupts


            timerTriggered = 0;
            sigFrequency = ((double)SystemCoreClock)/ ((double)timerPulses);
            Frequency = (unsigned int)sigFrequency;


            trace_printf( "%10d %8.1f\r", timerPulses, sigFrequency );

//
// Unmask EXTI2 or EXTI3 interrupts
//
            EXTI->IMR |= ( EXTI->IMR & ~EXTI_IMR_MR2 ) | int_mask;
        }

//
// Clear pending interrupt flag
//
        EXTI->PR = EXTI_PR_PR2;
    }
}

//
// This handler is declared in system/src/cmsis/vectors_stm32f0xx.c
//
void TIM2_IRQHandler()
{
//
// Check if update interrupt flag is indeed set
//
    if ((TIM2->SR & TIM_SR_UIF) != 0)
    {
        trace_printf("\n*** Overflow! ***\n");

        TIM2->SR &= ~(TIM_SR_UIF);              // Clear update interrupt flag
        TIM2->CR1 |= TIM_CR1_CEN;               // Restart stopped timer
    }
}

#pragma GCC diagnostic pop
