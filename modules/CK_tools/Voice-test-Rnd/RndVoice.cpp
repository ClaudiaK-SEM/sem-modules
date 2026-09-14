//#include "windows.h"
#include "RndVoice.h"
//#include "stdlib.h"

REGISTER_PLUGIN ( RndVoice, L"KX77FREE VOICE ID" );

RndVoice::RndVoice( IMpUnknown* host ) : MpBase( host )
{
	// Register pins.

    initializePin( 0, pinVoiceReset );
	initializePin( 1, pinGateIn );
	initializePin( 2, pinOutput );
	initializePin( 3, pinRnd );
    initializePin( 4, pinMul );
    initializePin( 5, pinMulRnd );
    initializePin( 6, pinOn );
    initializePin( 7, pinOffRnd );
}
/*int32_t RndVoice::open()
{
	MpBase::open();	// always call the base class
    init=true;
	return gmpi::MP_OK;
}*/
void RndVoice::sub_process(int bufferOffset, int sampleFrames )
{

	float* mul	= bufferOffset + pinMul.getBuffer();
	float* out	= bufferOffset + pinOutput.getBuffer();

    for( int s = sampleFrames; s > 0; --s )
    {
    //*out++=pinVoiceReset*0.1f;//0.1f;

    *out=*mul*rnd;//0.1f;
    mul++;
    out++;
    }

}
/*void RndVoice::sub_process_0(int bufferOffset, int sampleFrames )
{

	float* out	= bufferOffset + pinOutput.getBuffer();

    for( int s = sampleFrames; s > 0; --s )
    {
    *out++=0.0f;
    }

}*/
void RndVoice::onSetPins(void)
{

    if( pinOn==1 )
    {

        if( pinVoiceReset.isUpdated())
        {
            if( pinVoiceReset > 0.0f )
            {
            float RANGE_MIN = 0.0f;
            float  RANGE_MAX = 1.0f;
            float  r= (((float) rand() / (float) RAND_MAX) * RANGE_MAX + RANGE_MIN);
            if(pinRnd==1)rnd=(r*pinMulRnd)+pinOffRnd;
            else
            rnd=0.1f;

            pinOutput.setStreaming( true );
            SET_PROCESS(&RndVoice::sub_process);
            setSleep( false );
            }
            else
            {
            rnd=0.0f;
            pinOutput.setStreaming( false );
            SET_PROCESS(&RndVoice::sub_process);
            setSleep( true );
            }

        }

    }
    else
    {
    rnd=0.0f;
    pinOutput.setStreaming( false );
    SET_PROCESS(&RndVoice::sub_process);
    setSleep( true );
    }



    /*if( pinVoiceReset.isUpdated() && init==true)
    {
        init=false;
        pinOutput.setStreaming( false );
        SET_PROCESS(&RndVoice::sub_process_0);
        setSleep( true );
    }

    if( pinVoiceReset.isUpdated() && init==false)
    {
        if( pinVoiceReset > 0.0f )
        {
        pinOutput.setStreaming( true );
        SET_PROCESS(&RndVoice::sub_process);
        setSleep( false );
        }
        else
        {
        pinOutput.setStreaming( false );
        //SET_PROCESS(&RndVoice::sub_process);
        SET_PROCESS(&RndVoice::sub_process_0);
        setSleep( true );
        }

    }*/

}



