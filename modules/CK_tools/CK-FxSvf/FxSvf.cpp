#include "FxSvf.h"
//#include <mutex>
#include "math.h"

#define ONEPI 3.1415926535f
#define TWOPI 6.28318530718f
#define SVL 50.0f
static const float anti_denormal = 1e-18f;
inline void kill_denormal(float &p_sample ) { p_sample += anti_denormal;p_sample -= anti_denormal;};

REGISTER_PLUGIN ( FxSvf, L"KX-SVF-02" );

FxSvf::FxSvf( IMpUnknown* host ) : MpBase( host )
{
	// Register pins.
	initializePin( 0, pinIn );
	initializePin( 1, pinCutoff );
	initializePin( 2, pinRes );
	initializePin( 3, pinOut );
	initializePin( 4, res_mode );
	initializePin( 5, pinVoice );
	initializePin( 6, pinSVFPoles );
    initializePin( 7, pinOn );
    initializePin( 8, pinVoiceReset );
}
int32_t FxSvf::open()
{

    /*static std::mutex safeInit;
    std::lock_guard<std::mutex> lock(safeInit);

    int32_t need_initialise12;
	getHost()->allocateSharedMemory( L"CK77_SV2_volt", (void**) &fv_tbl, getSampleRate(), 10002*sizeof(float), need_initialise12 );

    int32_t need_initialise13;
	getHost()->allocateSharedMemory( L"CK77_SV2_res", (void**) &rv_tbl, getSampleRate(), 10002*sizeof(float), need_initialise13 );


    int32_t need_initialise14;
	getHost()->allocateSharedMemory( L"CK77_SE_P6", (void**) &p6_tbl, getSampleRate(), 10002*sizeof(float), need_initialise14 );


    int32_t need_initialise15;
	getHost()->allocateSharedMemory( L"CK77_SE_AP_COSA", (void**) &apcosA_tbl, getSampleRate(), 10002*sizeof(float), need_initialise15 );

    int32_t need_initialise16;
	getHost()->allocateSharedMemory( L"CK77_SE_AP_EXPA", (void**) &apexpA_tbl, getSampleRate(), 10002*sizeof(float), need_initialise16 );


    int32_t need_initialise17;
	getHost()->allocateSharedMemory( L"CK77_SE_AP_EXPB", (void**) &apexpB_tbl, getSampleRate(), 10002*sizeof(float), need_initialise17 );


    if(need_initialise17)
    {*/
            float sr=getSampleRate();

            for(int i=0;i<10001;i++)
            {
            float volt=float(i*0.001f);
            float f=440.0f*powf(2.0f,(volt-5.0f));
            float maxf=f/sr;
            float f0=ONEPI*maxf;
            fv_tbl[i] =2.0f*sinf(f0);
            float r=float(i*0.0512f);
            rv_tbl[i]=(0.000025f  * r * r + 0.009f * r + 7.8f)*0.1f;//0.1f=512 table_size
            //rv_tbl[i]=(0.0000145f  * r * r + 0.02f * r + 6.0f)*0.1f;//0.1f=512 table_size
            //float r=float(i*0.0001f);
            //rv_tbl[i]=0.2f*(expf(r*2.0f) + 2.5f);
             p6_tbl[i] = expf(-TWOPI * maxf);
            //all
            //factor_A = 2.f * cos( PI2 * maxf ) * exp( - PI * maxf *q_const ) ;
            //factor_B = exp( - PI2 * maxf * q_const );
             apcosA_tbl[i] = 2.f * cosf( TWOPI * maxf );
             apexpA_tbl[i] = expf(-ONEPI*maxf);
             apexpB_tbl[i] = expf(-TWOPI*maxf);
            }
   //}

  //run=false;


/**********************/

    cut0=-100.0f;
    res0=-100.0f;

    kr=0.0f;
    o=0.0f;

    yp6=0.0f;
    yp6b=0.0f;

    delay2 = delay1 = 0.0f;

    low=0.0f;
    band=0.0f;
    high=0.0f;
    notch=0.0f;

    low2=0.0f;
    band2=0.0f;
    high2=0.0f;
    notch2=0.0f;

/******************/

    cvolsv[0]= 1.0f;
    cvolsv[1]= 1.0f;
    cvolsv[2]= 1.0f;//1
    cvolsv[3]= 1.0f;
    cvolsv[4]= 1.0f;
    cvolsv[5]= 1.0f;
    cvolsv[6]= 1.0f;
    cvolsv[7]= 1.0f;
    cvolsv[8]= 1.0f;
    cvolsv[9]= 1.0f;
    cvolsv[10]= 1.0f;//
    cvolsv[11]= 1.0f;
    cvolsv[12]= 1.0f;
    cvolsv[13]= 1.0f;
    cvolsv[14]= 1.0f;
    cvolsv[15]= 1.0f;
    cvolsv[16]= 1.0f;
    cvolsv[17]= 0.75f;
    cvolsv[18]= 0.5f;//9
    cvolsv[19]= 0.25f;//
    cvolsv[20]= 0.125f;
    cvolsv[21]= 0.125f;

    cvolsv2[0]= 1.0f;
    cvolsv2[1]= 1.0f;
    cvolsv2[2]= 1.0f;//1
    cvolsv2[3]= 1.0f;
    cvolsv2[4]= 1.0f;
    cvolsv2[5]= 1.0f;
    cvolsv2[6]= 1.0f;
    cvolsv2[7]= 1.0f;
    cvolsv2[8]= 1.0f;
    cvolsv2[9]= 1.0f;
    cvolsv2[10]= 1.0f;//
    cvolsv2[11]= 1.0f;
    cvolsv2[12]= 1.0f;
    cvolsv2[13]= 1.0f;
    cvolsv2[14]= 1.0f;
    cvolsv2[15]= 1.0f;
    cvolsv2[16]= 1.0f;
    cvolsv2[17]= 1.0f;
    cvolsv2[18]= 1.0f;//9
    cvolsv2[19]= 1.5f;//
    cvolsv2[20]= 2.0f;
    cvolsv2[21]= 2.0f;

	//return gmpi::MP_OK;
	return MpBase::open();
}
/*void FxSvf::subProcess_1( int bufferOffset, int sampleFrames )
{
	// get pointers to in/output buffers.
	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
	float* mix	= bufferOffset + pinSVFPoles.getBuffer();

	for( int s = sampleFrames; s > 0; --s )
	{

        if(*cutoff !=cut0 || *resonance !=res0)
        {
        cut0 = *cutoff;
        res0 = *resonance;

        cut_in=*cutoff;
        if(cut_in>1.0f )cut_in=1.0f;
        if(cut_in<0.0f)cut_in=0.0f;

        if(o<0.0f)o=-o;
        kr=(o-1.0f)*0.25f;
        if(kr>0.25f)kr=0.25f;
        if(kr<0.0f)kr=0.0f;

        float q=(1.0f-kr)*(*resonance*0.98f);
        //float q=*resonance*0.98f;
		diff= 1.f - q;
        float q2=(-q*q)+q;
		diff2= 1.f - q2;

        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        f =fv_tbl[fk0]+(fract_fk0*(fv_tbl[fk0+1]-fv_tbl[fk0]));

        f2=f;

        float float_rk0=q*10000.0f;
        int rk0=int(float_rk0);
        if(f>rv_tbl[rk0] )f=rv_tbl[rk0];
        float_rk0=q2*10000.0f;
        rk0=int(float_rk0);
        if(f2>rv_tbl[rk0] )f2=rv_tbl[rk0];
        }


        low += f * band;
		high = *in - low - band * diff;
		band += high * f;


        low2 += f2 * band2;
		high2 = low  - low2 - band2 * diff2;
		band2 += high2 * f2;

        o=(*mix*(low2-low))+low;//(mix*(b-a))+a;
        *out=o;


		in++;
		cutoff++;
		resonance++;
        out++;
        mix++;
	}
}
void FxSvf::subProcess_2( int bufferOffset, int sampleFrames )
{
	// get pointers to in/output buffers.
	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
	float* mix	= bufferOffset + pinSVFPoles.getBuffer();

	for( int s = sampleFrames; s > 0; --s )
	{

        if(*cutoff !=cut0 || *resonance !=res0)
        {
        cut0 = *cutoff;
        res0 = *resonance;

        cut_in=*cutoff;
        if(cut_in>1.0f )cut_in=1.0f;
        if(cut_in<0.0f)cut_in=0.0f;

        if(o<0.0f)o=-o;
        kr=(o-1.0f)*0.25f;
        if(kr>0.25f)kr=0.25f;
        if(kr<0.0f)kr=0.0f;

        float q=(1.0f-kr)*(*resonance*0.98f);
        //float q=*resonance*0.98f;
		diff= 1.f - q;
        float q2=(-q*q)+q;
		diff2= 1.f - q2;

        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        f =fv_tbl[fk0]+(fract_fk0*(fv_tbl[fk0+1]-fv_tbl[fk0]));

        f2=f;

        float float_rk0=q*10000.0f;
        int rk0=int(float_rk0);
        if(f>rv_tbl[rk0] )f=rv_tbl[rk0];
        float_rk0=q2*10000.0f;
        rk0=int(float_rk0);
        if(f2>rv_tbl[rk0] )f2=rv_tbl[rk0];
        }


        low += f * band;
		high = *in - low - band * diff;
		band += high * f;

        low2 += f2 * band2;
		high2 = high - low2 - band2 * diff2;
		band2 += high2 * f2;


        o=(*mix*(high2-high))+high;//(mix*(b-a))+a;
        *out=o;

		in++;
		cutoff++;
		resonance++;
        out++;
        mix++;
	}
}
void FxSvf::subProcess_3( int bufferOffset, int sampleFrames )
{
	// get pointers to in/output buffers.
	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
	float* mix	= bufferOffset + pinSVFPoles.getBuffer();

	for( int s = sampleFrames; s > 0; --s )
	{


        if(*cutoff !=cut0 || *resonance !=res0)
        {
        cut0 = *cutoff;
        res0 = *resonance;

        cut_in=*cutoff;
        if(cut_in>1.0f )cut_in=1.0f;
        if(cut_in<0.0f)cut_in=0.0f;

        if(o<0.0f)o=-o;
        kr=(o-1.0f)*0.25f;
        if(kr>0.25f)kr=0.25f;
        if(kr<0.0f)kr=0.0f;

        float q=(1.0f-kr)*(*resonance*0.98f);
        //float q=*resonance*0.98f;
		diff= 1.f - q;
        float q2=(-q*q)+q;
		diff2= 1.f - q2;

        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        f =fv_tbl[fk0]+(fract_fk0*(fv_tbl[fk0+1]-fv_tbl[fk0]));

        f2=f;

        float float_rk0=q*10000.0f;
        int rk0=int(float_rk0);
        if(f>rv_tbl[rk0] )f=rv_tbl[rk0];
        float_rk0=q2*10000.0f;
        rk0=int(float_rk0);
        if(f2>rv_tbl[rk0] )f2=rv_tbl[rk0];
        }


        low += f * band;
		high = *in - low - band * diff;
		band += high * f;

        low2 += f2 * band2;
		high2 = band - low2 - band2 * diff2;
		band2 += high2 * f2;


        o=(*mix*(band2-band))+band;//(mix*(b-a))+a;
        *out=o;


		in++;
		cutoff++;
		resonance++;
        out++;
        mix++;
	}
}
void FxSvf::subProcess_4( int bufferOffset, int sampleFrames )
{
	// get pointers to in/output buffers.
	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
	float* mix	= bufferOffset + pinSVFPoles.getBuffer();

	for( int s = sampleFrames; s > 0; --s )
	{

        if(*cutoff !=cut0 || *resonance !=res0)
        {
        cut0 = *cutoff;
        res0 = *resonance;

        float cut_in=*cutoff;
        if(cut_in>1.0f )cut_in=1.0f;
        if(cut_in<0.0f)cut_in=0.0f;

        if(o<0.0f)o=-o;
        kr=(o-1.0f)*0.25f;
        if(kr>0.25f)kr=0.25f;
        if(kr<0.0f)kr=0.0f;

        //float q=(1.0f-kr)*(*resonance*0.98f);
        //float qinv=1.0f-*resonance;
        float q=(1.0f-kr)*(0.820f+(*resonance*0.130f));
		diff2=diff= 1.f - q;
		//diff2= 1.f - q2;



        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        f =fv_tbl[fk0]+(fract_fk0*(fv_tbl[fk0+1]-fv_tbl[fk0]));

        //f2=f;

        float float_rk0=q*10000.0f;
        int rk0=int(float_rk0);
        if(f>rv_tbl[rk0] )f=rv_tbl[rk0];
        //float_rk0=q2*10000.0f;
        //rk0=int(float_rk0);
        //if(f2>rv_tbl[rk0] )f2=rv_tbl[rk0];
        f2=f;
        }


        low += f * band;
		high = *in - low - band * diff;
		band += high * f;

        float n= high + low;

        low2 += f2 * band2;
		high2 = n - low2 - band2 * diff2;
		band2 += high2 * f2;

        float n2= high2 + low2;

        o=(*mix*(n2-n))+n;//(mix*(b-a))+a;
        *out=o;

		in++;
		cutoff++;
		resonance++;
        out++;
        mix++;
	}
}*/
void FxSvf::subProcess( int bufferOffset, int sampleFrames )
{
	// get pointers to in/output buffers.
	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
	float* mix	= bufferOffset + pinSVFPoles.getBuffer();

	for( int s = sampleFrames; s > 0; --s )
	{
		// TODO: Signal processing goes here.

		// Increment buffer pointers.
        *out =*in;

		in++;
		cutoff++;
		resonance++;
        out++;
        mix++;
	}
}
void FxSvf::subProcess_0( int bufferOffset, int sampleFrames )
{
	float* out	= bufferOffset + pinOut.getBuffer();
	for( int s = sampleFrames; s > 0; --s )
	{

        *out =0.0f;
        out++;

	}
}
void FxSvf::subProcess_1( int bufferOffset, int sampleFrames )
{
//SVLPR

	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
	float* mix	= bufferOffset + pinSVFPoles.getBuffer();

	for( int s = sampleFrames; s > 0; --s )
	{

        if(*cutoff !=cut0 || *resonance !=res0)
        {
        cut0 = *cutoff;

        if(*resonance !=res0 )
        {
        res0=*resonance;
        float cv=res0*20.0f;
        int iv=int(cv);
        float fcv=cv-float(iv);
        mulr=cvolsv[iv]+(fcv*(cvolsv[iv+1]-cvolsv[iv]));
        mulr2=cvolsv2[iv]+(fcv*(cvolsv2[iv+1]-cvolsv2[iv]));
        }

        float cut_in=*cutoff;
        if(cut_in>1.0f)cut_in=1.0f;
        if(cut_in<0.0f)cut_in=0.0f;
        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        f =fv_tbl[fk0]+(fract_fk0*(fv_tbl[fk0+1]-fv_tbl[fk0]));
        f2=f;

        if(o<0.0f)o=-o;
        kr=(o-1.0f)*0.25f;
        if(kr>0.25f)kr=0.25f;
        if(kr<0.0f)kr=0.0f;
        float q=(1.0f-kr)*(*resonance*0.996f);
		diff= 1.f - q;
        float float_rk0=q*10000.0f;
        int rk0=int(float_rk0);
        if(f>rv_tbl[rk0] )f=rv_tbl[rk0];
        if(f2>rv_tbl[0] )f2=rv_tbl[0];
        //f2=f;
        }

        low += f * band;
        if(low>SVL)low=SVL;
        if(low<-SVL)low=-SVL;
		high = (*in*0.25f) - low - band * diff;
		band += high * f;

        low2 += f2 * band2;
		high2 = low  - low2 - band2;
		band2 += high2 * f2;

        o=(((*mix*0.98f)*(low2-low))+low)*mulr;//(mix*(b-a))+a
        *out=o*4.0f*mulr2;

		in++;
		cutoff++;
		resonance++;
        out++;
        mix++;
	}
}
void FxSvf::subProcess_2( int bufferOffset, int sampleFrames )
{
//SVHPR
	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
	float* mix	= bufferOffset + pinSVFPoles.getBuffer();

	for( int s = sampleFrames; s > 0; --s )
	{

        if(*cutoff !=cut0 || *resonance !=res0)
        {
        cut0 = *cutoff;

        if(*resonance !=res0 )
        {
        res0=*resonance;
        float cv=res0*20.0f;
        int iv=int(cv);
        float fcv=cv-float(iv);
        mulr=cvolsv[iv]+(fcv*(cvolsv[iv+1]-cvolsv[iv]));
        mulr2=cvolsv2[iv]+(fcv*(cvolsv2[iv+1]-cvolsv2[iv]));
        }

        float cut_in=*cutoff;
        if(cut_in>1.0f)cut_in=1.0f;
        if(cut_in<0.0f)cut_in=0.0f;
        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        f =fv_tbl[fk0]+(fract_fk0*(fv_tbl[fk0+1]-fv_tbl[fk0]));
        f2=f;

        if(o<0.0f)o=-o;
        kr=(o-1.0f)*0.25f;
        if(kr>0.25f)kr=0.25f;
        if(kr<0.0f)kr=0.0f;
        float q=(1.0f-kr)*(*resonance*0.996f);
		diff= 1.f - q;
        float float_rk0=q*10000.0f;
        int rk0=int(float_rk0);
        if(f>rv_tbl[rk0] )f=rv_tbl[rk0];
        if(f2>rv_tbl[0] )f2=rv_tbl[0];
        //f2=f;
        }




        low += f * band;
        if(low>SVL)low=SVL;
        if(low<-SVL)low=-SVL;
		high = (*in*0.25f) - low - band * diff;
		band += high * f;

        low2 += f2 * band2;
		high2 = high  - low2 - band2;
		band2 += high2 * f2;

        o=(((*mix*0.98f)*(high2-high))+high)*mulr;//(mix*(b-a))+a
        *out=o*4.0f*mulr2;

		in++;
		cutoff++;
		resonance++;
        out++;
        mix++;
	}


}
void FxSvf::subProcess_3( int bufferOffset, int sampleFrames )
{
//SVBPR
	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
	float* mix	= bufferOffset + pinSVFPoles.getBuffer();

	for( int s = sampleFrames; s > 0; --s )
	{


        if(*cutoff !=cut0 || *resonance !=res0)
        {
        cut0 = *cutoff;

        if(*resonance !=res0 )
        {
        res0=*resonance;
        float cv=res0*20.0f;
        int iv=int(cv);
        float fcv=cv-float(iv);
        mulr=cvolsv[iv]+(fcv*(cvolsv[iv+1]-cvolsv[iv]));
        mulr2=cvolsv2[iv]+(fcv*(cvolsv2[iv+1]-cvolsv2[iv]));
        }

        float cut_in=*cutoff;
        if(cut_in>1.0f)cut_in=1.0f;
        if(cut_in<0.0f)cut_in=0.0f;
        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        f =fv_tbl[fk0]+(fract_fk0*(fv_tbl[fk0+1]-fv_tbl[fk0]));
        f2=f;

        if(o<0.0f)o=-o;
        kr=(o-1.0f)*0.25f;
        if(kr>0.25f)kr=0.25f;
        if(kr<0.0f)kr=0.0f;
        float q=(1.0f-kr)*(*resonance*0.996f);
		diff= 1.f - q;
        float float_rk0=q*10000.0f;
        int rk0=int(float_rk0);
        if(f>rv_tbl[rk0] )f=rv_tbl[rk0];
        if(f2>rv_tbl[0] )f2=rv_tbl[0];
        //f2=f;
        }

        low += f * band;
        if(low>SVL)low=SVL;
        if(low<-SVL)low=-SVL;
		high = (*in*0.25f) - low - band * diff;
		band += high * f;


        low2 += f2 * band2;
		high2 = band - low2 - band2;// * diff2;
		band2 += high2 * f2;


        o=(((*mix*0.98f)*(band2-band))+band)*mulr;//(mix*(b-a))+a;
        *out=o*4.0f*mulr2;


        /*if(o<0.0f)o=-o;
        if(o>4.2f || o<-4.2f)kr+=0.0001f;
        else
        kr-=0.0001f;
        if(kr>0.25f)kr=0.25f;
        if(kr<0.0f)kr=0.0f;*/

		in++;
		cutoff++;
		resonance++;
        out++;
        mix++;
	}
}
void FxSvf::subProcess_4( int bufferOffset, int sampleFrames )
{
//SVBRR
	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
	float* mix	= bufferOffset + pinSVFPoles.getBuffer();

	for( int s = sampleFrames; s > 0; --s )
	{

        if(*cutoff !=cut0 || *resonance !=res0)
        {
        cut0 = *cutoff;
        res0 = *resonance;

        float cut_in=*cutoff;
        if(cut_in>0.93f )cut_in=0.93f;
        if(cut_in<0.0f)cut_in=0.0f;

        //float q=*resonance*0.98f;

		//diff= 1.f - q;


        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        f =fv_tbl[fk0]+(fract_fk0*(fv_tbl[fk0+1]-fv_tbl[fk0]));

        f2=f;

        //float float_rk0=q*10000.0f;
        //int rk0=int(float_rk0);
        //if(f>rv_tbl[rk0] )f=rv_tbl[rk0];
        //if(f2>rv_tbl[0] )f2=rv_tbl[0];

        }


        low += f * band;
        //if(low>SVL)low=SVL;
        //if(low<-SVL)low=-SVL;
		high = *in - low - band;//*diff
		band += high * f;

        float n= high + low;

        low2 += f2 * band2;
		high2 = n - low2 - band2;
		band2 += high2 * f2;

        float n2= high2 + low2;

         o=(*mix*(n2-n))+n;//(mix*(b-a))+a;
        *out=o;



		in++;
		cutoff++;
		resonance++;
        out++;
        mix++;
	}
}
void FxSvf::subProcess_5( int bufferOffset, int sampleFrames )
{
//LP6
	// get pointers to in/output buffers.
	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
	float* mix	= bufferOffset + pinSVFPoles.getBuffer();

	for( int s = sampleFrames; s > 0; --s )
	{
        if(*cutoff !=cut0 || *resonance !=res0)
        {
        cut0 = *cutoff;
        res0 = *resonance;
        float cut_in=*cutoff;
        if(cut_in>1.0f )cut_in=1.0f;
        if(cut_in<0.0f)cut_in=0.0f;

        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        fp6 =p6_tbl[fk0]+(fract_fk0*(p6_tbl[fk0+1]-p6_tbl[fk0]));
        }

		// low pass
		yp6 = *in + fp6 * ( yp6 - *in );

		//kill_denormal(yp6);

		*out = yp6;

		in++;
		cutoff++;
		resonance++;
        out++;
        mix++;
	}
}
void FxSvf::subProcess_6( int bufferOffset, int sampleFrames )
{
//HP6
	// get pointers to in/output buffers.
	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
	float* mix	= bufferOffset + pinSVFPoles.getBuffer();

	for( int s = sampleFrames; s > 0; --s )
	{
        if(*cutoff !=cut0 || *resonance !=res0)
        {
        cut0 = *cutoff;
        res0 = *resonance;
        float cut_in=*cutoff;
        if(cut_in>1.0f )cut_in=1.0f;
        if(cut_in<0.0f)cut_in=0.0f;

        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        fp6 =p6_tbl[fk0]+(fract_fk0*(p6_tbl[fk0+1]-p6_tbl[fk0]));

        }

		// low pass
		yp6 = *in + fp6 * ( yp6 - *in );

		*out = *in-yp6;

		in++;
		cutoff++;
		resonance++;
        out++;
        mix++;
	}
}
void FxSvf::subProcess_7( int bufferOffset, int sampleFrames )
{
//EQ
	// get pointers to in/output buffers.
	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
	float* mix	= bufferOffset + pinSVFPoles.getBuffer();

	for( int s = sampleFrames; s > 0; --s )
	{

        if(*cutoff !=cut0 || *resonance !=res0)
        {
        cut0 = *cutoff;
        res0 = *resonance;
        float c=*cutoff*2.0f;
        float cut_in=c;
        if(cut_in>1.0f )cut_in=1.0f;
        if(cut_in<0.0f)cut_in=0.0f;

        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        fp6 =p6_tbl[fk0]+(fract_fk0*(p6_tbl[fk0+1]-p6_tbl[fk0]));

        float cut_inb=c-1.0f;
        if(cut_inb>1.0f )cut_inb=1.0f;
        if(cut_inb<0.0f)cut_inb=0.0f;

        float_fk0=cut_inb*10000.0f;
        fk0=int(float_fk0);
        fract_fk0=float_fk0-float(fk0);
        fp6b =p6_tbl[fk0]+(fract_fk0*(p6_tbl[fk0+1]-p6_tbl[fk0]));

        }

		// low pass
		yp6 = *in + fp6 * ( yp6 - *in );

        //kill_denormal(yp6);

		yp6b = yp6 + fp6b * ( yp6b - yp6 );

        //kill_denormal(yp6b);

		*out = yp6-yp6b;

		in++;
		cutoff++;
		resonance++;
        out++;
        mix++;
	}
}
void FxSvf::subProcess_8( int bufferOffset, int sampleFrames )
{
	// get pointers to in/output buffers.
	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
	float* mix	= bufferOffset + pinSVFPoles.getBuffer();


	for( int s = sampleFrames; s > 0; --s )
	{


        if(*cutoff !=cut0 || *resonance !=res0)
        {
        cut0 = *cutoff;
        res0 = *resonance;
        float cut_in=*cutoff;
        if(cut_in>1.0f )cut_in=1.0f;
        if(cut_in<0.0f)cut_in=0.0f;

        //FSAMPLE q_const = 1.5f * p_resonance + 0.5f;
        //q_const = max(q_const, 0.5f );
        //q_const = min(q_const, 4.f );

        float  q_const = 1.5f * *resonance + 0.5f;
        if(q_const<0.5f)q_const=0.5f;
        if(q_const>4.0f)q_const=4.0f;



        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        float cosA =apcosA_tbl[fk0]+(fract_fk0*(apcosA_tbl[fk0+1]-apcosA_tbl[fk0]));


  		//float idx = f * q_const;
		//idx = min( idx, 1.f );
		//factor_A = lookup_tableB->GetValueInterp2( f ) * lookup_tableA->GetValueInterp2( 0.5f * idx );
		//factor_B = lookup_tableA->GetValueInterp2( idx );


        //float idx=*resonance*cut_in*cut_in;
        float idx=cut_in*q_const;
        if(idx<0.0f)idx=0.0f;
        if(idx>1.0f)idx=1.0f;

         float_fk0=idx*10000.0f;
         fk0=int(float_fk0);
         fract_fk0=float_fk0-float(fk0);
         float expA =apexpA_tbl[fk0]+(fract_fk0*(apexpA_tbl[fk0+1]-apexpA_tbl[fk0]));
         float expB =apexpB_tbl[fk0]+(fract_fk0*(apexpB_tbl[fk0+1]-apexpB_tbl[fk0]));


 		factor_A = cosA * expA;
		factor_B = expB;

        }


		float temp = delay1 * factor_A;
		float inB=*in;
		float temp3 = inB  + temp - delay2 * factor_B;
        float outA = temp3 * factor_B + delay2 - temp;
        *out=(*mix*(outA-inB))+inB;//(mix*(b-a))+a;

		//kill_denormal(temp3);

		delay2 = delay1;
		delay1 = temp3;


		in++;
		cutoff++;
		resonance++;
        out++;
        mix++;
	}
}
void FxSvf::onSetPins(void)
{

        /*if( pinVoiceReset.isUpdated() && pinVoiceReset>0.0f)ResetVoice();;
        if(res_mode.isUpdated())ResetVoice();


        if((!pinIn.isStreaming() && pinIn == 0.f) || pinOn==0 )
        {
        run=false;
        pinOut.setStreaming(false);
        SET_PROCESS(&FxSvf::subProcess_0);
        setSleep(true);
        ResetVoice();
        }
        else
        {
            if(run==false)
            {
                run=true;
                pinOut.setStreaming(true);
                if(res_mode==0)SET_PROCESS(&FxSvf::subProcess);//--
                if(res_mode==1)SET_PROCESS(&FxSvf::subProcess_1);
                if(res_mode==2)SET_PROCESS(&FxSvf::subProcess_2);
                if(res_mode==3)SET_PROCESS(&FxSvf::subProcess_3);
                if(res_mode==4)SET_PROCESS(&FxSvf::subProcess_4);
                if(res_mode==5)SET_PROCESS(&FxSvf::subProcess_5);
                if(res_mode==6)SET_PROCESS(&FxSvf::subProcess_6);
                if(res_mode==7)SET_PROCESS(&FxSvf::subProcess_7);
                if(res_mode==8)SET_PROCESS(&FxSvf::subProcess_8);
                setSleep(false);
            }
        }*/


        if( pinVoiceReset.isUpdated() && pinVoiceReset>0.0f )ResetVoice();

        if( pinOn==1)
        {

            if( !pinIn.isStreaming() && pinIn == 0.f )
            {
            pinOut.setStreaming(false);
            SET_PROCESS(&FxSvf::subProcess_0);
            setSleep(true);
            if(pinVoice>0.f)ResetVoice();
            }
            else
            {
            if(res_mode.isUpdated())ResetVoice();
            pinOut.setStreaming(true);
            if(res_mode==0)SET_PROCESS(&FxSvf::subProcess);//--
            if(res_mode==1)SET_PROCESS(&FxSvf::subProcess_1);
            if(res_mode==2)SET_PROCESS(&FxSvf::subProcess_2);
            if(res_mode==3)SET_PROCESS(&FxSvf::subProcess_3);
            if(res_mode==4)SET_PROCESS(&FxSvf::subProcess_4);
            if(res_mode==5)SET_PROCESS(&FxSvf::subProcess_5);
            if(res_mode==6)SET_PROCESS(&FxSvf::subProcess_6);
            if(res_mode==7)SET_PROCESS(&FxSvf::subProcess_7);
            if(res_mode==8)SET_PROCESS(&FxSvf::subProcess_8);
            setSleep(false);
            }

        }
        else
        {
        pinOut.setStreaming(false);
        SET_PROCESS(&FxSvf::subProcess_0);
        setSleep(true);
        ResetVoice();
        }

}
void FxSvf::ResetVoice()
{
    cut0=pinCutoff+0.0000001f;
    res0=pinRes+0.0000001f;

    kr=0.0f;
    o=0.0f;

    yp6=0.0f;
    yp6b=0.0f;

    delay2 = delay1 = 0.0f;

    low=0.0f;
    band=0.0f;
    high=0.0f;
    notch=0.0f;

    low2=0.0f;
    band2=0.0f;
    high2=0.0f;
    notch2=0.0f;
}
