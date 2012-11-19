// CMSIS headers required for setting up SysTick Timer
#include "LPC17xx.h"

#include <cr_section_macros.h>
#include <NXP/crp.h>

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP ;

#include "leds.h"
#include "uart.h"
#include "logger.h"
#include "s0_input.h"
#include "rtc.h"

volatile uint32_t msTicks; // counter for 1ms SysTicks
volatile uint32_t uptime_secs = 0; // counter for 1ms SysTicks
extern volatile unsigned int eint3_count;
extern volatile uint32_t rtc_alarm_secs, rtc_alarm_minutes, rtc_alamr_hours;



// ****************
//  SysTick_Handler - just increment SysTick counter
void SysTick_Handler(void) {
  msTicks++;
}

// ****************
// systick_delay - creates a delay of the appropriate number of Systicks (happens every 10 ms)
__INLINE static void systick_delay (uint32_t delayTicks) {
  uint32_t currentTicks;

  currentTicks = msTicks;	// read current tick counter
  // Now loop until required number of ticks passes.
  while ((msTicks - currentTicks) < delayTicks);
}

// ****************
int main(void) {
	
	// Setup SysTick Timer to interrupt at 10 msec intervals
	if (SysTick_Config(SystemCoreClock / 100)) {
	    while (1);  // Capture error
	}

    /* Initialize RTC module */
	RTCInit();
	LPC_RTC->CIIR = IMSEC;
	RTCStart();
	NVIC_EnableIRQ(RTC_IRQn);

	led_init();	// Setup GPIO for LED2
	led2_on();		// Turn LED2 on
	//led_on(0);
	//led_on(1);

	systick_delay(100);
	led2_off();
	systick_delay(100);
	led2_on();


	UARTInit(0, 115200);	/* baud rate setting */
	UARTSendCRLF(0);
	UARTSendCRLF(0);
	UARTSendStringln(0, "UART2 online ...");


	//EINT3_enable();
	logger_logStringln("logger online ...");

	logger_logString("Value of RTC GPREG0: ");
	logger_logNumberln(LPC_RTC->GPREG0);
	logger_logString("Value of RTC GPREG1: ");
	logger_logNumberln(LPC_RTC->GPREG1);
	logger_logString("Value of RTC GPREG2: ");
	logger_logNumberln(LPC_RTC->GPREG2);
	logger_logString("Value of RTC GPREG3: ");
	logger_logNumberln(LPC_RTC->GPREG3);
	logger_logString("Value of RTC GPREG4: ");
	logger_logNumberln(LPC_RTC->GPREG4);

	LPC_RTC->GPREG0 = 0;
	LPC_RTC->GPREG1 = 0;
	LPC_RTC->GPREG2 = 0;
	LPC_RTC->GPREG3 = 0;
	LPC_RTC->GPREG4 = 0;


	while(1) {

		/* process logger */
		if (logger_dataAvailable() && UARTTXReady(0)) {
			uint8_t data = logger_read();
			UARTSendByte(0,data);
		}

		process_leds(msTicks);

		process_s0(msTicks);

		uint32_t triggerValue = s0_triggered(0);
		if (triggerValue) {
			logger_logStringln("s0_0");
			led_signal(0, 30, msTicks);
		}

		triggerValue = s0_triggered(1);
		if (triggerValue) {
			logger_logStringln("s0_1");
			led_signal(1, 30, msTicks);
		}

		if (rtc_alarm_secs != 0) {
			uptime_secs++;
			rtc_alarm_secs = 0;
			logger_logStringln("RTC Alarm Seconds");
			logger_logString("Uptime: ");
			logger_logNumberln(uptime_secs);
		}
		if (rtc_alarm_minutes != 0) {
			rtc_alarm_minutes = 0;
			logger_logStringln("RTC Alarm Minutes");
		}

	}
	return 0 ;
}
