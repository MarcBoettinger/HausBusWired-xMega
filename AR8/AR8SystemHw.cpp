#include "AR8SystemHw.h"

#include <Security/Checksum.h>
#include <DaliDimmerHw.h>
#include <DaliLine.h>
#include <Dimmer.h>
#include <DS1820.h>
#include <Peripherals/Eeprom.h>
#include <Peripherals/EventSystem.h>
#include <HacfIpStackManager.h>
#include <IrReceiver.h>
#include <Traces/Logger.h>
#include <LogicalButton.h>
#include <Security/ModuleId.h>
#include <PersistentRules.h>
#include <DigitalOutputUnit.h>
#include <RollerShutter.h>
#include <Peripherals/TimerCounter0.h>
#include <Protocols/IpStack/UdpConnection.h>
#include <Protocols/Ethernet/MAC.h>
#include <Gateway.h>
#include <HacfIpStackManager.h>

AR8SystemHw::AR8SystemHw()
{
   configure();
}

void AR8SystemHw::configure()
{
   // configure Logger
   Logger::instance().setStream( putc );
   Usart::instance( PortD, 1 ).init<BAUDRATE,             // baudrate
                                    true, // doubleClock
                                    USART_CMODE_ASYNCHRONOUS_gc, // asynchronous communication
                                    USART_PMODE_DISABLED_gc, // NoParity
                                    USART_CHSIZE_8BIT_gc, // 8-Bits
                                    false // 1 stopBit
                                    >();
   HwConfiguration::storage = &internalEeprom;

   // configure ports
   IoPort::instance( PortC ).setPinsAsOutput( Pin4 | Pin5 | Pin7 );

   IoPort::instance( PortD ).setPinsAsOutput( Pin4 | Pin7 );
   IoPort::instance( PortD ).configure( Pin5, PORT_OPC_PULLUP_gc, false,
                                        PORT_ISC_LEVEL_gc );

   IoPort::instance( PortE ).configure( Pin0 | Pin1 | Pin2 | Pin4,
                                        PORT_OPC_PULLUP_gc );
   IoPort::instance( PortE ).setPinsAsOutput( Pin3 | Pin5 );
   IoPort::instance( PortE ).setPins( Pin3 );

   TRACE_PORT_INIT( AllPins );

   IoPort::instance( PortR ).setPinsAsOutput( Pin0 | Pin1 );
   IoPort::instance( PortR ).set( Pin0 | Pin1 );

   DEBUG_H1( FSTR( "configure" ) );
   configureSlots();
   configureZeroCrossDetection();
   configureDaliLine();
   configureLogicalButtons();
   configureEthernet();
   configureTwi();

   enableInterrupts();
}

void AR8SystemHw::configureDaliLine()
{
   if ( daliHw.isConnected() )
   {
      DEBUG_M1( FSTR( "DaliHw detected" ) );
      daliLine = new DaliLine( daliHw );
   }
}

void AR8SystemHw::configureEthernet()
{
   uint16_t deviceId = HwConfiguration::HomeAutomation::instance().getDeviceId();
   MAC::local = MAC( 0xAE, 0xDE, 0x48, 0, HBYTE( deviceId ), LBYTE( deviceId ) );

   if ( enc28j60.init() == Enc28j60::OK )
   {
      new HacfIpStackManager( enc28j60 );
      new Gateway( UdpConnection::connect( IP( 255, 255, 255, 255 ), 9, 9, NULL ),
                   Gateway::UDP );
   }
}

#include <SoftTwi.h>

void AR8SystemHw::configureTwi()
{
   static SoftTwi twi;
   new Gateway( &twi, Gateway::TWI );
}

void AR8SystemHw::configureLogicalButtons()
{
   DEBUG_M1( FSTR( "LButtons" ) );

   uint8_t i = 0;
   uint8_t mask
      = HwConfiguration::HomeAutomation::instance().getLogicalButtonMask();
   while ( mask )
   {
      if ( mask & 1 )
      {
         new LogicalButton( i + 1 );
      }
      mask >>= 1;
      i++;
   }
}

void AR8SystemHw::configureSlots()
{
   DEBUG_M1( FSTR( "Slots" ) );

   uint8_t slot = MAX_SLOTS;
   uint8_t pinNumber = 0;
   Object* object;
   while ( slot-- )
   {
      if ( slot < ( MAX_SLOTS / 2 ) )
      {
         slotHw[slot].getDigitalOutput1()->setPortNumber( PortA );
         slotHw[slot].getDigitalOutput0()->setPortNumber( PortC );
         slotHw[slot].setTimerCounter0( &TimerCounter0::instance( PortC ) );
      }
      else
      {
         slotHw[slot].getDigitalOutput1()->setPortNumber( PortB );
         slotHw[slot].getDigitalOutput0()->setPortNumber( PortD );
         slotHw[slot].setTimerCounter0( &TimerCounter0::instance( PortD ) );
      }
      if ( pinNumber > 3 )
      {
         pinNumber = 0;
      }
      slotHw[slot].getDigitalOutput0()->setPinNumber( pinNumber );
      slotHw[slot].getDigitalOutput1()->setPinNumber( pinNumber );
      slotHw[slot].configure(
         HwConfiguration::HomeAutomation::instance().getSlotType( slot ) );

      if ( slotHw[slot].isDimmerHw() )
      {
         new Dimmer( slot + 1, &slotHw[slot] );
      }
      else if ( slotHw[slot].isPowerSocketHw() )
      {
         object = new DigitalOutputUnit( *slotHw[slot].getDigitalOutput1() );
         object->setInstanceId( slot + 1 );
      }
      else if ( slotHw[slot].isRollerShutterHw() )
      {
         new RollerShutter( slot + 1,
                            reinterpret_cast<RollerShutterHw*>( &slotHw[slot] ) );
      }
      else if ( slotHw[slot].isDoubleSwitchHw() )
      {
         object = new DigitalOutputUnit( *slotHw[slot].getDigitalOutput0() );
         object->setInstanceId( slot + 1 );
         object = new DigitalOutputUnit( *slotHw[slot].getDigitalOutput1() );
         object->setInstanceId( slot + 9 );
      }
      DEBUG_M4( "slot", slot, " = ", slotHw[slot].getType() );

      pinNumber++;
   }
}

void AR8SystemHw::configureZeroCrossDetection()
{
   TimerCounter1& phaseShifter = TimerCounter1::instance( PortD );
   phaseShifter.configEventAction( TC_EVSEL_CH1_gc, TC_EVACT_RESTART_gc );
   phaseShifter.configWGM( TC_WGMODE_NORMAL_gc );
   phaseShifter.setPeriod( 0xFFFF );
   // Select TCD1_CCA as event channel 0 multiplexer input.
   EventSystem::setEventSource( 0, EVSYS_CHMUX_TCD1_CCA_gc );

   TimerCounter0& dimmerC = TimerCounter0::instance( PortC );
   dimmerC.configWGM( TC_WGMODE_SS_gc );
   dimmerC.configEventAction( TC_EVSEL_CH0_gc, TC_EVACT_RESTART_gc );
   dimmerC.setPeriod( ZCD_DEFAULT_PERIOD );

   TimerCounter0& dimmerD = TimerCounter0::instance( PortD );
   dimmerD.configWGM( TC_WGMODE_SS_gc );
   dimmerD.configEventAction( TC_EVSEL_CH0_gc, TC_EVACT_RESTART_gc );
   dimmerD.setPeriod( ZCD_DEFAULT_PERIOD );

   // Select PA7 as event channel 1 multiplexer input.
   IoPort& portA = IoPort::instance( PortA );
   portA.configure( Pin7, PORT_OPC_PULLUP_gc, false, PORT_ISC_RISING_gc );
   portA.enableInterrupt0Source( Pin7 );
   EventSystem::setEventSource( 1, EVSYS_CHMUX_PORTA_PIN7_gc );

   if ( getFckE() < FCKE_V3_0 )
   {
      portA.configure( Pin5, PORT_OPC_PULLUP_gc, false, PORT_ISC_RISING_gc );
      // if slot0-4 has dimmer
      if ( slotHw[0].isDimmerHw() || slotHw[1].isDimmerHw()
         || slotHw[2].isDimmerHw() || slotHw[3].isDimmerHw() )
      {
         // Select PA5 as event channel 0 multiplexer input.
         EventSystem::setEventSource( 1, EVSYS_CHMUX_PORTA_PIN5_gc );
         portA.disableInterrupt0Source( Pin7 );
         portA.enableInterrupt0Source( Pin5 );
      }
      phaseShifter.setCompareA( 0x28 );
   }
   else
   {
      phaseShifter.setCompareA( 0x50 );
   }
   // start counters
   phaseShifter.forceUpdate();
#if F_CPU == 32000000UL
   phaseShifter.configClockSource( TC_CLKSEL_DIV256_gc );
#elif F_CPU == 8000000UL
   phaseShifter.configClockSource( TC_CLKSEL_DIV64_gc );
#else
#    error "F_CPU is not supported for phaseShifter"
#endif

   phaseShifter.setOverflowIntLevel( TC_OVFINTLVL_LO_gc );

   portA.enableInterrupt0( PORT_INT0LVL_MED_gc );
}

InternalEeprom* AR8SystemHw::getInternalEeprom() const
{
   return (InternalEeprom*) &internalEeprom;
}

