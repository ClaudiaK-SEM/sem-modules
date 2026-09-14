#include "MidiOnOff.h"

REGISTER_PLUGIN ( MidiOnOff, L"kx midi on off" );

/*#define NOTE_OFF                0x80
#define NOTE_ON                 0x90
#define SYSTEM_MSG				0xF0
#define POLY_AFTERTOUCH         0xA0
#define SYSTEM_EXCLUSIVE        0xF0
#define UNIVERSAL_REAL_TIME     0x7F
#define UNIVERSAL_NON_REAL_TIME 0x7E
#define SUB_ID_TUNING_STANDARD  0x08*/


MidiOnOff::MidiOnOff( IMpUnknown* host ) : MpBase( host )
{

	// Register pins.
	initializePin( 0, pinMIDIIn );
	initializePin( 1, pinMIDIOut );
	initializePin( 2, pinChannel);
	initializePin( 3, pinOn);
    init=0;
}

void MidiOnOff::onMidiMessage( int pin, unsigned char* midiMessage, int size )
{
	int s,c,n,v;// 3 bytes of MIDI message

	s = midiMessage[0] & 0xf0;
    c = midiMessage[0] & 0x0f;
	n = midiMessage[1];
	v = midiMessage[2];

	if(pinOn==1)
    {
        midiMessage[0]=s+c;
        if(pinChannel!=-1)midiMessage[0]=s+pinChannel;
        midiMessage[1]=n;
        midiMessage[2]=v;
        pinMIDIOut.send( midiMessage, size, blockPosition() );
    }



}
// Process audio.
/*void MidiOnOff::subProcess( int bufferOffset, int sampleFrames )
{
	// assign pointers to your in/output buffers. Each buffer is an array of float samples.
	float* in1  = bufferOffset + pinInput1.getBuffer();
	float* in2  = bufferOffset + pinInput2.getBuffer();
	float* out1 = bufferOffset + pinOutput1.getBuffer();

	for( int s = sampleFrames; s > 0; --s ) // sampleFrames = how many samples to process (can vary). repeat (loop) that many times
	{
		float input1 = *in1;	// get the sample 'POINTED TO' by in1.
		float input2 = *in2;

		// Multiplying the two input's samples together.
		float result = input1 * input2;

		// store the result in the output buffer.
		*out1 = result;

		// increment the pointers (move to next sample in buffers).
		++in1;
		++in2;
		++out1;
	}
}
*/

// One or more inputs updated.  Check pin update flags to determin which ones.
void MidiOnOff::onSetPins(void)
{

    if( pinChannel.isUpdated() || (pinOn.isUpdated() && pinOn==0))
	{
        if(init==1)
        {

           if(m_channel==-1)
           {

                for( int n = 0 ; n < 16 ; ++n )
                {
                unsigned char midiMsg[4];
                midiMsg[0] = 176 + n;
                midiMsg[1] = 123;
                midiMsg[2] = 0;//debug 14273 in place of 0
                //pinMIDIOut.send( midiMsg,3, blockPosition() );
                pinMIDIOut.send( midiMsg,3, -1 );
                }


           }
           else
           {
            unsigned char midiMsg[4];
            midiMsg[0] = 176 + m_channel;
            midiMsg[1] = 123;
            midiMsg[2] = 0;//debug 14273 in place of 0
            //pinMIDIOut.send( midiMsg,3, blockPosition() );
            pinMIDIOut.send( midiMsg,3, -1 );
           }

        }
	m_channel=pinChannel;
	init=1;
	}



	/*
	// LEVEL 1 - Simplest way to handle streaming status...
	// Do nothing. That's it.


	// LEVEL 2 - Determin if output is silent or active, then notify downstream modules.
	//           Downstream modules can then 'sleep' (save CPU) when processing silence.

	// If either input is active, output will be active. ( "||" means "or" ).
	bool OutputIsActive = pinInput1.isStreaming() || pinInput2.isStreaming();

	// Exception...
	// If either input zero, output is silent.
	if( !pinInput1.isStreaming() && pinInput1 == 0.0f )
	{
		OutputIsActive = false;
	}

	if( !pinInput2.isStreaming() && pinInput2 == 0.0f )
	{
		OutputIsActive = false;
	}

	// Transmit new output state to modules 'downstream'.
	pinOutput1.setStreaming( OutputIsActive );

	// Choose which function is used to process audio.
	// You can have one processing method, or several variations (each specialized for a certain condition).
	// For this example only one processing function needed.
	SET_PROCESS( &Gain::subProcess );

	// Normally module will sleep when no inputs or outputs are streaming,
	// however this module is an exception - when the volume on one input pin is zero, module can sleep
	// regardless of the other input.

	// control sleep mode manually.
	setSleep( !OutputIsActive );
*/
}
