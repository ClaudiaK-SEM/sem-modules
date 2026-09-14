// Copyright 2006 Jeff McClintock

#ifndef Scope3_dsp_H_INCLUDED
#define Scope3_dsp_H_INCLUDED

#include "mp_sdk_audio.h"

#define SCOPE_BUFFER_SIZE 4800
#define SCOPE_CHANNELS 1
typedef signed char ScopeResults[SCOPE_BUFFER_SIZE];

class Scope3: public MpBase
{
public:
	Scope3(IMpUnknown* host);

	// overrides
	virtual int32_t MP_STDCALL open();

	// methods
	void subProcess(int bufferOffset, int sampleFrames);
	void waitForTrigger1(int bufferOffset, int sampleFrames);
	void waitForTrigger2(int bufferOffset, int sampleFrames);
	void subProcessCruise(int bufferOffset, int sampleFrames);
	void forceTrigger();
	void onSetPins(void);
private:
	void sendResultToGui(int block_offset);

	// pins
	BlobOutPin pinSamplesA;
	AudioInPin pinSignalA;
	AudioInPin pinMs;
    BlobOutPin pinPatchStoreOut;
   	IntInPin pinOn;
	AudioInPin pinLum;
   // EnumInPin pinRefresh;

    int block;
    float ms;
	int B_SIZE;

	int index_;
	int timeout_;
	ScopeResults resultsA_;
	bool channelActive_[SCOPE_CHANNELS];
};

#endif
