
#include "HBWKey.h"

// Class HBWKey
HBWKey::HBWKey(uint8_t _pin, Config* _config, HBWChannel* _feedbackChannel) 
{
	keyPressedMillis = 0;
	keyPressNum = 0;
	pin = _pin;
	config = _config;
    feedbackChannel = _feedbackChannel;
	pinMode(pin,INPUT_PULLUP);
}


void HBWKey::loop(HBWDevice* device, uint8_t channel) 
{
    if( config->isUnlocked )
    {
	    uint32_t now = millis();
	    if(digitalRead(pin))
        {
		    // d.h. Taste nicht gedrueckt
		    // "Taste war auch vorher nicht gedrueckt" kann ignoriert werden
		    // Taste war vorher gedrueckt?
		    if(keyPressedMillis)
            {
			    // entprellen, nur senden, wenn laenger als 50ms gedrueckt
			    // aber noch kein "long" gesendet
			    if(now - keyPressedMillis >= 50 && !lastSentLong)
                {
				    keyPressNum++;
			        device->sendKeyEvent(channel,keyPressNum, false);
			    }
			    keyPressedMillis = 0;
                if( feedbackChannel && config->ledFeedbackEnabled )
                {
                    uint8_t cmd = KEY_FEEDBACK_OFF;
                    feedbackChannel->set( device, 1, &cmd );
                }
		    }
	    }
        else
        {
		    // Taste gedrueckt
		    // Taste war vorher schon gedrueckt
		    if(keyPressedMillis)
            {
			    // muessen wir ein "long" senden?
			    if(lastSentLong) 
                {   // schon ein LONG gesendet
				    if(now - lastSentLong >= 300)
                    {  // alle 300ms wiederholen
					    // keyPressNum nicht erhoehen
					    lastSentLong = now ? now : 1; // der Teufel ist ein Eichhoernchen
       			        device->sendKeyEvent(channel,keyPressNum, true);  // long press
				    }
			    }
                else if(now - keyPressedMillis >= long(config->long_press_time) * 100) 
                {
				    // erstes LONG
				    keyPressNum++;
				    lastSentLong = now ? now : 1;
   			        device->sendKeyEvent(channel,keyPressNum, true);  // long press
			    }
		    }
            else
            {
			    // Taste war vorher nicht gedrueckt
			    keyPressedMillis = now ? now : 1; // der Teufel ist ein Eichhoernchen
			    lastSentLong = 0;
                if( feedbackChannel && config->ledFeedbackEnabled )
                {
                    uint8_t cmd = KEY_FEEDBACK_ON;
                    feedbackChannel->set( device, 1, &cmd );
                }
		    }
	    }
    }
}