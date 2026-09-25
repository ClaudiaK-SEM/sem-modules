#ifndef AllFilters_H_INCLUDED
#define AllFilters_H_INCLUDED

#include "mp_sdk_audio.h"
#define NCoef 8

class AllFilters : public MpBase
{
public:

	AllFilters( IMpUnknown* host );
	void sub_process( int bufferOffset, int sampleFrames );
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
	void sub_process_16( int bufferOffset, int sampleFrames );
	void sub_process_17( int bufferOffset, int sampleFrames );
	void sub_process_18( int bufferOffset, int sampleFrames );
	void sub_process_19( int bufferOffset, int sampleFrames );
	void sub_process_20( int bufferOffset, int sampleFrames );
	void sub_process_21( int bufferOffset, int sampleFrames );
	void sub_process_22( int bufferOffset, int sampleFrames );
	void sub_process_23( int bufferOffset, int sampleFrames );
	void sub_process_24( int bufferOffset, int sampleFrames );
	void sub_process_25( int bufferOffset, int sampleFrames );
	void sub_process_26( int bufferOffset, int sampleFrames );
	void sub_process_27( int bufferOffset, int sampleFrames );
	void sub_process_28( int bufferOffset, int sampleFrames );
	void sub_process_29( int bufferOffset, int sampleFrames );
	void sub_process_30( int bufferOffset, int sampleFrames );
	void sub_process_31( int bufferOffset, int sampleFrames );
	void sub_process_32( int bufferOffset, int sampleFrames );
	void sub_process_33( int bufferOffset, int sampleFrames );
	void sub_process_34( int bufferOffset, int sampleFrames );
	void sub_process_35( int bufferOffset, int sampleFrames );
	void sub_process_0( int bufferOffset, int sampleFrames );

    void subProcess_synthiq(int bufferOffset, int sampleFrames );
    void subProcess_30q(int bufferOffset, int sampleFrames );
    void subProcess_24q(int bufferOffset, int sampleFrames );
    void subProcess_18q(int bufferOffset, int sampleFrames );
	void subProcess_12q(int bufferOffset, int sampleFrames );

	virtual void onSetPins(void);
	virtual int32_t MP_STDCALL open();
	//virtual void SetupLookupTable();

private:

#if defined(SE_1_1)
    bool CreateSharedLookup(wchar_t* p_name, void** p_returnPointer, float p_sampleRate, int p_size);
	void SetupLookupTable();
#endif

    void ResetVoice();

	AudioInPin pinIn;
	AudioInPin pinCutoff;
	AudioInPin pinRes;
	AudioOutPin pinOut;
	IntInPin res_mode;
	AudioInPin pinVoice;
	AudioInPin pinSVFPoles;
	FloatInPin pinVoiceReset;

    //float m_voice;

    bool run;

    float *oflo04_tbl;//
    float *ofk04_tbl;//
    //float *k0_tbl;//
    //float *r0_tbl;//



    //kx
    float *fk0_tbl;
    float *fs0_tbl;
    float *floo1_tbl;
    float *hp_k0_tbl;

    //kx over
    float *ofk0_tbl;
    float *oflo0_tbl;
    float *oflo1_tbl;
    //float *ofse0_tbl;

    //ladder
    float *k2vg_tbl;
    float *kacr_tbl;
    //ladder over
    float *k2vgo_tbl;
    float *kacro_tbl;
    //ladder over4
    float *k2vgo4_tbl;
    float *kacro4_tbl;



    //lp18
    float *k_tbl;
    float *r_tbl;

    //lp18 over
    float *k_tblo;
    float *r_tblo;

    //svf
    float *fv_tbl;
    float *rv_tbl;

    //tanh
    float *tanh_tbl;
    //float *tanh2_tbl;


/*
    float oflo04_tbl[10552];
    float ofk04_tbl[10552];
    //float *k0_tbl;
    //float *r0_tbl;

    //kx
    float fk0_tbl[10552];
    float fs0_tbl[10552];
    float floo1_tbl[10552];
    float hp_k0_tbl[10552];

    //kx over
    float ofk0_tbl[10552];
    float oflo0_tbl[10552];
    float oflo1_tbl[10552];
    //float *ofse0_tbl;

    //ladder
    float k2vg_tbl[10502];
    float kacr_tbl[10502];
    //ladder over
    float k2vgo_tbl[10502];
    float kacro_tbl[10502];
    //ladder over4
    float k2vgo4_tbl[10502];
    float kacro4_tbl[10502];



    //lp18
    float k_tbl[10402];
    float r_tbl[10402];

    //lp18 over
    float k_tblo[10402];
    float r_tblo[10402];

    //svf
    float fv_tbl[10002];
    float rv_tbl[10002];

    //tanh
    float tanh_tbl[20002];
    //float *tanh2_tbl;
*/


    float c;

////////////////////////

    float cut_in;
	float cut;
	float cut0;
	float cut1;
	float cut2;
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

    //float ACoef2_6[NCoef+1];
    //float BCoef2_6[NCoef+1];

    float yA2[NCoef+1]; //output samples
    float xA2[NCoef+1]; //input samples
    float yD2[NCoef+1]; //output samples
    float xD2[NCoef+1]; //input samples

   float iirU_2(float);
   float iirD_2(float);

    //double ACoef4_6[NCoef+1];
    //double BCoef4_6[NCoef+1];

    double yA4[NCoef+1]; //output samples
    double xA4[NCoef+1]; //input samples
    double yD4[NCoef+1]; //output samples
    double xD4[NCoef+1]; //input samples

   float iirU_4(double);
   float iirD_4(float);


////////////////////

   float ftanh(float);
   float ftanh2(float);
   float ftanh4(float);
   float ftanhs(float);
   float ftanhs2(float sample,float dist);
   float ftanhx(float sample,float r, float dist, float color);

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
	//float q;
    float diff;
	//float q2;
    float diff2;

    float kr;
    float o;

    float mulr;
    float mulr2;

    float cr;
    float corr[12];
    float corr2[12];
    float corr3[12];

    float cvol[12];
    float cvolsv[22];
    float cvolsv2[22];
    float cor_vol;


    float corr4[12];
    float corr5[12];

    float cor_volsv;





};

#endif

