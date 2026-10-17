#include "KxmodCS.h"


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








REGISTER_PLUGIN ( KxmodCS, L"KX-MOD CS" );

KxmodCS::KxmodCS( IMpUnknown* host ) : MpBase( host )
{
	// Register pins.
	initializePin( 0, pinMIDIIn );
	initializePin( 1, pinMIDIOut );
	initializePin( 2, uni );
	//initializePin( 3, voice_ammount );
	initializePin( 3, nvoice );
	initializePin( 4, cycle );
	initializePin( 5, fflag );
	initializePin( 6, ctrl );
	initializePin( 7, onfilter );
	//initializePin( 8, ch1 );
	//initializePin( 9, ch2 );
	//initializePin( 10, ch3 );
	//initializePin( 11, ch4 );
	//initializePin( 12, ch5 );
	//initializePin( 13, ch6 );
	initializePin( 8, power );
	initializePin( 9, can );
	initializePin( 10, pinMIDIOut2 );
	initializePin( 11, Lo1 );
	initializePin( 12, Hi1 );
	initializePin( 13, Lo2 );
	initializePin( 14, Hi2 );
	initializePin( 15, monodebug );
	initializePin( 16, NoteId );
	//initializePin( 15, AllNoteOff );
	//initializePin( 16, pinMIDIcc123 );

    init=0;
    //muni=-1;
    c_uni=0;
    state=false;
	somme=0;
	inc=0;
	last=0;
	f=0;
	for(int i=0;i<8;i++)
	{
	on[i]=0;
	note[i]=-1;
	chn[i]=i;
    first[i]=0;
	}

}

void KxmodCS::onMidiMessage( int pin, unsigned char* midiMessage, int size )
{



	if(power==1 && size<4)
	{

    int voice_ammount=nvoice;
    //if(uni==1 && nvoice>=3)voice_ammount=3; // Poly Unison of 3*2 notes
    if(uni==1 && nvoice>=4)voice_ammount=4; // Poly Unison of 4*2 notes
    int d_voice=3;
    if(nvoice>=3)d_voice=2;


	int s,c,n,v;// 3 bytes of MIDI message

	s = midiMessage[0] & 0xf0;
    c = midiMessage[0] & 0x0f;
	n = midiMessage[1];
	v = midiMessage[2];

    //***********************************************************************************************************************************************************/



         if(can==c || can==-1)
         {


         m_can=c;
            //**********************************************************************************************************************************************************************/
                //if(uni==0)
                //{
                    // vel=0 note off
                    //if(v==0 && s>143 && s<160)
                    if(v==0 && s==144)
                    {

                        if(n>=Lo2 && n<=Hi2)
                        {
                        midiMsg2[0]=s+c;
                        midiMsg2[1]=n;
                        midiMsg2[2]=v;
                        pinMIDIOut2.send( midiMsg2, size, blockPosition() );
                        }

                        if(n>=Lo1 && n<=Hi1)
                        {
                            for(int i=0;i<voice_ammount;i++)
                            {
                                if(on[i]==1 && note[i]==n )
                                {
                                //midiMsg[0]=s+c;
                                //midiMsg[1]=n;
                                //midiMsg[2]=v;
                                //pinMIDIOut.send( midiMsg, size, blockPosition() );

                                    if(uni==0)
                                    {
                                    midiMsg[0]=s+c;
                                    midiMsg[1]=n;
                                    midiMsg[2]=v;
                                    pinMIDIOut.send( midiMsg, size, blockPosition() );
                                    }
                                    else
                                    {
                                        for(int ch=0;ch<d_voice;ch++)// 2 or 3 voices per note
                                        {
                                        midiMsg[0]=s+ch;
                                        midiMsg[1]=n;
                                        midiMsg[2]=v;
                                        pinMIDIOut.send( midiMsg, size, blockPosition() );
                                        }
                                    }


                                on[i]=0;
                                note[i]=-1;
                                goto exit;
                                }
                            }


                        }

                    }

                    //note off
                    //if(s>127 && s<144)
                    if(s==128)
                    {

                        if(n>=Lo2 && n<=Hi2)
                        {
                        midiMsg2[0]=s+c;
                        midiMsg2[1]=n;
                        midiMsg2[2]=v;
                        pinMIDIOut2.send( midiMsg2, size, blockPosition() );
                        }

                        if(n>=Lo1 && n<=Hi1)
                        {
                            for(int i=0;i<voice_ammount;i++)
                            {
                                if(on[i]==1 && note[i]==n )
                                {
                                //midiMsg[0]=s+c;
                                //midiMsg[1]=n;
                                //midiMsg[2]=v;
                                //pinMIDIOut.send( midiMsg, size, blockPosition() );

                                    if(uni==0)
                                    {
                                    midiMsg[0]=s+c;
                                    midiMsg[1]=n;
                                    midiMsg[2]=v;
                                    pinMIDIOut.send( midiMsg, size, blockPosition() );
                                    }
                                    else
                                    {
                                        for(int ch=0;ch<d_voice;ch++)// 2 or 3 voices per note
                                        {
                                        midiMsg[0]=s+ch;
                                        midiMsg[1]=n;
                                        midiMsg[2]=v;
                                        pinMIDIOut.send( midiMsg, size, blockPosition() );
                                        }
                                    }

                                on[i]=0;
                                note[i]=-1;
                                goto exit;
                                }
                            }

                        }


                    }


/*
                    //poly after
                    if(s==160)
                    {

                        if(n>=Lo2 && n<=Hi2)
                        {
                        midiMsg2[0]=s+c;
                        midiMsg2[1]=n;
                        midiMsg2[2]=v;
                        pinMIDIOut2.send( midiMsg2, size, blockPosition() );
                        }

                        if(n>=Lo1 && n<=Hi1)
                        {

                            if(uni==0)
                            {
                            midiMsg[0]=s+c;
                            midiMsg[1]=n;
                            midiMsg[2]=v;
                            pinMIDIOut.send( midiMsg, size, blockPosition() );
                            }
                            else
                            {
                                for(int ch=0;ch<d_voice;ch++)
                                {
                                midiMsg[0]=s+ch;
                                midiMsg[1]=n;
                                midiMsg[2]=v;
                                pinMIDIOut.send( midiMsg, size, blockPosition() );
                                }
                            }

                        }
                    }
*/
            //***********************************************************************************************************************************************************/
                    // note on
                    //if(v !=0 && s>143 && s<160)
                    if(v !=0 && s==144)
                    {

                        if(n>=Lo2 && n<=Hi2)
                        {
                        midiMsg2[0]=s+c;
                        midiMsg2[1]=n;
                        midiMsg2[2]=v;
                        pinMIDIOut2.send( midiMsg2, size, blockPosition() );
                       //end test midiMsg2 note
                        }

                        if(n>=Lo1 && n<=Hi1)
                        {

                            //ctrl_ch=false;
                            // somme calcul
                            for(int i=0;i<voice_ammount;i++)somme=somme+on[i];

                            //if sommme =max  of the polyphony >del the first or the last note played
                            if(somme>=voice_ammount && fflag==2 )
                            {
                                for(int i=0;i<voice_ammount;i++)
                                {
                                    if(first[i]>=voice_ammount)
                                    {
                                    //midiMsg[0]=s+c;;
                                    //midiMsg[1]=note[i];
                                    //midiMsg[2]=0;
                                    //pinMIDIOut.send( midiMsg, size, blockPosition() );                       //end of the note on ifinMIDIOut.send( midiMsg, size, blockPosition() );

                                        if(uni==0)
                                        {
                                        midiMsg[0]=s+c;
                                        midiMsg[1]=note[i];
                                        midiMsg[2]=0;
                                        pinMIDIOut.send( midiMsg, size, blockPosition() );
                                        }
                                        else
                                        {
                                            for(int ch=0;ch<d_voice;ch++)// 2 or 3 voices per note
                                            {
                                            midiMsg[0]=s+ch;
                                            midiMsg[1]=note[i];
                                            midiMsg[2]=0;
                                            pinMIDIOut.send( midiMsg, size, blockPosition() );
                                            }
                                        }

                                    on[i]=0;
                                    note[i]=-1;
                                    }
                                }

                            }
                            if(somme>=voice_ammount && fflag==1 )
                            {
                                for(int i=0;i<voice_ammount;i++)
                                {
                                    if(first[i]==1)
                                    {
                                    //midiMsg[0]=s+c;
                                    //midiMsg[1]=note[i];
                                    //midiMsg[2]=0;
                                    //pinMIDIOut.send( midiMsg, size, blockPosition() );

                                        if(uni==0)
                                        {
                                        midiMsg[0]=s+c;
                                        midiMsg[1]=note[i];
                                        midiMsg[2]=0;
                                        pinMIDIOut.send( midiMsg, size, blockPosition() );
                                        }
                                        else
                                        {
                                            for(int ch=0;ch<d_voice;ch++)// 2 or 3 voices per note
                                            {
                                            midiMsg[0]=s+ch;
                                            midiMsg[1]=note[i];
                                            midiMsg[2]=0;
                                            pinMIDIOut.send( midiMsg, size, blockPosition() );
                                            }
                                        }

                                    on[i]=0;
                                    note[i]=-1;
                                    }
                                }

                            }


                            //check the first note off
                            if(cycle==0)last=0;
                            if(last>voice_ammount-1)last=0;
                            for(int i=0;i<voice_ammount;i++)
                            {
                                if(on[i]==0)
                                {
                                inc=chn[i];
                                goto exit1;
                                }
                            }
                            exit1:
                            for(int i=last;i<voice_ammount;i++)
                            {
                                if(on[i]==0  && chn[i]>=last)
                                {
                                inc=chn[i];
                                goto exit2;
                                }
                            }
                            exit2:


                            for(int i=0;i<voice_ammount;i++)
                            {
                                if(chn[i]==inc && on[i]==0)
                                {
                                //send note on
                                //midiMsg[0]=s+c;
                                //midiMsg[1]=n;
                                //midiMsg[2]=v;
                                //pinMIDIOut.send( midiMsg, size, blockPosition() );

                                    if(uni==0)// NoteId out works with int to list to sample & hold (poly audio streams)
                                    {
                                    midiMsg[0]=s+c;
                                    midiMsg[1]=n;
                                    midiMsg[2]=v;
                                    pinMIDIOut.send( midiMsg, size, blockPosition() );
                                    }
                                    else
                                    {

                                        // 2 or 3 voices per note, as it's 2 or 3 notes played in same time the NoteId doesn't work in unison
                                        // so the detune is done only with Voice-Test-Rnd modules
                                        for(int ch=0;ch<d_voice;ch++)
                                        {
                                        midiMsg[0]=s+ch;
                                        midiMsg[1]=n;
                                        midiMsg[2]=v;
                                        pinMIDIOut.send( midiMsg, size, blockPosition() );
                                        }
                                    }

                                    //set the note on flag
                                    on[i]=1;
                                    note[i]=n;

                                    NoteId=i;// NoteId out works with int to list to sample & hold (poly audio streams)
                                    NoteId.sendPinUpdate();


                                    //inc the first counter
                                    f=somme+1;
                                    if(somme>=voice_ammount)f=voice_ammount;
                                    //set the first flag
                                    first[i]=f;

                                    //dec the first value note if the polyphony is at max
                                    if(somme==voice_ammount && fflag==1 )
                                    for(int i=0;i<voice_ammount;i++)first[i]=first[i]-1;

                                    //inc the last counter
                                    if(cycle==1)
                                    {
                                    last=inc+1;
                                    }
                                //exit of the note on if
                                goto exit;

                                }
                            //end of the note on for
                            }
                       //end test midiMsg1 note
                       }



                    //end of the note on if
                    }

                    exit:
                    somme=0;
                //end uni if
                //}
            //***********************************************************************************************************************************************************/
                    //unisson
                /*if(uni==1)
                {
                //muni=uni;

                        if( s<160)
                        {


                           if(n>=Lo1 && n<=Hi1)
                           {
                            int d_voice=voice_ammount;
                            if(d_voice>3)d_voice=3;
                            for(int i=0;i<d_voice;i++)
                            {
                            midiMsg[0]=s+i;
                            midiMsg[1]=n;
                            midiMsg[2]=v;
                            pinMIDIOut.send( midiMsg, size, blockPosition() );
                            }
                           }


                           if(n>=Lo2 && n<=Hi2)
                           {
                            midiMsg2[0]=s+c;
                            midiMsg2[1]=n;
                            midiMsg2[2]=v;
                            pinMIDIOut2.send( midiMsg2, size, blockPosition() );
                           }



                        }



                }*/

            //***********************************************************************************************************************************************************/

/*
                    //Prg Change
                    if(s==192)
                    {

                            state=true;
                            midiMsg[0]=176+c;
                            midiMsg[1]=123;
                            midiMsg[2]=0;
                            pinMIDIOut.send( midiMsg, size, blockPosition() );

                            midiMsg2[0]=176+c;
                            midiMsg2[1]=123;
                            midiMsg2[2]=0;
                            pinMIDIOut2.send( midiMsg2, size, blockPosition() );

                                for(int i=0;i<6;i++)
                                {
                                on[i]=0;
                                note[i]=-1;
                                chn[i]=i;
                                first[i]=0;
                                }

                            c_uni=0;
                            somme=0;
                            inc=0;
                            last=0;
                            f=0;
                            state=false;

                    }
*/
/*
                    // ctrl
                     if(s>159 && n != ctrl && onfilter==1 && (s<192 || s>207))//pas de ctrl sauf 123, dans les deux cas!
                    {
                            if(n !=123)
                            {
                            }
                            if(n==123 )//&& m_clock !=blockPosition())
                            {
                            state=true;

                            midiMsg[0]=176+c;
                            midiMsg[1]=123;
                            midiMsg[2]=0;
                            pinMIDIOut.send( midiMsg, size, blockPosition() );

                            midiMsg2[0]=176+c;
                            midiMsg2[1]=123;
                            midiMsg2[2]=0;
                            pinMIDIOut2.send( midiMsg2, size, blockPosition() );
                            //pinMIDIcc123.send( midiMsg, size, blockPosition() );

                                for(int i=0;i<6;i++)
                                {
                                on[i]=0;
                                note[i]=-1;
                                chn[i]=i;
                                first[i]=0;
                                }
                            c_uni=0;
                            somme=0;
                            inc=0;
                            last=0;
                            f=0;
                            state=false;
                           }

                    }
*/
               //if(s>159 && onfilter==0 && (s<192 || s>207))
                    if(s==176 && n==123)
                    {

                            state=true;
                            midiMsg[0]=176+c;
                            midiMsg[1]=123;
                            midiMsg2[2]=0;//debug 0

                            pinMIDIOut.send( midiMsg, size, blockPosition() );

                            midiMsg2[0]=176+c;
                            midiMsg2[1]=123;
                            midiMsg2[2]=0;//debug 0
                            pinMIDIOut2.send( midiMsg2, size, blockPosition() );

                                for(int i=0;i<6;i++)
                                {
                                on[i]=0;
                                note[i]=-1;
                                chn[i]=i;
                                first[i]=0;
                                }

                            c_uni=0;
                            somme=0;
                            inc=0;
                            last=0;
                            f=0;
                            state=false;
                    }





            }
    // end system msg
	}
//end void module




}

void KxmodCS::onSetPins(void)
{


	if( uni.isUpdated() && state==false)
	{
    state=true;
	}
	if( nvoice.isUpdated() && state==false)
	{
    state=true;
	}
	if( cycle.isUpdated() && state==false)
	{
    state=true;
	}
	if( fflag.isUpdated() && state==false)
	{
    state=true;
	}
	if( ctrl.isUpdated() && state==false)
	{
    state=true;
	}
	if( onfilter.isUpdated() && state==false)
	{
    state=true;
	}
	/*if( ch1.isUpdated() && state==false)
	{
    state=true;
	}
	if( ch2.isUpdated() && state==false)
	{
    state=true;
	}
	if( ch3.isUpdated() && state==false)
	{
    state=true;
	}
	if( ch4.isUpdated() && state==false)
	{
    state=true;
	}
	if( ch5.isUpdated() && state==false)
	{
    state=true;
	}
	if( ch6.isUpdated() && state==false)
	{
    state=true;
	}*/

	if( power.isUpdated() && state==false)
	{
    if(power==0)state=true;
	}
	if( can.isUpdated() && state==false)
	{
    state=true;
	}
	if( Lo1.isUpdated() && state==false)
	{
    state=true;
	}
	if( Hi1.isUpdated() && state==false)
	{
    state=true;
	}
	if( Lo2.isUpdated() && state==false)
	{
    state=true;
	}
	if( Hi2.isUpdated() && state==false)
	{
    state=true;
	}
	if( monodebug.isUpdated() && state==false)
	{
    state=true;
	}
	/*if( AllNoteOff.isUpdated() && state==false)
	{
    if(AllNoteOff==1)state=true;
	}*/

    if(state==true)reset();


}
void KxmodCS::reset(void)
{


    //unsigned char midiMsg[4];

    if(init==1)
    {
        //for(int i=0;i<16;i++)
        //{
        //midiMsg[0] = 176+i;
        midiMsg[0] = 176;
        midiMsg[1] = 123;
        midiMsg[2] = 0;//debug 0
        //midiMsg2[0] = 176+i;
        midiMsg2[0] = 176;
        midiMsg2[1] = 123;
        midiMsg2[2] = 0;//debug 0
        //pinMIDIcc123.send( midiMsg,3, blockPosition() );
        pinMIDIOut.send( midiMsg,3, blockPosition() );
        pinMIDIOut2.send( midiMsg2,3, blockPosition() );
        //}
    }

    for(int i=0;i<8;i++)
    {
        note[i]=-1;
        chn[i]=i;
        first[i]=0;
        on[i]=0;
    }
    c_uni=0;
    somme=0;
    inc=0;
    last=0;
    f=0;
    state=false;
    init=1;


}
