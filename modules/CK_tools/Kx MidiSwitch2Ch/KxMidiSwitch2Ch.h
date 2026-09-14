#ifndef KXMIDISWITCH2CH_H_INCLUDED
#define KXMIDISWITCH2CH_H_INCLUDED

#include "mp_sdk_audio.h"

class KxMidiSwitch2Ch : public MpBase
{
public:
	KxMidiSwitch2Ch( IMpUnknown* host );
	//void subProcess( int bufferOffset, int sampleFrames );
    virtual void onMidiMessage(int pin, unsigned char* midiMessage, int size); // size < 4 for short msg, or > 4 for sysex
    //virtual int32_t MP_STDCALL open();
	virtual void onSetPins(void);

private:
	MidiInPin MidiIn;
	MidiOutPin MidiOutA;
	MidiOutPin MidiOutB;
	IntInPin sw;
	IntInPin allctrl;

    //int note_on;
    //int count;
	int init;
	int m_sw;
	int m_chan;
	int chan;
	//int onfilter;

	//int ctrl;
	//int n_buffer;
	//int note[32];

    unsigned char midiMsg[4];



};

#endif

