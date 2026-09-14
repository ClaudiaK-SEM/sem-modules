#include ".\Kxmidifilterall.h"

#define SUB_ID_TUNING_STANDARD  0x08
#define UNIVERSAL_NON_REAL_TIME 0x7E
#define UNIVERSAL_REAL_TIME     0x7F

#define NOTE_OFF				0x80 //128-143
#define NOTE_ON					0x90 //144-159
#define POLY_AFTERTOUCH			0xA0 //160-175
#define CONTROL_CHANGE			0xB0 //176-191
#define PROGRAM_CHANGE			0xC0 //192-207
#define CHANNEL_PRESSURE		0xD0 //208-223
#define PITCHBEND				0xE0 //224-239

#define SYSTEM_MSG				0xF0	/* F0 -> FF */
#define SYSTEM_EXCLUSIVE		0xF0
#define MIDI_CLOCK				0xF8
#define MIDI_CLOCK_START		0xFA
#define MIDI_CLOCK_CONT			0xFB
#define MIDI_CLOCK_STOP			0xFC
#define ACTIVE_SENSING			0xFE
#define MIDI_CC_ALL_SOUND_OFF	120
#define MIDI_CC_ALL_NOTES_OFF	123



REGISTER_PLUGIN ( Kxmidifilterall, L"My KX-MidiFilterALL" );

Kxmidifilterall::Kxmidifilterall( IMpUnknown* host ) : MpBase( host )
{
	// Register pins.
	initializePin( 0, PN_MIDI_IN );
	initializePin( 1, PN_MIDI_OUT );
	initializePin( 2, cc_f1 );//cc_f1
	initializePin( 3, cc_f2 );//cc_f2
	initializePin( 4, cc_f3 );//cc_f3
	initializePin( 5, cc_f4 );//cc_f4
	initializePin( 6, cc_f5 );//cc_f5
	initializePin( 7, f_prg );//f_prg
	initializePin( 8, f_bender );//f_bender
	initializePin( 9, f_note );//f_note
	initializePin( 10, f_after );//f_after
	initializePin( 11, f_pafter );//f_pafter
	initializePin( 12, f_cc );//f_cc
	initializePin( 13, reset );//reset
	initializePin( 14, can );//can
	initializePin( 15, PN_MIDI_PRG );
	initializePin( 16, PN_MIDI_BENDER );
	initializePin( 17, PN_MIDI_NOTE );
	initializePin( 18, PN_MIDI_AFTER );
	initializePin( 19, PN_MIDI_PAFTER );
	initializePin( 20, PN_MIDI_CC );
	initializePin( 21, cc_m1 );//cc_m1
	initializePin( 22, cc_m2 );//cc_m2
	initializePin( 23, cc_m3 );//cc_m3
	initializePin( 24, cc_m4 );//cc_m4
	initializePin( 25, cc_m5 );//cc_m5
	initializePin( 26, cc_on_m1 );//cc_on_m1
	initializePin( 27, cc_on_m2 );//cc_on_m2
	initializePin( 28, cc_on_m3 );//cc_on_m3
	initializePin( 29, cc_on_m4 );//cc_on_m4
	initializePin( 30, cc_on_m5 );//cc_on_m5
	initializePin( 31, can_prg );//can_prg
	initializePin( 32, can_bender );//can_bender
	initializePin( 33, can_note );//can_note
	initializePin( 34, can_after );//can_after
	initializePin( 35, can_pafter );//can_pafter
	initializePin( 36, can_cc );//can_cc
	initializePin( 37, trsp );//trsp
	initializePin( 38, rout );//rout
	initializePin( 39, MulV );//rout
    initializePin( 40, ChannelLo );
    initializePin( 41, ChannelHi );
    initializePin( 42, NoteLo );
    initializePin( 43, NoteHi );
    initializePin( 44, VelocityLo );
    initializePin( 45, VelocityHi );

    init=1;
    //m_ch_pa=0;
    //m_ch_n=0;
    //m_trsp=0;
    //m_noteLo=0;
    //m_noteHi=127;
    //m_ChannelLo=0;
    //m_ChannelHi=15;
}

// The core of a MIDI plugin
void Kxmidifilterall::onMidiMessage( int pin, unsigned char* midiMessage, int size )
{
	if(size<4)// size < 4 for short msg, or > 4 for sysex
	{
	int s,c,n,v;// 3 bytes of MIDI message

	s = midiMessage[0] & 0xf0;
    c = midiMessage[0] & 0x0f;
	n = midiMessage[1];
	v = midiMessage[2];





    if(c>=ChannelLo-1 && c<=ChannelHi-1 )
    {

    // ctrl
    int sc=s+c;

            // note on
            //if(v !=0 && s>143 && s<160)
            //note off
            //if(s>127 && s<144)
            // vel=0 note off
            //if(v==0 && s>143 && s<160)

            int ch_p=c;
            int ch_b=c;
            ch_n=c;
            int ch_a=c;
            int ch_pa=c;
            int ch_cc=c;
            //mcan=c;

                    //notes
                    if( sc>127 && sc<160 && f_note==0)
                    {


                    if(can_note!=-1)ch_n=can_note;

                        //if( sc>143 && sc<160)mevent=1;

                        if(n>=NoteLo && n<=NoteHi)
                        {
                            int t=trsp+n;
                            if(t>127)t=127;
                            if(t<0)t=0;
                            int vv=v;

                            if(v !=0 && s>143 && s<160)// note on
                            {
                            vv=MulV*v;
                            if(vv>VelocityHi)vv=VelocityHi;
                            if(vv<VelocityLo)vv=VelocityLo;
                            }

                            midiMsg[0] = s + ch_n;
                            midiMsg[1] = t;
                            midiMsg[2] = vv;
                            PN_MIDI_OUT.send( midiMsg, size, blockPosition() );
                            PN_MIDI_NOTE.send( midiMsg, size, blockPosition() );
                            m_ch_n=ch_n;

                        }
                    }

                    //poly after
                    if( sc>159 && sc<176 && f_pafter==0)
                    {

                        if(can_pafter!=-1)ch_pa=can_pafter;


                        if(n>=NoteLo && n<=NoteHi)
                        {
                            int t=trsp+n;
                            if(t>127)t=127;
                            if(t<0)t=0;
                            midiMsg[0] =  s + ch_pa;
                            midiMsg[1] = t;
                            midiMsg[2] = v;
                            PN_MIDI_OUT.send( midiMsg, size, blockPosition() );
                            PN_MIDI_PAFTER.send( midiMsg, size, blockPosition() );
                            m_ch_pa=ch_pa;

                        }

                    }

                    //ctrl
                    if( sc>175 && sc<192 && f_cc==0)
                    {

                            if(can_cc!=-1)ch_cc=can_cc;

                            if(n==cc_f1 || n==cc_f2 || n==cc_f3 || n==cc_f4 || n==cc_f5)
                            {
                                    if(cc_on_m1==1 && n==cc_f1)
                                    {
                                    midiMsg[0] = s + ch_cc ;
                                    midiMsg[1] = cc_m1;
                                    midiMsg[2] = v;
                                    PN_MIDI_OUT.send( midiMsg, size, blockPosition() );
                                    PN_MIDI_CC.send( midiMsg, size, blockPosition() );

                                    }
                                    if(cc_on_m2==1 && n==cc_f2)
                                    {
                                    midiMsg[0] = s + ch_cc ;
                                    midiMsg[1] = cc_m2;
                                    midiMsg[2] = v;
                                    PN_MIDI_OUT.send( midiMsg, size, blockPosition() );
                                    PN_MIDI_CC.send( midiMsg, size, blockPosition() );
                                    }
                                    if(cc_on_m3==1 && n==cc_f3)
                                    {
                                    midiMsg[0] = s + ch_cc ;
                                    midiMsg[1] = cc_m3;
                                    midiMsg[2] = v;
                                    PN_MIDI_OUT.send( midiMsg, size, blockPosition() );
                                    PN_MIDI_CC.send( midiMsg, size, blockPosition() );
                                    }
                                    if(cc_on_m4==1 && n==cc_f4)
                                    {
                                    midiMsg[0] = s + ch_cc ;
                                    midiMsg[1] = cc_m4;
                                    midiMsg[2] = v;
                                    PN_MIDI_OUT.send( midiMsg, size, blockPosition() );
                                    PN_MIDI_CC.send( midiMsg, size, blockPosition() );
                                    }
                                    if(cc_on_m5==1 && n==cc_f5)
                                    {
                                    midiMsg[0] = s + ch_cc ;
                                    midiMsg[1] = cc_m5;
                                    midiMsg[2] = v;
                                    PN_MIDI_OUT.send( midiMsg, size, blockPosition() );
                                    PN_MIDI_CC.send( midiMsg, size, blockPosition() );

                                    }


                            }
                            else
                            {
                            midiMsg[0] = s + ch_cc ;
                            midiMsg[1] = n;
                            midiMsg[2] = v;
                            PN_MIDI_OUT.send( midiMsg, size, blockPosition() );
                            PN_MIDI_CC.send( midiMsg, size, blockPosition() );
                            }

                            m_ch_cc=ch_cc;


                    }

                    //prg
                    if( sc>191 && sc<208 && f_prg==0)
                    {
                    if(can_prg!=-1)ch_p=can_prg;
                    midiMsg[0] = s + ch_p  ;
                    midiMsg[1] = n;
                    midiMsg[2] = v;
                    PN_MIDI_OUT.send( midiMsg, size, blockPosition() );
                    PN_MIDI_PRG.send( midiMsg, size, blockPosition() );
                    m_ch_p=ch_p;
                    }

                     //after
                    if( sc>207 && sc<224 && f_after==0)
                    {
                    if(can_after!=-1)ch_a=can_after;
                    midiMsg[0] = s + ch_a  ;
                    midiMsg[1] = n;
                    midiMsg[2] = v;
                    PN_MIDI_OUT.send( midiMsg, size, blockPosition() );
                    PN_MIDI_AFTER.send( midiMsg, size, blockPosition() );
                    m_ch_a=ch_a;
                    }

                    //bend
                    if( sc>223 && sc<240 && f_bender==0)
                    {
                    if(can_bender!=-1)ch_b=can_bender;
                    midiMsg[0] = s + ch_b  ;
                    midiMsg[1] = n;
                    midiMsg[2] = v;
                    PN_MIDI_OUT.send( midiMsg, size, blockPosition() );
                    PN_MIDI_BENDER.send( midiMsg, size, blockPosition() );
                    m_ch_b=ch_b;
                    }

            }



    }

}

void Kxmidifilterall::onSetPins(void)
{


                    if( init==0 && (m_can_note!=can_note) || (m_trsp!=trsp) || (m_noteLo!=NoteLo) || (m_noteHi!=NoteHi) || (m_ChannelLo!=ChannelLo) || (m_ChannelHi!=ChannelHi))
                    {
                    midiMsg[0] = 176+m_ch_n;
                    midiMsg[1] = 123;
                    midiMsg[2] = 0;
                    PN_MIDI_OUT.send( midiMsg, 3, blockPosition() );
                    PN_MIDI_NOTE.send( midiMsg, 3, blockPosition() );
                    }

                    if( init==0 && reset==1 && m_reset==0)
                    {

                        for(int i=0;i<16;i++)
                        {
                                midiMsg[0] = 176+i;
                                //midiMsg[0] = 176+ mcan;
                                midiMsg[1] = 121;
                                midiMsg[2] = 0;


                                if(rout==1 || rout==0)PN_MIDI_PRG.send( midiMsg, 3, blockPosition() );

                                if(rout==2 || rout==0)PN_MIDI_BENDER.send( midiMsg, 3, blockPosition() );

                                if(rout==3 || rout==0)PN_MIDI_NOTE.send( midiMsg, 3, blockPosition() );

                                if(rout==4 || rout==0)PN_MIDI_AFTER.send( midiMsg, 3, blockPosition() );

                                if(rout==5 || rout==0)PN_MIDI_PAFTER.send( midiMsg, 3, blockPosition() );

                                if(rout==6 || rout==0)PN_MIDI_OUT.send( midiMsg, 3, blockPosition() );


                                midiMsg[0] = 176+i;
                                //midiMsg[0] = 176+ mcan;
                                midiMsg[1] = 123;
                                midiMsg[2] = 0;


                                if(rout==1 || rout==0)PN_MIDI_PRG.send( midiMsg, 3, blockPosition() );

                                if(rout==2 || rout==0)PN_MIDI_BENDER.send( midiMsg, 3, blockPosition() );

                                if(rout==3 || rout==0)PN_MIDI_NOTE.send( midiMsg, 3, blockPosition() );

                                if(rout==4 || rout==0)PN_MIDI_AFTER.send( midiMsg, 3, blockPosition() );

                                if(rout==5 || rout==0)PN_MIDI_PAFTER.send( midiMsg, 3, blockPosition() );

                                if(rout==6 || rout==0)PN_MIDI_OUT.send( midiMsg, 3, blockPosition() );


                        }
                    m_reset=reset;
                    }

                    if(reset.isUpdated() && reset==0)
                    {
                    m_reset=reset;
                    }

                    init=0;
                    m_can_note=can_note;
                    m_ch_n=ch_n;
                    m_trsp=trsp;
                    m_noteLo=NoteLo;
                    m_noteHi=NoteHi;
                    m_ChannelLo=ChannelLo;
                    m_ChannelHi=ChannelHi;


}

