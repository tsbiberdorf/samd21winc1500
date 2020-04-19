#include <atmel_start.h>

#include <hal_gpio.h>
#include <hal_delay.h>

#include "hal_io.h"
#include "hal_rtos.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

extern void task_winc1500_create(void);

static TaskHandle_t xCreatedLedTask;

#define TASK_LED_STACK_SIZE (1024 / sizeof(portSTACK_TYPE))
#define TASK_LED_STACK_PRIORITY (tskIDLE_PRIORITY + 1)

void SERCOM3_Handler()
{
	while(1);
}

void HardFault_Handler()
{
	volatile int cnt = 1;
	while(cnt)
	{
		cnt++;
	}
}
/**
 * OS task that blinks LED
 */
static void task_led(void *p)
{
	(void)p;

	printf("LED task started\r\n");

	for (;;) 
	{
		gpio_toggle_pin_level(LED0);
		os_sleep(500);
	}
}

/**
 * \brief Create OS task for LED blinking
 */
static void task_led_create(void)
{
	/* Create task to make led blink */
	if (xTaskCreate(task_led, "Led", TASK_LED_STACK_SIZE, NULL, TASK_LED_STACK_PRIORITY, &xCreatedLedTask) != pdPASS) {
		while (1) {
			;
		}
	}
}


void vApplicationIdleHook(void)
{

}

void vApplicationMallocFailedHook(void)
{
	while(1)
	{
		;
	}

}

int main(void)
{
	atmel_start_init();

	task_led_create();
	task_winc1500_create();
	vTaskStartScheduler();

/*
	while (true) {
		delay_ms(500);
		gpio_toggle_pin_level(LED0);
	}
*/
}
