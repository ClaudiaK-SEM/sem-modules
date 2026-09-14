#ifndef KXMIDIFILTERALL_H_INCLUDED
#define KXMIDIFILTERALL_H_INCLUDED

#include "mp_sdk_audio.h"

class Kxmidifilterall : public MpBase
{
public:
	Kxmidifilterall( IMpUnknown* host );
    //void subProcess( int bufferOffset, int sampleFrames );
	virtual void onMidiMessage(int pin, unsigned char* midiMessage, int size); // size < 4 for short msg, or > 4 for sysex
	virtual void onSetPins(void);

private:

	MidiInPin PN_MIDI_IN;
	MidiOutPin PN_MIDI_OUT;
	IntInPin cc_f1;//cc_f1
	IntInPin cc_f2;//cc_f2
	IntInPin cc_f3;//cc_f3
	IntInPin cc_f4;//cc_f4
	IntInPin cc_f5;//cc_f5
	IntInPin f_prg;//f_prg
	IntInPin f_bender;//f_bender
	IntInPin f_note;//f_note
	IntInPin f_after;//f_after
	IntInPin f_pafter;//f_pafter
	IntInPin f_cc;//f_cc
	IntInPin reset;//reset
	IntInPin can;//can
	MidiOutPin PN_MIDI_PRG;
	MidiOutPin PN_MIDI_BENDER;
	MidiOutPin PN_MIDI_NOTE;
	MidiOutPin PN_MIDI_AFTER;
	MidiOutPin PN_MIDI_PAFTER;
	MidiOutPin PN_MIDI_CC;
	IntInPin cc_m1;//cc_m1
	IntInPin cc_m2;//cc_m2
	IntInPin cc_m3;//cc_m3
	IntInPin cc_m4;//cc_m4
	IntInPin cc_m5;//cc_m5
	IntInPin cc_on_m1;//cc_on_m1
	IntInPin cc_on_m2;//cc_on_m2
	IntInPin cc_on_m3;//cc_on_m3
	IntInPin cc_on_m4;//cc_on_m4
	IntInPin cc_on_m5;//cc_on_m5
	IntInPin can_prg;//can_prg
	IntInPin can_bender;//can_bender
	IntInPin can_note;//can_note
	IntInPin can_after;//can_after
	IntInPin can_pafter;//can_pafter
	IntInPin can_cc;//can_cc
	IntInPin trsp;//trsp
	IntInPin rout;//rout
	FloatInPin MulV;//rout
	IntInPin ChannelLo;
	IntInPin ChannelHi;
	IntInPin NoteLo;
	IntInPin NoteHi;
	IntInPin VelocityLo;
	IntInPin VelocityHi;

 int init;
 int m_reset;
 int mcan;
 int mevent;

 int ch_n;
 int m_ch_n;
 int m_ch_pa;
 int m_ch_p;
 int m_ch_b;
 int m_ch_a;
 int m_ch_cc;

 int m_trsp;
 int m_noteLo;
 int m_noteHi;
 int m_ChannelLo;
 int m_ChannelHi;
 int m_can_note;


 unsigned char midiMsg[4];


};

#endif

