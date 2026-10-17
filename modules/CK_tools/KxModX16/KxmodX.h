#ifndef KxmodX_H_INCLUDED
#define KxmodX_H_INCLUDED

#include "mp_sdk_audio.h"

class KxmodX : public MpBase
{
public:
	KxmodX( IMpUnknown* host );
	//void subProcess( int bufferOffset, int sampleFrames );
	virtual void onMidiMessage(int pin, unsigned char* midiMessage, int size); // size < 4 for short msg, or > 4 for sysex
	virtual void onSetPins(void);

private:



	MidiInPin pinMIDIIn;
	MidiOutPin pinMIDIOut1;
	MidiOutPin pinMIDIOut2;
    MidiOutPin pinMIDIOut3;
	MidiOutPin pinMIDIOut4;
	MidiOutPin pinMIDIOut5;
    MidiOutPin pinMIDIOut6;
    MidiOutPin pinMIDIOut;
	IntInPin uni;
	IntInPin voice_ammount;
	IntInPin cycle;
	IntInPin fflag;
	IntInPin ctrl;
	IntInPin onfilter;
	IntInPin ch1;
	IntInPin ch2;
	IntInPin ch3;
	IntInPin ch4;
	IntInPin ch5;
	IntInPin ch6;
	IntInPin power;
	IntInPin can;


    void reset();///////////

    unsigned char midiMessage2[4];

	int init;
	//short fvoice;
	//int muni;
    //short ch1,ch2,ch3,ch4,ch5,ch6;

    //bool ctrl_ch;
	bool state;

    int c_uni;

	//short uni;
	//short onfilter;
	int allctrl;
	//short ctrl;
	//short fflag;
	//short cycle;
	//short voice_ammount;
	int on[8];
	int somme;
	int last;
	int note[8];
	int inc;
	int chn[8];
	int first[8];
	//int lst[8];
	int mem;
	int f;
    int fs;
    //short power;
    unsigned int m_clock;

    //short can;
};

#endif

