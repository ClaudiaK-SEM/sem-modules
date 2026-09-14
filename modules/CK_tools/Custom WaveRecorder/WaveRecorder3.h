#ifndef WAVERECORDER3_H_INCLUDED
#define WAVERECORDER3_H_INCLUDED

#include "../se_sdk3/mp_sdk_audio.h"
#include <memory>
#include <vector>

#ifndef _MSC_VER
typedef int32_t LONG;
typedef int16_t SHORT;
#endif


struct wave_file_header2
{
	char chnk1_name[4];
	LONG chnk1_size;
	char chnk2_name[4];
	char chnk3_name[4];
	LONG chnk3_size;
	SHORT wFormatTag;
	SHORT nChannels;
	LONG nSamplesPerSec;
	LONG nAvgBytesPerSec;
	SHORT nBlockAlign;
	SHORT wBitsPerSample;
	char chnk4_name[4];
	LONG chnk4_size;
};

class WaveRecorder3 : public MpBase2
{
public:
	WaveRecorder3();
	~WaveRecorder3();
	virtual int32_t MP_STDCALL open() override;
	//void subProcess(int sampleFrames);
	void subProcessTmp(int sampleFrames);
	//void subProcess16bit(int sampleFrames);
	//void subProcess24bit(int sampleFrames);
	void subProcessOut(int sampleFrames);
	virtual void onSetPins(void) override;


private:
	StringInPin pinFileName;
	IntInPin pinFormat;
	FloatInPin pinTimeLimit;
    IntInPin pinOn;
	FloatOutPin pinSpl;
	FloatOutPin pinDur;
	FloatOutPin pinWrite;
    IntInPin pinMul;
    IntInPin pinPreview;
    IntInPin pinMulLoop;
    AudioOutPin pinOut;
    AudioInPin pinIn;

	//std::vector< std::unique_ptr<AudioInPin> > AudioIns;
	//std::vector< float* > AudioInPtrs;
	std::vector< unsigned char > AudioBuffer;
	//std::vector< float > TmpBuffer;

	FILE* outputStream;
	int sampleCount;
	//int sampleFrameCount;

    float tmp_fb[3840032];//20 sec 192k or 80 sec 48k


   LONG filesize;

    float diff;
    float offset;
    float msample;
    int mul;
    int pos;
    int durbuffer;
    int inbuffer;
    int posbuffer;
    int init;
    int splr;
    int in;
    int out;
    int c;
    int o;
    int dur;
    int freq;
    float time;
	wchar_t fullFilename[500];
	float sec;
	float sp;
	int ForceEnd;

    size_t BufferSize;
    //size_t TmpBufferSize;

	wave_file_header2 waveHeader;
};

#endif

