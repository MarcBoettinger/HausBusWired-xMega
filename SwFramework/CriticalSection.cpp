/*
 * CriticalSection.cpp
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#include "CriticalSection.h"

#ifndef __AVR

#include <Windows.h>

static CRITICAL_SECTION cs;

CriticalSection::CriticalSection()
{
   if ( cs.LockCount == 0 )
   {
      InitializeCriticalSection( &cs );
   }
   EnterCriticalSection( &cs );
}

CriticalSection::~CriticalSection()
{
   LeaveCriticalSection( &cs );
}

#endif
