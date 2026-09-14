#include "PrgCh.h"
#include <Time.h>
REGISTER_PLUGIN (PrgCh, L"KxPrgChange" );

PrgCh::PrgCh( IMpUnknown* host ) : MpBase( host )
{
	// Register pins.
	initializePin( 0, PrgReset );
	initializePin( 1, Value );
	initializePin( 2, Time );
	initializePin( 3, ResetValue );
	initializePin( 4, MidiIn );
	initializePin( 5, Mode );
	initializePin( 6, pinRnd );
	initializePin( 7, pinTrig );
	initializePin( 8, ZeroTime );
}
int32_t PrgCh::open()
{
		// always call the base class.

    noteOn=0;
    init=1;
    ctrl_ch=false;
    rndnote_on=false;
    ms=getSampleRate()*0.01f; //to avoid to mul *time by 10 ...
    while_init=1;
    srand(time(0));
    /*float RANGE_MIN = 0.0f;
    float  RANGE_MAX = 2147483647.0f;
    m_Rnd = int(((float) rand() / (float) RAND_MAX) * RANGE_MAX + RANGE_MIN);*/


	return MpBase::open();
}
void PrgCh::onMidiMessage( int pin, unsigned char* midiMessage, int size )
{

	int s,c,n,v; // 3 bytes of MIDI message

	s = midiMessage[0] & 0xf0; //type+channel
    c = midiMessage[0] & 0x0f; //channel
	n = midiMessage[1]; //byte1 (ex: note)
	v = midiMessage[2]; //byte2 (ex: vel)


        /*/if(v !=0 && s>143 && s<160)//when "note_on" only the module goes to sleep, you can change this...
        {
        rndnote_on=true;//
        }
        else
        {
        rndnote_on=false;
        }*/





        if(v !=0 && s>143 && s<160 && ctrl_ch==true && Mode==2)//when "note_on" only the module goes to sleep, Midi_PrgCh=2
        {
        ctrl_ch=false;//
        ResetValue.setStreaming(false);//auto sleep mode
        SET_PROCESS(&PrgCh::subProcess_0);//fill the out buffer with 0
        }

       if(v !=0 && s>143 && s<160 && Mode==2 && noteOn==0)//note on = rnd generated if sub process is subProcess_infini, Midi_PrgCh=2
        {
        noteOn=1;
        //srand(0);
        float RANGE_MIN = 0.0f;
        float  RANGE_MAX = 2147483647.0f;
        int  r= int(((float) rand() / (float) RAND_MAX) * RANGE_MAX + RANGE_MIN);
        pinRnd=r;
        pinRnd.sendPinUpdate();
        }


       if(v !=0 && s>143 && s<160 && Mode==3)//note on = rnd generated always, Midi_RND=3
        {
        //srand(0);
        float RANGE_MIN = 0.0f;
        float  RANGE_MAX = 2147483647.0f;
        int  r= int(((float) rand() / (float) RAND_MAX) * RANGE_MAX + RANGE_MIN);
        pinRnd=r;
        pinRnd.sendPinUpdate();
        }

   // midiMessage[0]=s+c; you can use c to change the Midi channel
   // midiMessage[1]=n;
   // midiMessage[2]=v;
   //pinMIDIOut.send( midiMessage, size, blockPosition() );


}

void PrgCh::subProcess_infini( int bufferOffset, int sampleFrames )
{
	// get pointers to in/output buffers.
	float* value	= bufferOffset + Value.getBuffer();
	//float* time	= bufferOffset + Time.getBuffer();
	float* out	= bufferOffset + ResetValue.getBuffer();

	for( int s = sampleFrames; s > 0; --s )
	{

		*out=*value;

		++value;
		//++time;
		++out;
	}


}
void PrgCh::subProcess_time( int bufferOffset, int sampleFrames )
{
	// get pointers to in/output buffers.
	float* value	= bufferOffset + Value.getBuffer();
	float* time	= bufferOffset + Time.getBuffer();
	float* out	= bufferOffset + ResetValue.getBuffer();
	float* zerotime	= bufferOffset + ZeroTime.getBuffer();

	for( int s = sampleFrames; s > 0; --s )
	{
		if(Mode!=0 && while_init==1)limit=int(*zerotime*ms);
		else
        limit=int(*time*ms);

		*out=0.0f;

        if(count<limit)
		{
        if(Mode==0 || while_init==0)*out=*value;
        if(Mode!=0 && while_init==1)*out=0.0f;
		++count;
		}
		else
 		{
        ++sleepm;
 		}

		++value;
		++time;
		++out;
		++zerotime;
	}

	if(sleepm>=sampleFrames && count>=limit)//min time = 2 ms!
	{
    while_init=0;
    ResetValue.setStreaming(false);//auto sleep mode
    SET_PROCESS(&PrgCh::subProcess_0);//fill the out buffer with 0
	}


}
void PrgCh::subProcess_0( int bufferOffset, int sampleFrames )
{

	float* out	= bufferOffset + ResetValue.getBuffer();

	for( int s = sampleFrames; s > 0; --s )
	{
		*out=0.0f;
		++out;
	}
}
void PrgCh::onSetPins(void)
{

    //Plugin_INIT=0,Host_PrgCh=1,Midi_PrgCh=2,Midi_RND=3,Manual_RND=4,Init_RND=5

    if(init==1)
    {
        count=0;
        sleepm=0;
        ResetValue.setStreaming(true);
        SET_PROCESS(&PrgCh::subProcess_time);
    }

    if(init==1 && Mode==5)//Init_RND
    {
        srand(time(0));
        float RANGE_MIN = 0.0f;
        float  RANGE_MAX = 2147483647.0f;
        int  r= int(((float) rand() / (float) RAND_MAX) * RANGE_MAX + RANGE_MIN);
        pinRnd=r;
        pinRnd.sendPinUpdate();
    }


    //running when host prg_change (value out) but goes to sleep (zero output) after a time in milliseconde!
    if( PrgReset.isUpdated() && while_init==0 && Mode==1 && init==0) //Host_PrgCh
    {
        count=0;
        sleepm=0;
        ResetValue.setStreaming(true);
        SET_PROCESS(&PrgCh::subProcess_time);

        float RANGE_MIN = 0.0f;
        float  RANGE_MAX = 2147483647.0f;
        int  r= int(((float) rand() / (float) RAND_MAX) * RANGE_MAX + RANGE_MIN);
        pinRnd=r;
        pinRnd.sendPinUpdate();

    }

    //running when host prg_change (Value out) but goes to sleep (zero output) when a note on!
    if( PrgReset.isUpdated()  && ctrl_ch==false && while_init==0 && Mode==2 && init==0)//Midi_PrgCh
    {
        noteOn=0;
        ctrl_ch=true;
        ResetValue.setStreaming(true);
        SET_PROCESS(&PrgCh::subProcess_infini);
    }

    if(Mode==4 && pinTrig !=0.0f && pinTrig.isUpdated() && while_init==0 && init==0)//Manual_RND
    {
        //srand(0);
        float RANGE_MIN = 0.0f;
        float  RANGE_MAX = 2147483647.0f;
        int  r= int(((float) rand() / (float) RAND_MAX) * RANGE_MAX + RANGE_MIN);
        pinRnd=r;
        pinRnd.sendPinUpdate();
    }

    init=0;
}

