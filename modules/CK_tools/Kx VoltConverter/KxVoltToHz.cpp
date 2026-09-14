#include ".\KxVoltToHz.h"

REGISTER_PLUGIN( KxVoltToHz, L"KxVoltConverter" );


KxVoltToHz::KxVoltToHz( IMpUnknown* host ) : MpBase( host )
{
	initializePin( 0, pinToGUI );
	//initializePin( 1, pinFromGUI );
    initializePin( 1, pinVolt );
    initializePin( 2, pinFloat );
    initializePin( 3, pinInt );
    initializePin( 4, pinFloatOut );
    initializePin( 5, pinMode );
    //m_volt=0.0f;
    //m_float=0.0f;
    //m_int=0;
}
void KxVoltToHz::onSetPins(void)
{
         //if(init==0 && pinVolt!=0.0f)pinToGUI=pinFloatOut=pinVolt*10.0f;
         //if(init==0 && pinFloat!=0.0f)pinToGUI=pinFloatOut=pinFloat;
         //if(init==0 && pinInt!=0)pinToGUI=pinFloatOut=float(pinInt);
         //init=1;
      if(pinMode==0)
      {
        //if(pinVolt.isStreaming()==false || pinVolt.isStreaming()==true || m_volt !=pinVolt*10.0f)// || (pinToGUI !=pinVolt*10.0f && pinVolt != 0.0f))
        //if(pinVolt.isStreaming()==false || pinVolt.isStreaming()==true || m_volt !=pinVolt*10.0f || (pinToGUI !=pinVolt*10.0f && pinVolt != 0.0f))
        if(pinVolt.isStreaming()==false || pinVolt.isStreaming()==true || pinToGUI !=pinVolt*10.0f || pinToGUI==0.0f)
        {
        //m_volt=pinToGUI=pinFloatOut=pinVolt*10.0f;
        pinToGUI=pinFloatOut=pinVolt*10.0f;
        pinToGUI.sendPinUpdate();
        pinFloatOut.sendPinUpdate();
        }

      }
      if(pinMode==1)
      {
        //if( pinFloat.isUpdated() || m_float !=pinFloat|| (pinToGUI !=pinFloat && pinFloat != 0.0f))
        if( pinFloat.isUpdated() || pinToGUI !=pinFloat || pinToGUI==0.0f )
        {
        pinToGUI=pinFloatOut=pinFloat;
        //m_float=pinFloat;
        pinToGUI.sendPinUpdate();
        pinFloatOut.sendPinUpdate();
        }
      }

      if(pinMode==2)
      {
        //if( pinInt.isUpdated() || m_int !=pinInt || (pinToGUI !=pinInt && pinInt != 0))
        if( pinInt.isUpdated() || pinToGUI !=float(pinInt) || pinToGUI==0.0f)
        {
        pinToGUI=pinFloatOut=float(pinInt);
        //m_int=pinInt;
        pinToGUI.sendPinUpdate();
        pinFloatOut.sendPinUpdate();
        }
      }



}


