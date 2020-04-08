/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "driver_init.h"
#include <peripheral_clk_config.h>
#include <utils.h>
#include <hal_init.h>
#include <hpl_gclk_base.h>
#include <hpl_pm_base.h>

struct spi_m_sync_descriptor WINC_SPI;

struct usart_sync_descriptor EDBG_UART;

void WINC_IRQ_init(void)
{
	_gclk_enable_channel(EIC_GCLK_ID, CONF_GCLK_EIC_SRC);

	// Set pin direction to input
	gpio_set_pin_direction(PB04, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(PB04,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(PB04, PINMUX_PB04A_EIC_EXTINT4);

	ext_irq_init();
}

void WINC_SPI_PORT_init(void)
{

	// Set pin direction to input
	gpio_set_pin_direction(PA04, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(PA04,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(PA04, PINMUX_PA04D_SERCOM0_PAD0);

	gpio_set_pin_level(PA06,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(PA06, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(PA06, PINMUX_PA06D_SERCOM0_PAD2);

	gpio_set_pin_level(PA07,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(PA07, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(PA07, PINMUX_PA07D_SERCOM0_PAD3);
}

void WINC_SPI_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM0);
	_gclk_enable_channel(SERCOM0_GCLK_ID_CORE, CONF_GCLK_SERCOM0_CORE_SRC);
}

void WINC_SPI_init(void)
{
	WINC_SPI_CLOCK_init();
	spi_m_sync_init(&WINC_SPI, SERCOM0);
	WINC_SPI_PORT_init();
}

void EDBG_UART_PORT_init(void)
{

	gpio_set_pin_function(PA22, PINMUX_PA22C_SERCOM3_PAD0);

	gpio_set_pin_function(PA23, PINMUX_PA23C_SERCOM3_PAD1);
}

void EDBG_UART_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM3);
	_gclk_enable_channel(SERCOM3_GCLK_ID_CORE, CONF_GCLK_SERCOM3_CORE_SRC);
}

void EDBG_UART_init(void)
{
	EDBG_UART_CLOCK_init();
	usart_sync_init(&EDBG_UART, SERCOM3, (void *)NULL);
	EDBG_UART_PORT_init();
}

void delay_driver_init(void)
{
	delay_init(SysTick);
}

void system_init(void)
{
	init_mcu();

	// GPIO on PA05

	gpio_set_pin_level(CS_PIN,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   true);

	// Set pin direction to output
	gpio_set_pin_direction(CS_PIN, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(CS_PIN, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB05

	gpio_set_pin_level(CE_PIN,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(CE_PIN, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(CE_PIN, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB06

	gpio_set_pin_level(RESET_PIN,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(RESET_PIN, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(RESET_PIN, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB30

	gpio_set_pin_level(LED0,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(LED0, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(LED0, GPIO_PIN_FUNCTION_OFF);

	WINC_IRQ_init();

	WINC_SPI_init();

	EDBG_UART_init();

	delay_driver_init();
}
