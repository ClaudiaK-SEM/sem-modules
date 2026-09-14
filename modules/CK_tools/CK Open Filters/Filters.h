#ifndef Filters_H_INCLUDED
#define Filters_H_INCLUDED

#include "mp_sdk_audio.h"
#define NCoef 8

class CK_Filters : public MpBase
{
public:

	CK_Filters( IMpUnknown* host );
	void sub_process( int bufferOffset, int sampleFrames );
	void sub_process_0( int bufferOffset, int sampleFrames );
	void sub_process_1( int bufferOffset, int sampleFrames );
	void sub_process_2( int bufferOffset, int sampleFrames );
	void sub_process_3( int bufferOffset, int sampleFrames );
	void sub_process_4( int bufferOffset, int sampleFrames );
	void sub_process_5( int bufferOffset, int sampleFrames );
	void sub_process_6( int bufferOffset, int sampleFrames );
	void sub_process_7( int bufferOffset, int sampleFrames );
	void sub_process_8( int bufferOffset, int sampleFrames );
	void sub_process_9( int bufferOffset, int sampleFrames );
	void sub_process_10( int bufferOffset, int sampleFrames );
	void sub_process_11( int bufferOffset, int sampleFrames );
	void sub_process_12( int bufferOffset, int sampleFrames );
	void sub_process_13( int bufferOffset, int sampleFrames );
	void sub_process_14( int bufferOffset, int sampleFrames );
	void sub_process_15( int bufferOffset, int sampleFrames );


	virtual void onSetPins(void);
	virtual int32_t MP_STDCALL open();


private:

    void ResetVoice();

	AudioInPin pinIn;
	AudioInPin pinCutoff;
	AudioInPin pinRes;
	AudioOutPin pinOut;
	IntInPin res_mode;
	AudioInPin pinSVFPoles;
    BoolInPin pinOnOff;
	FloatInPin pinVoiceReset;

    //float m_voice;

    bool run;

    //kx
    float *fk0_tbl;
    float *fs0_tbl;
    float *hp_k0_tbl;

    //kx over
    float *ofk0_tbl;
    float *oflo0_tbl;

    //ladder
    float *k2vg_tbl;
    float *kacr_tbl;
    //ladder over
    float *k2vgo_tbl;
    float *kacro_tbl;


    //svf
    float *fv_tbl;
    float *rv_tbl;

    //tanh
    float *tanh_tbl;

////////////////////////

    float cut_in;
	float cut;
	float cut0;
	float cutL;


	float r0;
	float res;
	float res0;
	float res1;
	float resL;

	float k0;
	float p0;
	float scale0;

/////////////////////////

	float x0;
	float lx0;

	float y0;
	float y1;
	float y2;
	float y3;
	float y4;
	float oldx0;
	float oldy0;
	float oldy1;
	float oldy2;
	float oldy3;

    float ly0;
	float ly1;
	float ly2;
	float ly3;
	float loldx0;
	float loldy0;
	float loldy1;
	float loldy2;

	float hk0;
	float hp0;

	float hx0;
	float hy0;//
	float hy1;//
	float hy2;//
	float hy3;//

	//float hy[4];	///
	float holdx0;
	float holdy0;
	float holdy1;
	float holdy2;

////////////////////

    float up_overA[9];
    float over[9];
    float delay_inA;


    float yA2[NCoef+1]; //output samples
    float xA2[NCoef+1]; //input samples
    float yD2[NCoef+1]; //output samples
    float xD2[NCoef+1]; //input samples

   float iirU_2(float);
   float iirD_2(float);


////////////////////

   float ftanh(float);
   float ftanh2(float);
   float ftanh4(float);
   float ftanhs2(float sample,float dist);


////////////////////


    float k2vg;
    float kacr;

    float az1;
    float az2;
    float az3;
    float az4;
    float az5;
    float ay1;
    float ay2;
    float ay3;
    float ay4;
    float amf;

////////////////////


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

    float mulr;
    float mulr2;


    float cvolsv[22];
    float cvolsv2[22];





};

#endif

