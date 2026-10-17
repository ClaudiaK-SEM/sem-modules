#ifndef DelayTrois_H_INCLUDED
#define DelayTrois_H_INCLUDED

#include "mp_sdk_audio.h"

class DelayTrois : public MpBase
{
public:

    ~DelayTrois();
	DelayTrois( IMpUnknown* host );
	void subProcess( int bufferOffset, int sampleFrames );
	void subProcess_0( int bufferOffset, int sampleFrames );
	virtual void onSetPins(void);
    virtual int32_t MP_STDCALL open();


private:

    float *buffer;
    //float buffer[1920000];

	AudioInPin pinInput;
	AudioInPin pinFeedback;
	AudioInPin pinDelayms;
	AudioInPin pinMod;
	AudioInPin pinFXGain;
	AudioInPin pinPan;
	AudioInPin pinResetMemory;
	AudioInPin pinStOut;
	AudioOutPin pinFXOutL;
	AudioOutPin pinFXOutR;
	AudioOutPin pinFXMono;
	IntInPin on;
	IntInPin sinc;


    //if shared lookup used
    //float *sin_tbl;
    //float *cos_tbl;

    float sin_tbl[10008];
    float cos_tbl[10008];

    float m_fx_gain;
    float limitfeed;
    float m_feed;
    float out_delay;
    float m_d_time;
    float m_mod;
    float m_pan;
    float mod2;
    int count;
	double d_count;

	int buffer_size;
	double fbuffer_size;
	double delay_size;
    float mul;

	//float cx;
    float l_pan;
    float r_pan;

    bool run_on;

    float m_id;
    float hermite(float,float,float,float,double);


    int at_min;
    int time_a;
    float inc_at;
    float at;

    //void CreateBuffer();
    //void ClearBuffer();
    //std::vector<float> buffer;


};

#endif

