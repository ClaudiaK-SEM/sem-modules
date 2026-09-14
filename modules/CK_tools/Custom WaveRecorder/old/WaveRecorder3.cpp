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

REGISTER_PLUGIN2 ( WaveRecorder2, L"Wave Recorder3" );


#define WAVE_FORMAT_PCM     1
#define  WAVE_FORMAT_IEEE_FLOAT 0x0003  /*  Microsoft Corporation  */

WaveRecorder2::WaveRecorder2( ) :
outputStream(nullptr)
{
}
//AGain::AGain() : MpBase2( )

int32_t WaveRecorder2::open()
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

	PinIterator it(this);
	{
		int idx = 0;
		for( it.first() ; !it.isDone() ; it.next() )
		{
			if( it.getDirection() == gmpi::MP_IN && it.getDatatype() == 5 )
			{
				AudioIns.push_back(std::unique_ptr<AudioInPin>(new AudioInPin()));
				initializePin(idx, *( AudioIns.back() ));
			}
			++idx;
		}
	}


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

	float* zero = nullptr;
	AudioInPtrs.assign(AudioIns.size(), zero);

	return r;
}
void WaveRecorder2::subProcessTmp(int sampleFrames)
{

	// get pointers to in/output buffers.
	for( size_t i = 0; i < AudioIns.size(); ++i )
	{
		AudioInPtrs[i] = getBuffer(*AudioIns[i]);
	}


	float* output	= getBuffer(pinOut);
	float* buffer = (float*)&( AudioBuffer[0] );


	for( int s = sampleFrames; s > 0; --s )
	{
		for( size_t i = 0; i < AudioInPtrs.size(); ++i )
		{
            float tmp;
            float sample = *AudioInPtrs[i]++;
            if(sample >=diff && msample<diff && init==0)
            {
            init=1;
            offset=0.0f-sample;
            in=pos;
            }

            if(sample >=diff && msample<diff && init==1 && pos>=freq)//splr/20)
            {
            o=1;
            out=pos;
            dur=pos-in;
            init=2;
            }

            if(init==0 || init==2)
            {
            tmp= 0.0f;
            if(pos<=88000*mul)tmp_fb[pos]=tmp;
            }

            if(init==1)
            {
            tmp= sample;
            if(pos<=88000*mul)tmp_fb[pos]=tmp;

            }

			pos++;
			msample=sample;

		}

    *output++ = 0.0f;
	}
}
void WaveRecorder2::subProcess(int sampleFrames)
{
	// get pointers to in/output buffers.
	for( size_t i = 0; i < AudioIns.size(); ++i )
	{
		AudioInPtrs[i] = getBuffer(*AudioIns[i]);
	}


	float* buffer = (float*)&( AudioBuffer[0] );
	//float* output	= getBuffer(pinOut);

	for( int s = sampleFrames; s > 0; --s )
	{
		for( size_t i = 0; i < AudioInPtrs.size(); ++i )
		{
        *buffer++ = tmp_fb[pos+in];
        if(pos<dur)pos++;
        else
        o=2;
		}
    //*output++ = 0.0f;
	}

	const int sampleSizeBytes = sizeof(float) / sizeof(char);
	size_t Channels = AudioInPtrs.size();
	size_t s = sampleFrames * sampleSizeBytes * Channels;
	if( fwrite(&( AudioBuffer[0] ), 1, s, outputStream) != s )
	{
		//error
	}
	//sampleFrameCount += sampleFrames;
	sampleCount += s;

    if( o==2)
    {
    return;
    }


}
void WaveRecorder2::subProcess16bit(int sampleFrames)
{
	// get pointers to in/output buffers.
	for( size_t i = 0; i < AudioIns.size(); ++i )
	{
		AudioInPtrs[i] = getBuffer(*AudioIns[i]);
	}

	short* buffer = (short*)&( AudioBuffer[0] );
	//float* output	= getBuffer(pinOut);
	const float scale_factor = 0x7fff;


	for( int s = sampleFrames; s > 0; --s )
	{
		for( size_t i = 0; i < AudioInPtrs.size(); ++i )
		{
        float sample = scale_factor * (std::min)(1.0f, (std::max)(-1.0f, tmp_fb[pos+in]));
        *buffer++ = (short) FastRealToIntTruncateTowardZero(sample);
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
	sampleCount += s;

  if( o==2)
    {
    return;
    }
}
void WaveRecorder2::subProcess24bit(int sampleFrames)
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
	sampleCount += s;

  if( o==2)
    {
    return;
    }
}

void WaveRecorder2::onSetPins(void)
{

	if( pinMul.isUpdated() )
	{
    time=pinTimeLimit;
    mul=pinMul;
    int limit;
    limit=int(8800000000.0f/getSampleRate());
    if(mul>limit)mul=limit;

    //freq=mul*int(getSampleRate()/time);
	}


	// Check which pins are updated.
    if( pinFileName.isUpdated())
    {
            //c++;
            wstring txt=L".wav";
            //wsprintf(txt,L".%i.wav",c);
            wstring filename = StripExtension(pinFileName) + txt;
           // wstring filename = StripExtension(pinFileName) + L".wav";

			//wstring filename = StripExtension(pinFileName);

			//wchar_t fullFilename[500];
			getHost()->resolveFilename(filename.c_str(), sizeof(fullFilename) / sizeof(fullFilename[0]), fullFilename);
    }

    //if( pinOn.isUpdated() && pinOn==0 &&  outputStream != 0 )
    if( pinOn.isUpdated() && pinOn==1 && dur==0)
    {
    time=pinTimeLimit;
      for( size_t i = 0; i <mul; ++i )
      {
       //freq=mul*int(getSampleRate()/time);;
      freq=mul*int(getSampleRate())/int(time);
      if(freq>=int(time))break;
      }

    //freq=mul*int(getSampleRate()/time);;
    pinSpl=-1;
    pinWrite=0.0f;
    pinDur=dur;
    in=0;
    out=0;
    diff=0.0f;
    offset=0.0f;
    msample=1.0f;
    pos=0;
    init=0;

    //splr=int(getSampleRate());


		int Channels = AudioInPtrs.size();

		if( Channels > 0 )
		{

            SET_PROCESS2(&WaveRecorder2::subProcessTmp);

			size_t s = getBlockSize() * waveHeader.nBlockAlign;
			AudioBuffer.resize(s);
			//sampleFrameCount = 0;
			sampleCount = 0;
			//Set sleep mode (optional).
			setSleep(false);

		}
		else
		{
			SET_PROCESS2(&WaveRecorder2::subProcessNothing);
			setSleep(true);
		}
    }


    if( o==1 && pinOn.isUpdated() && pinOn==0  )
	{

    inbuffer=in;
    durbuffer=dur;
    time=pinTimeLimit;
    freq=mul*int(getSampleRate()/time);
    pos=0;


		int Channels = AudioInPtrs.size();

		if( Channels > 0 )
		{


			//wstring filename = StripExtension(pinFileName) + L".wav";
            //wsprintf(fullFilename,L".%i.wav", c );
			//wstring filename = StripExtension(pinFileName)+ txt;

			//wchar_t fullFilename[500];
			//getHost()->resolveFilename(filename.c_str(), sizeof(fullFilename) / sizeof(fullFilename[0]), fullFilename);



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
            f=pinMul*(getSampleRate()/dur);
            //int fi;
            //fi=int(f);

			if( outputStream==0 || dur==0 )//|| fi != freq)
			{
            pinSpl=0.0f;
            pinWrite=0.0f;
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
				//SET_PROCESS2(&WaveRecorder2::subProcessTmp);
				SET_PROCESS2(&WaveRecorder2::subProcess16bit);
			}
			else
			{
				waveHeader.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
				waveHeader.wBitsPerSample = 32;
				//SET_PROCESS2(&WaveRecorder2::subProcessTmp);
				SET_PROCESS2(&WaveRecorder2::subProcess);
			}

			waveHeader.chnk3_size = 16;
			waveHeader.nChannels = AudioInPtrs.size();
			waveHeader.nSamplesPerSec = (int)getSampleRate();
			waveHeader.nAvgBytesPerSec = waveHeader.nSamplesPerSec * waveHeader.nChannels * (waveHeader.wBitsPerSample / 8);
			waveHeader.nBlockAlign = waveHeader.nChannels * (waveHeader.wBitsPerSample / 8);
            filesize=(LONG)( dur * waveHeader.nChannels * waveHeader.wBitsPerSample / 8);
            waveHeader.chnk4_size = filesize;
           // waveHeader.chnk4_size = (LONG)( dur * waveHeader.nChannels * waveHeader.wBitsPerSample / 8);
            waveHeader.chnk1_size = waveHeader.chnk4_size + 36;

			if( fwrite(&waveHeader, 1, sizeof(waveHeader), outputStream) != sizeof(waveHeader) )
			{

			}




			size_t s = getBlockSize() * waveHeader.nBlockAlign;
			AudioBuffer.resize(s);
			//sampleFrameCount = 0;
			sampleCount = 0;

			// write wave header.
			/*if( pinFormat == 0 )
			{
				SET_PROCESS2(&WaveRecorder2::subProcess16bit);
			}
			else
			{
				SET_PROCESS2(&WaveRecorder2::subProcess);
			}*/



			//Set sleep mode (optional).
			setSleep(false);

		}
		else
		{
			SET_PROCESS2(&WaveRecorder2::subProcessNothing);
			setSleep(true);
		}








	}

    if( o==2 && outputStream !=0)
    {

		//int Channels = AudioInPtrs.size();

		//if( Channels > 0 )
		//{




                pinWrite=1.0f;
                /*std::wstring_convert<std::codecvt_utf8<wchar_t> > stringConverter;
                auto utf8Filename = stringConverter.to_bytes(fullFilename);
                char txt[50];
                float f;
                f=getSampleRate()/dur;
                sprintf(txt,"OK - dur spl=%i samplerate/spl=%f",dur,f);
                MessageBoxA(0, utf8Filename.c_str(), txt, MB_OK);*/



                fclose(outputStream);
                outputStream=nullptr;



                //float* zero = nullptr;
                //AudioInPtrs.assign(AudioIns.size(), zero);

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
                SET_PROCESS2(&WaveRecorder2::subProcessNothing);
                setSleep(true);
                }
                else
                {
                posbuffer=0;
                SET_PROCESS2(&WaveRecorder2::subProcessOut);
                setSleep(false);
                }


			//}
		//}
		//else
		//{
			//SET_PROCESS2(&WaveRecorder2::subProcessNothing);
			//setSleep(true);
		//}
    }
   /* else
    {
    pinSpl=-1;
    pinWrite=0.0f;
    }*/

	if( pinTimeLimit.isUpdated() )
	{
	}
	if( pinFormat.isUpdated() )
	{
	}


}

WaveRecorder2::~WaveRecorder2()
{

	if( outputStream )
	{
    fclose(outputStream);
	}
}
void WaveRecorder2::subProcessOut(int sampleFrames)
{
	// get pointers to in/output buffers.

	float* output	= getBuffer(pinOut);

	// Apply audio processing.
	while( --sampleFrames >= 0 )
	{
     *output++ = tmp_fb[posbuffer+inbuffer];

    posbuffer++;
    if(posbuffer>=durbuffer)posbuffer-=durbuffer;
    //else
	//posbuffer-=durbuffer;
	}
}
