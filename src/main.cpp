/*
 * main.cpp
 *
 * Created: 03.12.2016 02:31:44
 *  Author: haus-bus
 */ 

#include <Framework/Framework.h>
#include <Homematic/HMWRS485.h>
#include "Homematic/HMW_LC_Sw2_DR.h"


static const uint8_t debugLevel( DEBUG_LEVEL_LOW );
#define getId() FSTR("Main::")


static usart_serial_options_t usart_options = 
{
	.baudrate = USART_SERIAL_BAUDRATE,
	.charlength = USART_SERIAL_CHAR_LENGTH,
	.paritytype = USART_SERIAL_PARITY,
	.stopbits = USART_SERIAL_STOP_BIT
};

void putc( char c )
{
	udi_cdc_putc( c );
}

int main (void)
{
	// Authorize interrupts
	irq_initialize_vectors();
	cpu_irq_enable();
	
	// Initialize the sleep manager service
	sleepmgr_init();
	
	// Initialize the clock service
	sysclk_init();
	board_init();
	
	// Initialize system timer
	rtc_init();
	
	// Initialize RS485 interface
	usart_serial_init(USART_SERIAL, &usart_options);
	
	// Enable USB Stack Device
	udc_start ();
	
	Logger::setStream( putc );
	
	HMWRS485 hmwrs485(USART_SERIAL, RS485_RXEN_GPIO, RS485_TXEN_GPIO );
	setup( &hmwrs485 );
	Timestamp timestamp;
	
	while (true)
	{
		hmwrs485.loop();
		loop();
		if( timestamp.since() >= SystemTime::S )
		{
			timestamp = Timestamp();
			PORTR.OUTTGL = 1;
		}
	}
	
	return 1;
}
