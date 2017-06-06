/*
** HBWLinkDimmer
**
** Direkte Verknuepfung (Peering), zu Dimmern
** Ein Link-Objekt steht immer fuer alle (direkt aufeinander folgenden) Verknuepfungen
** des entsprechenden Typs.
**
*/

#include "HBWLinkDimmer.h"


HBWLinkDimmer::HBWLinkDimmer(uint8_t _numLinks, uint16_t _eepromStart) 
{
	numLinks = _numLinks;
	eepromStart = _eepromStart;
}
 
// processKeyEvent wird aufgerufen, wenn ein Tastendruck empfangen wurde

void HBWLinkDimmer::receiveKeyEvent(HBWDevice* device, uint32_t senderAddress, uint8_t senderChannel, 
                                          uint8_t targetChannel, boolean longPress) 
{

  Config data;

  // read what to do from EEPROM
  for(byte i = 0; i < numLinks; i++) 
  {
	  device->readEEPROM(&data, eepromStart + sizeof(data) * i, sizeof(data) );

	  // compare sender channel
	  if(data.sensorChannel != senderChannel) continue;

	  // compare target channel
	  if(data.ownChannel != targetChannel) continue;

      if(data.sensorAddress == 0xFFFFFFFF) continue;

      // the endianess in the EEPROM is BigEndian, we need it in LittleEndian
      flipEndianess( &data.sensorAddress );
	  if(data.sensorAddress != senderAddress) continue;

	  // ok, we have found a match
	  // differs for short and long
      uint8_t cmdData[] = { 255, data.shortOffLevel, data.shortOnLevel, data.blinkOnTime, data.blinkOffTime, data.blinkQuantity };
      uint8_t length = sizeof(cmdData);
   	  if( longPress )
      {
          cmdData[1] = data.longOffLevel;
          cmdData[2] = data.longOnLevel;

      	  // we can have
      	  switch( data.longActionType ) 
          {
          	  case 0: // -> ON
              {
                cmdData[0] = data.longOnLevel;
                length = 1;
          	    break;
              }
          	  case 1: // -> OFF
              {
                cmdData[0] = data.longOffLevel;
                length = 1;
          	    break;
              }
              case 3: // -> TOGGLE
              {
                cmdData[0] = HBWChannel::TOGGLE;
                break;
              }
              case 4: // -> BLINK_ON
              {
                cmdData[0] = HBWChannel::BLINK_ON;
                break;
              }
              case 5: // -> BLINK_TOGGLE
              {
                cmdData[0] = HBWChannel::BLINK_TOGGLE;
                break;
              }


              case 2: // -> INACTIVE
          	  default: 
              {
                continue;
              }
           }
      }
      else
      {
      	  switch( data.shortActionType )
      	  {
          	  case 0: // -> ON
          	  {
              	  cmdData[0] = data.shortOnLevel;
                  length = 1;
              	  break;
          	  }
          	  case 1: // -> OFF
          	  {
              	  cmdData[0] = data.shortOffLevel;
                  length = 1;
              	  break;
          	  }
              case 3: // -> TOGGLE
              {
                  cmdData[0] = HBWChannel::TOGGLE;
                  break;
              }
              case 4: // -> BLINK_ON
              {
                  cmdData[0] = HBWChannel::BLINK_ON;
                  break;
              }
              case 5: // -> BLINK_TOGGLE
              {
                  cmdData[0] = HBWChannel::BLINK_TOGGLE;
                  break;
              }
              case 2: // -> INACTIVE
              default:
              {
                  continue;
              }
      	  }
      }
	  device->set(targetChannel,length,cmdData);    // channel, data length, data
  }
}
 
