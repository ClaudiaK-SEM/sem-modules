#ifndef RndVoice_H_INCLUDED
#define RndVoice_H_INCLUDED

#include <math.h>
#include "mp_sdk_audio.h"
//#include "smart_audio_pin.h"

class RndVoice : public MpBase
{
public:
	RndVoice( IMpUnknown* host );

	void sub_process( int bufferOffset, int sampleFrames );
    //void sub_process_0( int bufferOffset, int sampleFrames );
	//void sub_process_static( int bufferOffset, int sampleFrames );
	virtual void onSetPins(void);
	//virtual int32_t MP_STDCALL open();
    //void ChooseSubProcess( int blockPosition );

private:

	//bool init;
	//float reset;
	float rnd;
	FloatInPin pinVoiceReset;
    AudioInPin pinGateIn;
	AudioOutPin pinOutput;
	IntInPin pinRnd;
	AudioInPin pinMul;
	FloatInPin pinMulRnd;
	IntInPin pinOn;
	FloatInPin pinOffRnd;
};

#endif

