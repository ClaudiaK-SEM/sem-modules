// based of SE WaveRecorder2 module (check the SDK3 files)


#include <codecvt>
#include <locale>
#include "./WaveRecorder3.h"
#include "../shared/string_utilities.h"
#include "../shared/xp_simd.h"
#include "../se_sdk3/PinIterator.h"
#include "../shared/xplatform.h"
#include <string>

//#include "windows.h"
//#include "assert.h"


using namespace std;
using namespace gmpi;

REGISTER_PLUGIN2 ( WaveRecorder3, L"Wave Recorder3" );

#define splmax 3840000 // temporary buffer
#define WAVE_FORMAT_PCM     1
#define  WAVE_FORMAT_IEEE_FLOAT 0x0003  /*  Microsoft Corporation  */

WaveRecorder3::WaveRecorder3( ) :
outputStream(nullptr)
{
}

int32_t WaveRecorder3::open()
{

	auto r = MpPluginBase::open();

	// Register pins.
	initializePin(  pinFileName );
	initializePin(  pinFormat );
	initializePin(  pinTimeLimit );
	initializePin(  pinOn );
	initializePin(  pinSpl );
	initializePin(  pinDur );
	initializePin(  pinWrite );
	initializePin(  pinMul );
	initializePin(  pinPreview );
	initializePin(  pinOut );
	initializePin(  pinMulLoop );
	initializePin(  pinIn );


    in=0;
    out=0;
    mul=1;
    diff=0.0f;
    offset=0.0f;
    msample=1.0f;
    pos=0;
    init=0;
    splr=int(getSampleRate());
    dur=0;
    o=0;
    c=0;


	return r;
}
void WaveRecorder3::subProcessTmp(int sampleFrames)//to write temporary buffer up to 3840000 20 sec max at 192k, o=1
{


	float* output	= getBuffer(pinOut);
	float* input	= getBuffer(pinIn);

    //float* tmp_fb = (float*)&( TmpBuffer[0] );

	for( int s = sampleFrames; s > 0; --s )
	{

            float tmp;
            float sample = *input++;
            if(sample >=diff && msample<diff && init==0)//zero crossing IN loop
            {
            init=1;
            offset=0.0f-sample;
            in=pos;
            }

            if(sample >=diff && msample<diff && init==1 && pos>=freq)//zero crossing END loop, o=1
            {
            o=1;
            out=pos;
            dur=pos-in;
            init=2;
            }



            if(init==0)
            {
            tmp= 0.0f;
            }

            if(init==1)
            {
            tmp= sample;
            }


            //if(pos<=TmpBufferSize && init<2)//written while <= 3840000
            if(pos<=splmax && init<2)//written while <= 3840000
            {
            tmp_fb[pos]=tmp;
            //*tmp_fb=tmp;
			pos++;
            }

            if(pos>freq+ForceEnd)
            {
            o=1;
            out=pos;
            dur=pos-in;
            init=2;
            }

			msample=sample;// n-1 zero crossing



    *output++ = 0.0f;
	}

	pinWrite=(freq+ForceEnd-pos)/getSampleRate(); // write buffer led off
    //pinWrite=sampleFrames;

    if( o==1)
    {
    pinWrite=0.0f; // write buffer led off
    SET_PROCESS2(&WaveRecorder3::subProcessNothing);
    return;
    }

}
/*void WaveRecorder3::subProcess(int sampleFrames)// copy the temporary buffer, o=2, 32 bit wav file
{
	// get pointers to in/output buffers.
	float* output	= getBuffer(pinOut);
	float* input	= getBuffer(pinIn);

    float* wavbuff = (float*)&( AudioBuffer[0] );

	for( int s = sampleFrames; s > 0; --s )
	{
        *wavbuff++ = tmp_fb[pos+in];
        if(pos<dur)pos++;
        else
        o=2;
        *input++;
        *output++ = 0.0f;
	}


    pinWrite=pos/getSampleRate(); // write buffer led off


	const int sampleSizeBytes = sizeof(float) / sizeof(char);
	size_t Channels = 1;
	size_t s = sampleFrames * sampleSizeBytes * Channels;

    if( fwrite(&( AudioBuffer[0] ), 1, s, outputStream) != s )
	{
		//error
	}

    if( o==2)
    {
    SET_PROCESS2(&WaveRecorder3::subProcessNothing);
    return;
    }
}
void WaveRecorder3::subProcess16bit(int sampleFrames)// copy the temporary buffer, o=2, 16 bit wav file
{
	// get pointers to in/output buffers.
	float* output	= getBuffer(pinOut);
	float* input	= getBuffer(pinIn);


    short* wavbuff = (short*)&( AudioBuffer[0] );
	const float scale_factor = 0x7fff;


	for( int s = sampleFrames; s > 0; --s )
	{
        float sample = scale_factor * (std::min)(1.0f, (std::max)(-1.0f, tmp_fb[pos+in]));
        *wavbuff++ = (short) FastRealToIntTruncateTowardZero(sample);
        if(pos<dur)pos++;
        else
        o=2;
        *input++;
        *output++ = 0.0f;
	}


    pinWrite=pos/getSampleRate(); // write buffer led off

	const int sampleSizeBytes = sizeof(short) / sizeof(char);
	int Channels = 1;
	size_t s = sampleFrames * sampleSizeBytes * Channels;
	if( fwrite(&( AudioBuffer[0] ), 1, s, outputStream) != s )
	{
		//error
	}

    if( o==2)
    {
    SET_PROCESS2(&WaveRecorder3::subProcessNothing);
    return;
    }
}*/
/*void WaveRecorder3::subProcess24bit(int sampleFrames)
{
	// get pointers to in/output buffers.
	for( size_t i = 0; i < AudioIns.size(); ++i )
	{
		AudioInPtrs[i] = getBuffer(*AudioIns[i]);
	}

	short* buffer = (short*)&( AudioBuffer[0] );
	//float* output	= getBuffer(pinOut);
	//const float scale_factor = 0x7fff;
    //const float scale_factor = 0x7fffff;

	for( int s = sampleFrames; s > 0; --s )
	{
		for( size_t i = 0; i < AudioInPtrs.size(); ++i )
		{

        float sample = tmp_fb[pos+in];
        *buffer++ = sample * 0xFFFFFF / 0xFFFFFFFF;
        if(pos<dur)pos++;
        else
        o=2;
		}
    //*output++ = 0.0f;
	}

	const int sampleSizeBytes = sizeof(short) / sizeof(char);
	int Channels = AudioInPtrs.size();
	size_t s = sampleFrames * sampleSizeBytes * Channels;
	if( fwrite(&( AudioBuffer[0] ), 1, s, outputStream) != s )
	{
		//error
	}
	//sampleFrameCount += sampleFrames;
	//sampleCount += s;

    if( o==2)
    {
    SET_PROCESS2(&WaveRecorder3::subProcessNothing);
    return;
    }
}*/

void WaveRecorder3::onSetPins(void)
{

	if( pinMul.isUpdated() )//int
	{
    mul=pinMul;//ex 220
	}

	// Check which pins are updated.
    if( pinFileName.isUpdated())
    {
            //c++;
            wstring txt=L".wav";
            wstring filename = StripExtension(pinFileName) + txt;
			getHost()->resolveFilename(filename.c_str(), sizeof(fullFilename) / sizeof(fullFilename[0]), fullFilename);
    }

    if( pinOn.isUpdated() && pinOn==1 && dur==0)//to fill the temporary buffer with the audio buffer
    {

    time=pinTimeLimit;//hz value

    sp=getSampleRate()/time;// fs/hz = number of spl, float value

        for( size_t i = 1; i <mul+1; ++i )//increment cycles number
        {
          float frq=i*(sp);// i*number of spl (cycle), float
          freq=int(frq);// i*number of spl (cycle), int
          float ff=frq-freq;//0 or not

          if(ff==0.0f)break;//to detect integer number of spl
          else
          {
          freq=i*int(sp);//force integer
          }
        }


    ForceEnd=freq;
    freq*=pinMulLoop;// to increase the duration file  (1 up to 40)

    sec=freq/getSampleRate();// file duration
    o=0;
    pinSpl=-1;
    pinDur=dur;
    in=0;
    out=0;
    diff=0.0f;
    offset=0.0f;
    msample=1.0f;
    pos=0;
    init=0;
    pinWrite=(freq+ForceEnd-pos)/getSampleRate();// file duration to pin write (led and value)


    //TmpBufferSize=freq+ForceEnd+32;
    //TmpBuffer.resize(TmpBufferSize);

    SET_PROCESS2(&WaveRecorder3::subProcessTmp);

    setSleep(false);
    }


    if( o==1 && pinOn.isUpdated() && pinOn==0  )//if temporary buffer is filled (o=1) and sampling button is off, the wav file is written
	{

    inbuffer=in;
    durbuffer=dur;
    time=pinTimeLimit;
    pos=0;


			if( outputStream !=0 )
			{
            //std::wstring_convert<std::codecvt_utf8<wchar_t> > stringConverter;
            //auto utf8Filename = stringConverter.to_bytes(fullFilename);
            //MessageBoxA(0, utf8Filename.c_str(), "file closed before to write !", MB_OK);
            fclose(outputStream);
			}
#ifdef _WIN32
			outputStream = _wfopen(fullFilename, L"wb");
#else
            std::wstring_convert<std::codecvt_utf8<wchar_t> > stringConverter;
            auto utf8Filename = stringConverter.to_bytes(fullFilename);
            outputStream = fopen(utf8Filename.c_str(), "wb");
#endif

            float f;
            f=dur/(getSampleRate()/time);

			if( outputStream==0 || dur==0 )//|| fi != freq)
			{
            pinSpl=-1;
            pinDur=dur;
            //std::wstring_convert<std::codecvt_utf8<wchar_t> > stringConverter;
            //auto utf8Filename = stringConverter.to_bytes(fullFilename);
            //MessageBoxA(0, utf8Filename.c_str(), " failed to write !", MB_OK);
            //fclose(outputStream);
            return;
			}
			else
            {
            pinSpl=f;
            pinDur=dur;
            }

			//fwprintf(outputStream, L"SynthEdit MIDI Event log\n");
			//fwprintf(outputStream, L"Samplerate:%f\n", getSampleRate());
			//fwprintf(outputStream, L"Block Size:%d\n", getBlockSize());

			// Write partial wave file header.
			memset(&waveHeader, 0, sizeof(waveHeader));

			// write wave header.
			memcpy(waveHeader.chnk1_name, "RIFF", 4);
			memcpy(waveHeader.chnk2_name, "WAVE", 4);
			memcpy(waveHeader.chnk3_name, "fmt ", 4);
			memcpy(waveHeader.chnk4_name, "data", 4);

			// write wave header.
			if( pinFormat == 0 )
			{
				waveHeader.wFormatTag = WAVE_FORMAT_PCM;
				waveHeader.wBitsPerSample = 16;
				//SET_PROCESS2(&WaveRecorder3::subProcess16bit);
			}
			else
			{
				waveHeader.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
				waveHeader.wBitsPerSample = 32;
				//SET_PROCESS2(&WaveRecorder3::subProcess);
			}

			waveHeader.chnk3_size = 16;
			waveHeader.nChannels = 1;
			waveHeader.nSamplesPerSec = (int)getSampleRate();
			waveHeader.nAvgBytesPerSec = waveHeader.nSamplesPerSec * waveHeader.nChannels * (waveHeader.wBitsPerSample / 8);
			waveHeader.nBlockAlign = waveHeader.nChannels * (waveHeader.wBitsPerSample / 8);
            filesize=(LONG)( dur * waveHeader.nChannels * waveHeader.wBitsPerSample / 8); //file size=duration value
            waveHeader.chnk4_size = filesize;
            waveHeader.chnk1_size = waveHeader.chnk4_size + 36;

			if( fwrite(&waveHeader, 1, sizeof(waveHeader), outputStream) != sizeof(waveHeader) )
			{

			}

			//BufferSize = getBlockSize() * waveHeader.nBlockAlign;
			BufferSize = (dur+32) * waveHeader.nBlockAlign;
			AudioBuffer.resize(BufferSize);

			if( pinFormat == 0 )
			{

                short* wavbuff = (short*)&( AudioBuffer[0] );
                const float scale_factor = 0x7fff;


                for( int s = dur+10; s > 0; --s )
                {
                    float sample = scale_factor * (std::min)(1.0f, (std::max)(-1.0f, tmp_fb[pos+in]));
                    *wavbuff++ = (short) FastRealToIntTruncateTowardZero(sample);
                    if(pos<dur)pos++;
                    else
                    o=2;
                }


                pinWrite=pos/getSampleRate(); // write buffer led off

                const int sampleSizeBytes = sizeof(short) / sizeof(char);
                int Channels = 1;
                size_t s = dur * sampleSizeBytes * Channels;
                if( fwrite(&( AudioBuffer[0] ), 1, s, outputStream) != s )
                {

                }


			}
			else
			{

                float* wavbuff = (float*)&( AudioBuffer[0] );
                const float scale_factor = 0x7fff;

                for( int s = dur+10; s > 0; --s )
                {
                    *wavbuff++ = tmp_fb[pos+in];
                    if(pos<dur)pos++;
                    else
                    o=2;
                }


                pinWrite=pos/getSampleRate(); // write buffer led off

                const int sampleSizeBytes = sizeof(float) / sizeof(char);
                int Channels = 1;
                size_t s = dur * sampleSizeBytes * Channels;
                if( fwrite(&( AudioBuffer[0] ), 1, s, outputStream) != s )
                {

                }

			}

           //Set sleep mode (optional).
			setSleep(false);
	}


    if( o==2 && outputStream !=0)//o=2 when the temporary buffer is copied to the wav buffer (when module pins are checked)
    {


                fclose(outputStream);
                outputStream=nullptr;
                pinWrite=dur/getSampleRate();

                o=0;
                dur=0;
                in=0;
                out=0;
                diff=0.0f;
                offset=0.0f;
                msample=1.0f;
                pos=0;
                init=0;

                if(pinPreview==0)
                {
                SET_PROCESS2(&WaveRecorder3::subProcessNothing);
                setSleep(true);
                }
                else
                {
                posbuffer=0;
                SET_PROCESS2(&WaveRecorder3::subProcessOut);
                setSleep(false);
                }

    }


	if( pinTimeLimit.isUpdated() )
	{
	}
	if( pinFormat.isUpdated() )
	{
	}

}

WaveRecorder3::~WaveRecorder3()
{
	if( outputStream )fclose(outputStream);// to close the file stream
}

void WaveRecorder3::subProcessOut(int sampleFrames)// preview process to read the temporary buffer and verify the loop
{

	float* output	= getBuffer(pinOut);
	float* input	= getBuffer(pinIn);

	while( --sampleFrames >= 0 )
	{

     *output++ = tmp_fb[posbuffer+inbuffer];
    posbuffer++;
    if(posbuffer>=durbuffer)posbuffer-=durbuffer;
	*input++;
	}
}
