/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "driver_examples.h"
#include "driver_init.h"
#include "utils.h"

/**
 * Example of using EDBG_UART to write "Hello World" using the IO abstraction.
 */
void EDBG_UART_example(void)
{
	struct io_descriptor *io;
	usart_sync_get_io_descriptor(&EDBG_UART, &io);
	usart_sync_enable(&EDBG_UART);

	io_write(io, (uint8_t *)"Hello World!", 12);
}

void delay_example(void)
{
	delay_ms(5000);
}
