/*
 * MS6System.h
 *
 *  Created on: 28.08.2014
 *      Author: Viktor Pankraz
 */
#ifndef Systems_HomeAutomationSystems_MS6System_H
#define Systems_HomeAutomationSystems_MS6System_H

#include <DigitalPort.h>
#include <HomeAutomation.h>

#include "PbsSystemHw.h"

extern MOD_ID_SECTION const ModuleId moduleId;

class PbsSystem: public HomeAutomation
{
  ////    Constructors and destructors    ////

public:

  PbsSystem();

  ////    Operations    ////

  static void start();

  ////    Additional operations    ////

  inline DigitalPort* getDigitalPortA() const
  {
    return (DigitalPort*) &digitalPortA;
  }

  inline DigitalPort* getDigitalPortB() const
  {
    return (DigitalPort*) &digitalPortB;
  }

  inline DigitalPort* getDigitalPortC() const
  {
    return (DigitalPort*) &digitalPortC;
  }

  inline DigitalPort* getDigitalPortD() const
  {
    return (DigitalPort*) &digitalPortD;
  }

  inline PbsSystemHw* getHardware() const
  {
    return (PbsSystemHw*) &hardware;
  }

  ////    Relations and components    ////

protected:

  DigitalPort digitalPortA;

  DigitalPort digitalPortB;

  DigitalPort digitalPortC;

  DigitalPort digitalPortD;

  PbsSystemHw hardware;
};

#endif
