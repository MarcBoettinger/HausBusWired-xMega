/*
 * PortPin.h
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */
 
#include "PortPin.h"

PortPin::PortPin( uint8_t _portNumber, uint8_t _pinNumber ) :
    pinNumber( _pinNumber ), portNumber( _portNumber )
{
}

void PortPin::configInput()
{
  getIoPort().setPinsAsInput( getPin() );
}

void PortPin::configOutput()
{
  getIoPort().setPinsAsOutput( getPin() );
}

uint8_t PortPin::getPin() const
{
  return (1 << pinNumber);
}

