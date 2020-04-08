#include "atmel_start.h"
#include "stdio_start.h"
#include "winc_init.h"
#include <string.h>
#include "main.h"
#include "common/include/nm_common.h"
#include "driver/include/m2m_wifi.h"
#include "socket/include/socket.h"

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

static TaskHandle_t xCreatedWiFiTask;

#define TASK_WIFI_STACK_SIZE (1024 / sizeof(portSTACK_TYPE))
#define TASK_WIFI_STACK_PRIORITY (tskIDLE_PRIORITY + 1)


static void task_winc1500(void *p)
{
	int cnt = 0;
	(void)p;
	for (;;)
	{
		printf("wifi %d\r\n",cnt++);
		os_sleep(500);
	}
}


/**
 * \brief Create OS task for WiFi Operations
 */
void task_winc1500_create(void)
{
	/* Create task to make led blink */
	if (xTaskCreate(task_winc1500, "WiFi", TASK_WIFI_STACK_SIZE, NULL, TASK_WIFI_STACK_PRIORITY, &xCreatedWiFiTask) != pdPASS) {
		while (1) {
			;
		}
	}
}

