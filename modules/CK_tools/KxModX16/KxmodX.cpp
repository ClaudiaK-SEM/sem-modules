#include "KxmodX.h"


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








REGISTER_PLUGIN ( KxmodX, L"KX-MOD X16" );

KxmodX::KxmodX( IMpUnknown* host ) : MpBase( host )
{
	// Register pins.
	initializePin( 0, pinMIDIIn );
	initializePin( 1, pinMIDIOut1 );
	initializePin( 2, pinMIDIOut2 );
	initializePin( 3, pinMIDIOut3 );
	initializePin( 4, pinMIDIOut4 );
	initializePin( 5, pinMIDIOut5 );
	initializePin( 6, pinMIDIOut6 );
	initializePin( 7, pinMIDIOut );
	initializePin( 8, uni );
	initializePin( 9, voice_ammount );
	initializePin( 10, cycle );
	initializePin( 11, fflag );
	initializePin( 12, ctrl );
	initializePin( 13, onfilter );
	initializePin( 14, ch1 );
	initializePin( 15, ch2 );
	initializePin( 16, ch3 );
	initializePin( 17, ch4 );
	initializePin( 18, ch5 );
	initializePin( 19, ch6 );
	initializePin( 20, power );
	initializePin( 21, can );

    init=0;
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
void KxmodX::onMidiMessage( int pin, unsigned char* midiMessage, int size )
{


	if(power==1 && size<4)
	{


	int s,c,n,v;// 3 bytes of MIDI message

	s = midiMessage[0] & 0xf0;
    c = midiMessage[0] & 0x0f;
	n = midiMessage[1];
	v = midiMessage[2];

    //***********************************************************************************************************************************************************/



         if(c==can || can==-1)
         {



            //**********************************************************************************************************************************************************************/
                if(uni==0)
                {
                    // vel=0 note off
                    //if(v==0 && s>143 && s<160)
                    if(v==0 && s==144)
                    {

                            for(int i=0;i<voice_ammount;i++)
                            {
                                if(on[i]==1 && note[i]==n )
                                {
                                midiMessage2[0]=s+chn[i];
                                midiMessage2[1]=n;
                                midiMessage2[2]=v;
                                pinMIDIOut.send( midiMessage2, size, blockPosition() );
                                if(chn[i]==ch1)pinMIDIOut1.send( midiMessage2, size, blockPosition() );
                                if(chn[i]==ch2)pinMIDIOut2.send( midiMessage2, size, blockPosition() );
                                if(chn[i]==ch3)pinMIDIOut3.send( midiMessage2, size, blockPosition() );
                                if(chn[i]==ch4)pinMIDIOut4.send( midiMessage2, size, blockPosition() );
                                if(chn[i]==ch5)pinMIDIOut5.send( midiMessage2, size, blockPosition() );
                                if(chn[i]==ch6)pinMIDIOut6.send( midiMessage2, size, blockPosition() );
                                on[i]=0;
                                note[i]=-1;
                                goto exit;
                                }
                            }

                    }

                    //note off
                    //if(s>127 && s<144)
                    if(s==128)
                    {

                            for(int i=0;i<voice_ammount;i++)
                            {
                                if(on[i]==1 && note[i]==n )
                                {
                                midiMessage2[0]=s+chn[i];
                                midiMessage2[1]=n;
                                midiMessage2[2]=v;
                                pinMIDIOut.send( midiMessage2, size, blockPosition() );
                                if(chn[i]==ch1)pinMIDIOut1.send( midiMessage2, size, blockPosition() );
                                if(chn[i]==ch2)pinMIDIOut2.send( midiMessage2, size, blockPosition() );
                                if(chn[i]==ch3)pinMIDIOut3.send( midiMessage2, size, blockPosition() );
                                if(chn[i]==ch4)pinMIDIOut4.send( midiMessage2, size, blockPosition() );
                                if(chn[i]==ch5)pinMIDIOut5.send( midiMessage2, size, blockPosition() );
                                if(chn[i]==ch6)pinMIDIOut6.send( midiMessage2, size, blockPosition() );
                                on[i]=0;
                                note[i]=-1;
                                goto exit;
                                }
                            }

                    }
            //***********************************************************************************************************************************************************/
                    // note on
                    //if(v !=0 && s>143 && s<160)
                    if(v !=0 && s==144)
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
                                    midiMessage2[0]=s+chn[i];
                                    midiMessage2[1]=note[i];
                                    midiMessage2[2]=0;
                                    pinMIDIOut.send( midiMessage2, size, blockPosition() );
                                    if(chn[i]==ch1)pinMIDIOut1.send( midiMessage2, size, blockPosition() );
                                    if(chn[i]==ch2)pinMIDIOut2.send( midiMessage2, size, blockPosition() );
                                    if(chn[i]==ch3)pinMIDIOut3.send( midiMessage2, size, blockPosition() );
                                    if(chn[i]==ch4)pinMIDIOut4.send( midiMessage2, size, blockPosition() );
                                    if(chn[i]==ch5)pinMIDIOut5.send( midiMessage2, size, blockPosition() );
                                    if(chn[i]==ch6)pinMIDIOut6.send( midiMessage2, size, blockPosition() );
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
                                    midiMessage2[0]=s+chn[i];
                                    midiMessage2[1]=note[i];
                                    midiMessage2[2]=0;
                                    pinMIDIOut.send( midiMessage2, size, blockPosition() );
                                    if(chn[i]==ch1)pinMIDIOut1.send( midiMessage2, size, blockPosition() );
                                    if(chn[i]==ch2)pinMIDIOut2.send( midiMessage2, size, blockPosition() );
                                    if(chn[i]==ch3)pinMIDIOut3.send( midiMessage2, size, blockPosition() );
                                    if(chn[i]==ch4)pinMIDIOut4.send( midiMessage2, size, blockPosition() );
                                    if(chn[i]==ch5)pinMIDIOut5.send( midiMessage2, size, blockPosition() );
                                    if(chn[i]==ch6)pinMIDIOut6.send( midiMessage2, size, blockPosition() );
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
                                if(on[i]==0  )
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
                                    midiMessage2[0]=s+chn[i];
                                    midiMessage2[1]=n;
                                    midiMessage2[2]=v;
                                    pinMIDIOut.send( midiMessage2, size, blockPosition() );
                                    if(chn[i]==ch1)pinMIDIOut1.send( midiMessage2, size, blockPosition() );
                                    if(chn[i]==ch2)pinMIDIOut2.send( midiMessage2, size, blockPosition() );
                                    if(chn[i]==ch3)pinMIDIOut3.send( midiMessage2, size, blockPosition() );
                                    if(chn[i]==ch4)pinMIDIOut4.send( midiMessage2, size, blockPosition() );
                                    if(chn[i]==ch5)pinMIDIOut5.send( midiMessage2, size, blockPosition() );
                                    if(chn[i]==ch6)pinMIDIOut6.send( midiMessage2, size, blockPosition() );
                                    //set the note on flag
                                    on[i]=1;
                                    note[i]=n;
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

                    //end of the note on if
                    }
                    exit:
                    somme=0;
                }
            //***********************************************************************************************************************************************************/
                //unisson
                if(uni==1)
                {

                        if( s<160)
                        {

                            for(int i=0;i<voice_ammount;i++)
                            {
                            midiMessage2[0]=s+i;
                            midiMessage2[1]=n;
                            midiMessage2[2]=v;
                            pinMIDIOut.send( midiMessage2, size, blockPosition() );
                            if(i==ch1)pinMIDIOut1.send( midiMessage2, size, blockPosition() );
                            if(i==ch2)pinMIDIOut2.send( midiMessage2, size, blockPosition() );
                            if(i==ch3)pinMIDIOut3.send( midiMessage2, size, blockPosition() );
                            if(i==ch4)pinMIDIOut4.send( midiMessage2, size, blockPosition() );
                            if(i==ch5)pinMIDIOut5.send( midiMessage2, size, blockPosition() );
                            if(i==ch6)pinMIDIOut6.send( midiMessage2, size, blockPosition() );
                            }

                          //if(v !=0 && s>143 && s<160) note on
                           /* if(v !=0 && s==144)
                            {
                            note[c_uni]=n;
                            on[c_uni]=1;
                            c_uni++;
                            if(c_uni>voice_ammount-1)c_uni=0;
                            }*/
                        }

                }

            //***********************************************************************************************************************************************************/
                    // ctrl
                     if(s>159 && n != ctrl && onfilter==1 && (s<192 || s>207))//pas de ctrl sauf 123, dans les deux cas!
                    {
                            if(n !=123)
                            {
                                for(int i=0;i<voice_ammount;i++)
                                {
                                midiMessage2[0]=s+i;
                                midiMessage2[1]=n;
                                midiMessage2[2]=v;
                                pinMIDIOut.send( midiMessage2, size, blockPosition() );
                                }
                            }
                            if(s==176 && n==123 )//&& m_clock !=blockPosition())
                            {
                            state=true;

                                for(int i=0;i<6;i++)
                                {
                                midiMessage2[0]=176+i;
                                midiMessage2[1]=123;
                                midiMessage2[2]=0;//debug 0
                                pinMIDIOut.send( midiMessage2, size, blockPosition() );
                                if(i==ch1)pinMIDIOut1.send( midiMessage2, size, blockPosition() );
                                if(i==ch2)pinMIDIOut2.send( midiMessage2, size, blockPosition() );
                                if(i==ch3)pinMIDIOut3.send( midiMessage2, size, blockPosition() );
                                if(i==ch4)pinMIDIOut4.send( midiMessage2, size, blockPosition() );
                                if(i==ch5)pinMIDIOut5.send( midiMessage2, size, blockPosition() );
                                if(i==ch6)pinMIDIOut6.send( midiMessage2, size, blockPosition() );
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

                    if(s>159 && onfilter==0 && (s<192 || s>207))
                    //if(s>159 && (s<192 || s>207))
                    {
                           if(n !=123)
                           {
                                for(int i=0;i<voice_ammount;i++)
                                {
                                midiMessage2[0]=s+i;
                                midiMessage2[1]=n;
                                midiMessage2[2]=v;
                                pinMIDIOut.send( midiMessage2, size, blockPosition() );
                                }

                           }
                           if(s==176 && n==123)
                           {
                           state=true;

                                for(int i=0;i<6;i++)
                                {
                                midiMessage2[0]=176+i;
                                midiMessage2[1]=123;
                                midiMessage2[2]=0;//debug 0
                                pinMIDIOut.send( midiMessage2, size, blockPosition() );
                                if(i==ch1)pinMIDIOut1.send( midiMessage2, size, blockPosition() );
                                if(i==ch2)pinMIDIOut2.send( midiMessage2, size, blockPosition() );
                                if(i==ch3)pinMIDIOut3.send( midiMessage2, size, blockPosition() );
                                if(i==ch4)pinMIDIOut4.send( midiMessage2, size, blockPosition() );
                                if(i==ch5)pinMIDIOut5.send( midiMessage2, size, blockPosition() );
                                if(i==ch6)pinMIDIOut6.send( midiMessage2, size, blockPosition() );
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


            }
    // end system msg
	}
//end void module




}

void KxmodX::onSetPins(void)
{


	if( uni.isUpdated() && state==false)
	{
    state=true;
	}
	if( voice_ammount.isUpdated() && state==false)
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
	if( ch1.isUpdated() && state==false)
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
	}
	if( power.isUpdated() && state==false)
	{
    if(power==0)state=true;
	}
	if( can.isUpdated() && state==false)
	{
    state=true;
	}


    if(state==true)reset();


}
void KxmodX::reset(void)
{


            //unsigned char midiMsg[4];


            for(int i=0;i<6;i++)
            {
                if(init==1)
                {
                midiMessage2[0] = 176 + i;
                midiMessage2[1] = 123;
                midiMessage2[2] = 0;//debug 0
                pinMIDIOut.send( midiMessage2,3, blockPosition() );
                if(i==ch1)pinMIDIOut1.send( midiMessage2, 3, blockPosition() );
                if(i==ch2)pinMIDIOut2.send( midiMessage2, 3, blockPosition() );
                if(i==ch3)pinMIDIOut3.send( midiMessage2, 3, blockPosition() );
                if(i==ch4)pinMIDIOut4.send( midiMessage2, 3, blockPosition() );
                if(i==ch5)pinMIDIOut5.send( midiMessage2, 3, blockPosition() );
                if(i==ch6)pinMIDIOut6.send( midiMessage2, 3, blockPosition() );
                }

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
        init=1;

}
