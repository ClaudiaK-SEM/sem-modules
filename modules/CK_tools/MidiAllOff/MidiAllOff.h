#ifndef MidiAllOff_H_INCLUDED
#define MidiAllOff_H_INCLUDED

#include "mp_sdk_audio.h"
//#include "hasMidiTuning.h"

//class MidiAllOff : public MpBase, public hasMidiTuning
class MidiAllOff : public MpBase
{
public:
	MidiAllOff( IMpUnknown* host );
	//void subProcess( int bufferOffset, int sampleFrames );
	virtual void onMidiMessage(int pin, unsigned char* midiMessage, int size); // size < 4 for short msg, or > 4 for sysex
    virtual void onSetPins(void);

private:
	MidiInPin pinMIDIIn;
	MidiOutPin pinMIDIOut;
    IntInPin pinChannel;
    int m_channel;
    int init;
};

#endif

