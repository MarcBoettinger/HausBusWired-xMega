/*
 * TimerCounter0.cpp
 *
 *  Created on: 17.07.2017
 *      Author: Viktor Pankraz
 */

#include "TimerCounter0.h"

TimerCounter0& TimerCounter0::instance( uint8_t portNumber )
{
   switch ( portNumber )
   {
    #ifdef TCC0
      case PortC:
         return *reinterpret_cast<TimerCounter0*> ( &TCC0 );
    #endif

    #ifdef TCD0
      case PortD:
         return *reinterpret_cast<TimerCounter0*> ( &TCD0 );
    #endif

    #ifdef TCE0
      case PortE:
         return *reinterpret_cast<TimerCounter0*> ( &TCE0 );
    #endif

    #ifdef TCF0
      case PortF:
         return *reinterpret_cast<TimerCounter0*> ( &TCF0 );
    #endif

   }
   fatalErrorLoop();
   return *(TimerCounter0*)0;
}

