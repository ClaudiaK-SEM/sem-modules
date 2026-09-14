#include "Scope3.h"
#include "../shared/xplatform.h"

REGISTER_PLUGIN( Scope3, L"KX SCOPE V4" );

/* TODO !!!
	properties->flags = UGF_VOICE_MON_IGNORE;
	properties->gui_flags = CF_CONTROL_VIEW|CF_STRUCTURE_VIEW;
*/

Scope3::Scope3( IMpUnknown* host ) : MpBase( host )
,index_( 0 )
{
	// Associate each pin object with it's ID in the XML file
	initializePin( 0, pinSamplesA );
	initializePin( 1, pinSignalA );
	initializePin( 2, pinMs );
	//initializePin( 3, pinVoiceActive );
	//initializePin( 4, pinPolyDetect );
    initializePin( 3, pinPatchStoreOut );
    initializePin( 4, pinOn );
    initializePin( 5, pinLum );
}

int32_t Scope3::open()
{

	//SET_PROCESS( &Scope3::subProcess );

	channelActive_[0]= true; // TODO !! getPin(i)->IsConnected();

	// Scope3 must transmit an initial value on all output pins.
   // pinSamplesA.setValueRaw(0, 0);
	pinSamplesA.sendPinUpdate();


    //getSampleRate()
    //B_SIZE=SCOPE_BUFFER_SIZE;
   // int intValues[2];
   // intValues[0]=B_SIZE;
   // intValues[1]=SCOPE_BUFFER_SIZE;
   // pinPatchStoreOut.setValueRaw( sizeof(intValues), &intValues );
   // pinPatchStoreOut.sendPinUpdate();


	// determin if polyphonic or not.
	//int isCloned;
	//getHost()->isCloned( &isCloned );
	//pinPolyDetect = isCloned != 0;

   ms=getSampleRate()/1000.0f;

	return MpBase::open();
}

// wait for waveform to restart.
void Scope3::waitForTrigger1(int bufferOffset, int sampleFrames)
{
	float* signala	= bufferOffset + pinSignalA.getBuffer();

	for(int s = sampleFrames ; s > 0 ; s--)
	{
		if(*signala++ < 0.f)
		{
			index_ = 0;
			SET_PROCESS(&Scope3::waitForTrigger2);
			waitForTrigger2(bufferOffset + sampleFrames - s, s);
			return;
		}
	}


/*	timeout_ -= sampleFrames;
	if(timeout_ < 0)
	{
    SET_PROCESS(&Scope3::waitForTrigger2);
	}*/
}

void Scope3::waitForTrigger2(int bufferOffset, int sampleFrames)
{
	float* signala	= bufferOffset + pinSignalA.getBuffer();

	for(int s = sampleFrames ; s > 0 ; s--)
	{
		if(*signala++ > 0.f)
		{
			forceTrigger();
			(this->*(getSubProcess()))(bufferOffset + sampleFrames - s, s);
			return;
		}
	}


/*	timeout_ -= sampleFrames;
	if(timeout_ < 0)
	{
		forceTrigger();
	}*/
}

void Scope3::subProcess(int bufferOffset, int sampleFrames)
{
	// get pointers to in/output buffers
	float* signalA	= bufferOffset + pinSignalA.getBuffer();
    float* ms_in	= bufferOffset + pinMs.getBuffer();


	B_SIZE=(int)(*ms_in*ms*10.0f);
	if(B_SIZE<=11)B_SIZE=11;
	if(B_SIZE>=SCOPE_BUFFER_SIZE)B_SIZE=SCOPE_BUFFER_SIZE;

	int count = B_SIZE - index_;
	if(count > sampleFrames)
		count = sampleFrames;

	int remain = sampleFrames - count;

	//if( channelActive_[0] )
	//{
		int i = index_;
		for(int c = count ; c > 0 ;c--)
		{
            float limit=*signalA++ * 127.0f;
            if(limit>=127.0f)limit=127.0f;
            if(limit<=-127.0f)limit=-127.0f;
			assert( i < B_SIZE );
			//resultsA_[i++] = signed char(limit);
			resultsA_[i++] = (signed char) (limit);
		}
	//}
/*	if( channelActive_[1] )
	{
		int i = index_;
		for(int c = count ; c > 0 ;c--)
		{
			resultsB_[i++] = *signalB++;
		}
	}
*/
	index_ += count;

	if(index_ >= B_SIZE )//+ 1)
	{
		sendResultToGui(bufferOffset);
		(this->*(getSubProcess()))(bufferOffset + sampleFrames - remain, remain);
	}
}

// do nothing for 1/25th second.  Gives GUI time to display image.
void Scope3::subProcessCruise(int bufferOffset, int sampleFrames)
{


        timeout_ -= sampleFrames;
        if(timeout_ < 0)
        {
        // in absence of trigger signal, redraw 3 times per second.
        //timeout_ = (int)getSampleRate() / 3;
        SET_PROCESS(&Scope3::waitForTrigger1);
        }

/*
	int count = timeout_ - index_;
	if(count > sampleFrames)
		count = sampleFrames;
	index_ += count;

	if(index_ > timeout_ + 1 )
	{
    SET_PROCESS(&Scope3::waitForTrigger1);
	}*/
}

void Scope3::forceTrigger()
{
	index_ = 0;
	SET_PROCESS(&Scope3::subProcess);
}

void Scope3::sendResultToGui(int block_offset)
{
/*	int timeR=25;
	if(pinRefresh==0)timeR=25;
	if(pinRefresh==1)timeR=30;
    if(pinRefresh==2)timeR=50;
    if(pinRefresh==3)timeR=60;*/

    block=block_offset;
    int int_sr=int(getSampleRate());
    ms=getSampleRate()/1000.0f;


    int intValues[3];
    intValues[0]=B_SIZE;
    intValues[1]=int_sr;
    intValues[2]=int(pinLum*10.0f);
	pinSamplesA.setValueRaw( sizeof(resultsA_), &resultsA_ );
    pinPatchStoreOut.setValueRaw( sizeof(intValues), &intValues );
	pinSamplesA.sendPinUpdate( block_offset );
    pinPatchStoreOut.sendPinUpdate( block_offset );


	// waste of CPU to send updates more often than GUI can repaint,
	// wait approx 1/25th seccond between captures.
	//timeout_ = (int)getSampleRate() / 25;
    int min_dsp =(int)(getSampleRate()/20.0f);//15
	if(B_SIZE<=min_dsp)timeout_=min_dsp;
	else
	timeout_=B_SIZE;
	SET_PROCESS(&Scope3::subProcessCruise);
}

void Scope3::onSetPins(void)  // one or more pins_ updated.  Check pin update flags to determin which ones.
{

    //index_ = 0;
    //SET_PROCESS(&Scope3::subProcess);

    //if(pinSignalA.isUpdated() && pinSignalA !=0)
    if(pinOn.isUpdated())
	{
        if(pinOn==1)
        {
        index_ = 0;
        SET_PROCESS(&Scope3::subProcess);
        setSleep(false);
        }
        else
        {
        pinSamplesA.setValueRaw( 0,0);
        pinSamplesA.sendPinUpdate();
        setSleep(true);
        }
    }


	/*if( pinVoiceActive.isUpdated() )
	{
		//_RPT2(_CRT_WARN, "Scope3::onSetPins pinVoiceActive = %f [%x]\n", (double) pinVoiceActive, this );
        if( pinVoiceActive == 0.0f )
		{
			// send blank capture to indicate voice muted.
			//pinSamplesA.setValueRaw(0, 0);
			//pinSamplesA.sendPinUpdate();

			// do nothing.
			//timeout_ = (int)getSampleRate() / 2;
			SET_PROCESS(&Scope3::subProcessCruise);
		}
		else
		{
			//index_ = 0;
			//SET_PROCESS(&Scope3::subProcess);
		}
	}*/

}


