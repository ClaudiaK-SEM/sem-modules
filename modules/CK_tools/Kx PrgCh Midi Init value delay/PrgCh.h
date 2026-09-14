#ifndef PrgCh_H_INCLUDED
#define PrgCh_H_INCLUDED

#include "mp_sdk_audio.h"
//#include "hasMidiTuning.h"

//class PrgCh : public MpBase, public hasMidiTuning
class PrgCh : public MpBase
{
public:
	PrgCh( IMpUnknown* host );
	void subProcess_0( int bufferOffset, int sampleFrames );
	void subProcess_time( int bufferOffset, int sampleFrames );
    void subProcess_infini( int bufferOffset, int sampleFrames );
	virtual void onMidiMessage(int pin, unsigned char* midiMessage, int size); // size < 4 for short msg, or > 4 for sysex
    virtual void onSetPins(void);
	virtual int32_t MP_STDCALL open();

private:

	IntInPin PrgReset;
	AudioInPin Value;
	AudioInPin Time;
	AudioOutPin ResetValue;
	MidiInPin MidiIn;
	IntInPin Mode;
	IntOutPin pinRnd;
	FloatInPin pinTrig;
	AudioInPin ZeroTime;

	int noteOn;
	int m_Rnd;
	int prg;
	int init;
	int sleepm;
	int count;
	bool ctrl_ch;
	float ms;
	int limit;
	bool rndnote_on;
    int while_init;
};
#endif
