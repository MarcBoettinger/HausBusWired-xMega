#ifndef SwFramework_Stream_H
#define SwFramework_Stream_H

#include "SwFramework.h"
#include "Object.h"
#include "DefaultTypes.h"

class Stream : public Object
{
   public:

      class TransferDescriptor;

      enum State
      {
         DISABLED,
         IDLE,
         WRITING,
         READING,
         HALTED,
         BUSY
      };

      enum Status
      {
         SUCCESS,
         LOCKED,
         ABORTED,
         STOPPED,
         RESET,
         INVALID_BUFFER,
         INVALID_STREAM,
         TIMEOUT,
         NOT_SUPPORTED,
         NO_ACK,
         NO_RECEIVER,
         NO_DATA,
         ARB_LOST
      };

      enum SeekPosition
      {
         START,
         CURRENT,
         END
      };

      class TransferDescriptor
      {
         ////    Constructors and destructors    ////

         public:

            inline TransferDescriptor()
            {
               reset();
            }

            ////    Operations    ////
            void reset();

            ////    Additional operations    ////
            inline uint8_t* getPData() const
            {
               return pData;
            }

            inline void setPData( uint8_t* p_pData )
            {
               pData = p_pData;
            }

            inline uint16_t getBytesRemaining() const
            {
               return bytesRemaining;
            }

            inline void setBytesRemaining( uint16_t p_bytesRemaining )
            {
               bytesRemaining = p_bytesRemaining;
            }

            inline uint16_t getBytesTransferred() const
            {
               return bytesTransferred;
            }

            inline void setBytesTransferred( uint16_t p_bytesTransferred )
            {
               bytesTransferred = p_bytesTransferred;
            }

            ////    Attributes    ////

            // Transfer descriptor pointer to a buffer where the data is read/stored.
            uint8_t* pData;

            // Number of remaining bytes to transfer.
            uint16_t bytesRemaining;

            // Number of bytes transferred for the current operation.
            uint16_t bytesTransferred;
      };

      ////    Constructors and destructors    ////

      inline Stream() :
         state( DISABLED )
      {

      }

      ////    Additional operations    ////

      inline State getState() const
      {
         return state;
      }

   protected:

      inline void setState( State p_state )
      {
         state = p_state;
      }

      ////    Attributes    ////

      State state;
};

#endif

