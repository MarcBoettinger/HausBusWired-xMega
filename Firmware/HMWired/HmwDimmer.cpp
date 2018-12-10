/*
 * HBWDimmer.cpp
 *
 * Created: 26.04.2017 09:01:56
 * Author: viktor.pankraz
 */


#include "HmwDimmer.h"
#include "HmwDevice.h"

#define getId() FSTR( "HmwDimmer." ) << channelId

const uint8_t HmwDimmer::debugLevel( DEBUG_LEVEL_HIGH | DEBUG_STATE_L3 );

HmwDimmer::HmwDimmer( PortPin _portPin, PortPin _enablePin, Config* _config ) :
   pwmOutput( _portPin.getPortNumber(), _portPin.getPinNumber() ),
   enableOutput( _enablePin ),
   config( _config ),
   actionParameter( NULL ),
   nextFeedbackTime( 0 ),
   nextActionTime( 0 )
{
   type = HmwChannel::HMW_DIMMER;
   SET_STATE_L1( OFF );
   pwmOutput.setInverted( config->getDimmingMode() == Config::DIMM_L );
}


void HmwDimmer::set( uint8_t length, uint8_t const* const data )
{
   if ( ( length == 1 ) && ( *data <= MAX_LEVEL ) )
   {
      setLevel( *data );
      nextActionTime.reset();
   }
   else
   {
      if ( length == sizeof( ActionParameter ) )
      {
         actionParameter = (ActionParameter const*)data;
         switch ( actionParameter->actionType )
         {
            case JUMP_TO_TARGET:
            {
               handleJumpToTargetCmd();
               break;
            }
            default:
            {
               WARN_3( FSTR( "HmwDimmer::set actionType: 0x" ), (uint8_t)actionParameter->actionType, FSTR( " not implemented" ) );
            }
         }
      }
      else if ( getLevel() )
      {
         setLevel( 0 );
      }
      else
      {
         setLevel( MAX_LEVEL );
      }
   }

   // Logging
   if ( !nextFeedbackTime.isValid() && config->isLogging() )
   {
      nextFeedbackTime = Timestamp();
      nextFeedbackTime += ( HmwDevice::getLoggingTime() * 100 );
   }
}


uint8_t HmwDimmer::get( uint8_t* data )
{
   // map 0-100% to 0-200
   ( *data ) = getLevel();
   return 1;
}

void HmwDimmer::loop( uint8_t channel )
{
   if ( nextActionTime.isValid() && nextActionTime.since() )
   {
      handleStateChart();
   }

   // feedback trigger set?
   if ( !nextFeedbackTime.isValid() )
   {
      return;
   }

   if ( !nextFeedbackTime.since() )
   {
      return;
   }

   uint8_t level;
   uint8_t errcode = HmwDevice::sendInfoMessage( channel, get( &level ), &level );

   // sendInfoMessage returns 0 on success, 1 if bus busy, 2 if failed
   if ( errcode )
   {
      // bus busy
      // try again later, but insert a small delay
      nextFeedbackTime += 250;
   }
   else
   {
      nextFeedbackTime.reset();
   }
}

void HmwDimmer::checkConfig()
{
   pwmOutput.setInverted( config->getDimmingMode() == Config::DIMM_L );
}

void HmwDimmer::setLevel( uint8_t level )
{
   // special function for Config::levelFactor == 0, no PWM
   if ( getLevel() != level )
   {
      level ? enableOutput.set() : enableOutput.clear();
      pwmOutput.set( level * NORMALIZE_LEVEL );
   }
}

uint8_t HmwDimmer::getLevel() const
{
   // normalize to 0-200
   return pwmOutput.isSet() / NORMALIZE_LEVEL;
}

void HmwDimmer::handleStateChart( bool fromMainLoop )
{
   if ( actionParameter == NULL )
   {
      return;
   }
   switch ( state )
   {
      case TIME_OFF:
      case OFF:
      {
         if ( isValidActionTime( actionParameter->onDelayTime ) )
         {
            SET_STATE_L1( DELAY_ON );
            nextActionTime = Timestamp();
            nextActionTime += actionParameter->onDelayTime * SystemTime::S / 10;
         }
         else
         {
            SET_STATE_L1( RAMP_UP );
            nextActionTime = Timestamp();
            nextActionTime += actionParameter->rampOnTime * SystemTime::S / 10;
         }
         break;
      }
      case DELAY_ON:
      {
         if ( fromMainLoop )
         {
            SET_STATE_L1( RAMP_UP );
            nextActionTime = Timestamp();
            nextActionTime += actionParameter->rampOnTime * SystemTime::S / 10;
            break;
         }
      }
      case RAMP_UP:
      {
         if ( isValidActionTime( actionParameter->onTime ) )
         {
            SET_STATE_L1( TIME_ON );
            nextActionTime = Timestamp();
            nextActionTime += actionParameter->onTime * SystemTime::S / 10;
         }
         else
         {
            SET_STATE_L1( ON );
            nextActionTime.reset();
         }
         setLevel( actionParameter->onLevel );
         break;
      }
      case TIME_ON:
      case ON:
      {
         if ( actionParameter->offDelayTime < MAX_NEXT_ACTION_TIME )
         {
            SET_STATE_L1( DELAY_OFF );
            nextActionTime = Timestamp();
            nextActionTime += actionParameter->offDelayTime * SystemTime::S / 10;
         }
         else
         {
            SET_STATE_L1( RAMP_DOWN );
            nextActionTime = Timestamp();
            nextActionTime += actionParameter->rampOffTime * SystemTime::S / 10;
         }
         break;
      }
      case DELAY_OFF:
      {
         if ( fromMainLoop )
         {
            SET_STATE_L1( RAMP_DOWN );
            nextActionTime = Timestamp();
            nextActionTime += actionParameter->rampOffTime * SystemTime::S / 10;
            break;
         }
      }
      case RAMP_DOWN:
      {
         if ( isValidActionTime( actionParameter->offTime ) )
         {
            SET_STATE_L1( TIME_OFF );
            nextActionTime = Timestamp();
            nextActionTime += actionParameter->offTime * SystemTime::S / 10;
         }
         else
         {
            SET_STATE_L1( OFF );
            nextActionTime.reset();
         }
         setLevel( actionParameter->offLevel );
         break;
      }
      default:
      {
         WARN_3( FSTR( "HmwDimmer::handleJumpToTargetCmd from state: 0x" ), (uint8_t)state, FSTR( " not implemented" ) );
      }
   }
}

void HmwDimmer::handleJumpToTargetCmd()
{
   handleStateChart( false );
}