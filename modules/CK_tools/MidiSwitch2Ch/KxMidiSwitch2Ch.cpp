#include "KxMidiSwitch2Ch.h"

REGISTER_PLUGIN ( KxMidiSwitch2Ch, L"My KX MIDI SWITCH2 CH" );

KxMidiSwitch2Ch::KxMidiSwitch2Ch( IMpUnknown* host ) : MpBase( host )
{
	// Register pins.
	initializePin( 0, MidiIn );
	initializePin( 1, MidiOutA );
	initializePin( 2, MidiOutB );
	initializePin( 3, sw );
	initializePin( 4, allctrl );

    init=1;
}
// The core of a MIDI plugin
void KxMidiSwitch2Ch::onMidiMessage( int pin, unsigned char* midiMessage, int size )
{
	if(size<4)// size < 4 for short msg, or > 4 for sysex
	{
	int s,c,n,v;// 3 bytes of MIDI message

	s = midiMessage[0] & 0xf0;
    chan = midiMessage[0] & 0x0f;
	n = midiMessage[1];
	v = midiMessage[2];

    midiMsg[0] = s + chan;
    midiMsg[1] = n;
    midiMsg[2] = v;
    if(sw==0)MidiOutA.send( midiMsg, size, blockPosition() );
    if(sw==1)MidiOutB.send( midiMsg, size, blockPosition() );

	}

}



void KxMidiSwitch2Ch::onSetPins(void)
{

        if( init==0 && sw !=m_sw )
        {
            for(int i=0;i<16;i++)
            {
            midiMsg[0] = 176+i;
            midiMsg[1] = 123;
            midiMsg[2] = 0;
            if(sw==0 && m_sw==1)MidiOutB.send( midiMsg, 3, blockPosition() );
            if(sw==1 && m_sw==0)MidiOutA.send( midiMsg, 3, blockPosition() );
            }

		m_sw=sw;

        }

		if (init==0 && chan !=m_chan && allctrl==1 )
		{
        midiMsg[0] = 176+m_chan;
        midiMsg[1] = 123;
        midiMsg[2] = 0;
        if(sw==0)MidiOutA.send( midiMsg, 3, blockPosition() );
        if(sw==1)MidiOutB.send( midiMsg, 3, blockPosition() );

		m_chan=chan;

		}

    init=0;
	m_chan=chan;
	m_sw=sw;
}

