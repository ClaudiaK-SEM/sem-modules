#ifndef MidiOnOff_H_INCLUDED
#define MidiOnOff_H_INCLUDED

#include "mp_sdk_audio.h"
//#include "hasMidiTuning.h"

//class MidiOnOff : public MpBase, public hasMidiTuning
class MidiOnOff : public MpBase
{
public:
	MidiOnOff( IMpUnknown* host );
	//void subProcess( int bufferOffset, int sampleFrames );
	virtual void onMidiMessage(int pin, unsigned char* midiMessage, int size); // size < 4 for short msg, or > 4 for sysex
    virtual void onSetPins(void);

private:
	MidiInPin pinMIDIIn;
	MidiOutPin pinMIDIOut;
    IntInPin pinChannel;
    IntInPin pinOn;
    int m_channel;
    int init;
};

#endif

