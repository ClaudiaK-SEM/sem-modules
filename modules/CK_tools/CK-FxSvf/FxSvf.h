#ifndef FxSvf_H_INCLUDED
#define FxSvf_H_INCLUDED

#include "mp_sdk_audio.h"

class FxSvf : public MpBase
{
public:
	FxSvf( IMpUnknown* host );
	void subProcess( int bufferOffset, int sampleFrames );
	void subProcess_0( int bufferOffset, int sampleFrames );

	void subProcess_1( int bufferOffset, int sampleFrames );
	void subProcess_2( int bufferOffset, int sampleFrames );
	void subProcess_3( int bufferOffset, int sampleFrames );
	void subProcess_4( int bufferOffset, int sampleFrames );
	void subProcess_5( int bufferOffset, int sampleFrames );
	void subProcess_6( int bufferOffset, int sampleFrames );
	void subProcess_7( int bufferOffset, int sampleFrames );
	void subProcess_8( int bufferOffset, int sampleFrames );

	virtual void onSetPins(void);
	virtual int32_t MP_STDCALL open();


private:

    void ResetVoice();

	AudioInPin pinIn;
	AudioInPin pinCutoff;
	AudioInPin pinRes;
	AudioOutPin pinOut;
	IntInPin res_mode;
	AudioInPin pinVoice;
	AudioInPin pinSVFPoles;
	IntInPin pinOn;
	FloatInPin pinVoiceReset;


	//state_type in_stat;//test open
	//state_type in_stat1;//test pin
    //float m_voice;
	//int static_count;
    //short res_mode;

    //svf
    /*float *fv_tbl;
    float *rv_tbl;
    float *p6_tbl;
    float *apcosA_tbl;
    float *apexpA_tbl;
    float *apexpB_tbl;*/

    float fv_tbl[10002];
    float rv_tbl[10002];
    float p6_tbl[10002];
    float apcosA_tbl[10002];
    float apexpA_tbl[10002];
    float apexpB_tbl[10002];

    //bool run;

    float cut_in;
	float cut;
	float cut0;
	float r0;
	float res0;

    float low;
    float band;
	float high;
	float notch;

    float low2;
    float band2;
	float high2;
	float notch2;

	float f;
	float f2;
    float diff;
    float diff2;

    float kr;
    float o;

    float fp6;
    float fp6b;
    float yp6;
    float yp6b;

    float factor_A;
    float factor_B;

    float delay1;
    float delay2;

    float mulr;
    float mulr2;

    float cvolsv[22];
    float cvolsv2[22];
    float cor_volsv;
};

#endif

