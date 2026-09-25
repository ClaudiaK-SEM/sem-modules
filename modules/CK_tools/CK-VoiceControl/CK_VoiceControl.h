#ifndef CK_VoiceControl_H_INCLUDED
#define CK_VoiceControl_H_INCLUDED

#include "mp_sdk_audio.h"
#include "smart_audio_pin.h"

using namespace gmpi;


class CK_VoiceControl : public MpBase2
{
public:
	CK_VoiceControl( );
	int32_t open() override;
    //void subProcess(int sampleFrames);
	void onSetPins() override;

private:

	float p[12];

    int init;
    int BlobOn;

    float Glide;
    float m_Glide;
    float GlideM;
    float autoG;
    float timeG;

    BlobOutPin BlobToGui;
	IntInPin pinPolyphony;
	IntInPin pinPolyRes;
	IntInPin pinPolyMode;
	IntInPin pinMono;
	IntInPin pinRetrigger;
	IntInPin pinMonoPiority;
	FloatInPin pinGlide;
	IntInPin pinGlideRate;
	IntInPin pinAutoGlide;
	IntInPin pinBRange;
	IntInPin pinVoiceRefresh;
	IntInPin pinMidiToCv;
	IntInPin pinPolyGlide;

};

#endif

