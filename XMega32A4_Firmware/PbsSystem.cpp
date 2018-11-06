/*
 * MS6System.cpp
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */

#include "DebugOptions.h"
#include <Gateway.h>
#include "HomeAutomation.h"
#include <PersistentRules.h>
#include <RuleEngine.h>
#include <Scheduler.h>
#include <Time/WeekTime.h>
#include <Release.h>
#include "PbsSystem.h"

PbsSystem::PbsSystem() :
   digitalPortA( PortA ), digitalPortB( PortB ), digitalPortC( PortC ),
   digitalPortD( PortD )
{
   digitalPortB.setNotUseablePins( Pin4 | Pin5 | Pin6 | Pin7 );
#if ( CONTROLLER_ID == 4 )
   digitalPortA.setNotUseablePins( Pin6 | Pin7 );
   digitalPortD.setNotUseablePins( Pin6 | Pin7 );
#endif
}

void PbsSystem::start()
{
   static const uint8_t MAX_JOBS = 40;
   Scheduler::setup( MAX_JOBS );

   SystemTime::init( SystemTime::RTCSRC_RCOSC_1024, 1024 );

   static PbsSystem ms6;
   Scheduler::runJobs();
}
