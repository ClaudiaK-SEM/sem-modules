/*

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

***********************

About the original base of code from the musicdsp.org archives:

Moog ladder filter code is based on the work of Antti Huovilainen.
Credits: Victor Lazzarini has implemented Antti's model in Csound, now the new url is https://www.csounds.com.
You will find all infos here on the new url of musicdsp.org, I found them since many years ago:
https://www.musicdsp.org/en/latest/Filters/196-moog-filter.html

For the other Moog code:
I rewrote the tuning and I like this one because it's easy to do many variations but that needs to be tuned if you change the poles number...
https://www.musicdsp.org/en/latest/Filters/24-moog-vcf.html

The base of SVF code is from various sources like musicdsp.org and Jeff McClintock's SDK3 source files.
https://www.synthedit.com/software-development-kit/

Usually I work on the quality of resonance and the tuning, the base to do a good filter, I try to control the level of the resonance ^^, etc...
I added a simple oversampling code based on IIR filters done with WinFilter.

Claudia Kalensky, KX77FREE 09/2024.

The sources are available on:
http://kx77free.free.fr/data/Some-KX77FREE-sem-source-code.zip

The releases are done with Code::Blocks and MSV 2022, Stable SDK, this module is included inside the
X64 CK KX77FREE Tools.sem

*/


#include ".\Filters.h"
//#include "math.h"
#include <mutex>

#define ONEPI 3.1415926535f
#define SVL 50.0f

   const float ACoef2[NCoef+1] = {
        0.00043129342223095427,
        0.00345034737784763410,
        0.01207621582246671900,
        0.02415243164493343800,
        0.03019053955616679900,
        0.02415243164493343800,
        0.01207621582246671900,
        0.00345034737784763410,
        0.00043129342223095427
    };

    const float BCoef2[NCoef+1] = {
        1.00000000000000000000,
        -3.81307666472334230000,
        8.19217369565690840000,
        -11.75732332758495600000,
        12.11412703451017900000,
        -9.07397777071923710000,
        4.82474299365420210000,
        -1.67857586087027230000,
        0.30113216341142074000
    };



REGISTER_PLUGIN ( CK_Filters, L"Open-CK-FILTERS" );

CK_Filters::CK_Filters( IMpUnknown* host ) : MpBase( host )
{
	// Register pins.
	initializePin( pinIn );
	initializePin( pinCutoff );
	initializePin( pinRes );
	initializePin( pinOut );
	initializePin( res_mode );
	initializePin( pinSVFPoles );
    initializePin( pinOnOff );
    initializePin( pinVoiceReset );


}
int32_t CK_Filters::open()
{
    run=false;

	// fix for race conditions.
	static std::mutex safeInit;
	std::lock_guard<std::mutex> lock(safeInit);

    int32_t need_initialise;
	getHost()->allocateSharedMemory( L"CK_ok0-pitch", (void**) &fk0_tbl, getSampleRate(), 10552*sizeof(float), need_initialise );

    int32_t need_initialise1;
	getHost()->allocateSharedMemory( L"CK_ok0_2-pitch", (void**) &ofk0_tbl, getSampleRate(), 10552*sizeof(float), need_initialise1 );

    int32_t need_initialise3;
	getHost()->allocateSharedMemory( L"CK_oslo0-pitch", (void**) &oflo0_tbl, getSampleRate(), 10552*sizeof(float), need_initialise3 );

    int32_t need_initialise7;
	getHost()->allocateSharedMemory( L"CK_ohp-pitch", (void**) &hp_k0_tbl, getSampleRate(), 10552*sizeof(float), need_initialise7 );


    int32_t need_initialise11;
	getHost()->allocateSharedMemory( L"CK_o_Tanh", (void**) &tanh_tbl, getSampleRate(), 20002*sizeof(float), need_initialise11 );


    int32_t need_initialise12;
	getHost()->allocateSharedMemory( L"CK_SV1_volt", (void**) &fv_tbl, getSampleRate(), 10002*sizeof(float), need_initialise12 );

    int32_t need_initialise13;
	getHost()->allocateSharedMemory( L"CK_SV1_res", (void**) &rv_tbl, getSampleRate(), 10002*sizeof(float), need_initialise13 );


    int32_t need_initialise18;
	getHost()->allocateSharedMemory( L"CK_okacr", (void**) &kacr_tbl, getSampleRate(), 10552*sizeof(float), need_initialise18 );

    int32_t need_initialise19;
	getHost()->allocateSharedMemory( L"CK_ok2vg", (void**) &k2vg_tbl, getSampleRate(), 10552*sizeof(float), need_initialise19 );

    int32_t need_initialise20;
	getHost()->allocateSharedMemory( L"CK_okacro", (void**) &kacro_tbl, getSampleRate(), 10552*sizeof(float), need_initialise20 );

    int32_t need_initialise21;
	getHost()->allocateSharedMemory( L"CK_ok2vgo", (void**) &k2vgo_tbl, getSampleRate(), 10552*sizeof(float), need_initialise21 );


	if(need_initialise7)
    {

			float sr=getSampleRate();
            float sr2=getSampleRate()*2.0f;


  			float low = 440.0f*powf(2.0f,((1.0f-0.152999f)-5.0f));

			for (int i=0; i<10551; i++)
			{
 			float cut = 440.0f*powf(2.0f,(((float(i)*0.001f)-0.152999f)-5.0f));

			float cuthp=cut;
			if(i<1001 && sr>96000.0f )cuthp=low;

			float f=2.0f*cut/sr;
			float k=3.516399f*f-0.525050f*f*f-1.0f;
            fk0_tbl[i] = k;                                 //0

            float hpcut=(sr/2.0f)-cuthp;
			float fhp=2.0f*hpcut/sr;
			float khp=3.6f*fhp-1.5999f*fhp*fhp-1.0f;

            hp_k0_tbl[i]=khp;                               //7

			float f2=2.0f*cut/sr2;
			float k2=3.516399f*f2-0.525050f*f2*f2-1.0f;
            ofk0_tbl[i] = k2;                               //1
            float p2=(k2+1.0f)*0.5f;
            oflo0_tbl[i]=-1.3f+expf(-((p2*0.5f)-0.78f)*2.2f);//over         //3

			}

    }


    if(need_initialise11)
    {
             for (int i=0; i<20001; i++)//11001
            {
            float x2=(i*0.0001f)-1.0f;
            tanh_tbl[i]= tanhf(x2);             //11
            //tanh2_tbl[i]=tanhf(x2*2.0f);
            }
    }

    if(need_initialise13)
    {
            float sr=getSampleRate();

            for(int i=0;i<10001;i++)
            {
            float volt=float(i*0.001f);
            float f0=ONEPI*(440.0f*powf(2.0f,(volt-5.0f)))/sr;
            fv_tbl[i] =2.0f*sinf(f0);                                                       //12
            float r=float(i*0.0512f);
            rv_tbl[i]=(0.000025f  * r * r + 0.009f * r + 7.8f)*0.1f;//0.1f=512 table_size   //13
            }
    }

	if(need_initialise21)
    {

        float sr=getSampleRate();
        float sr2=getSampleRate()*2.0f;

        for (int i=0; i<10501; i++)//10601=over , 10111=no over
        {
        float freq =440.0f*powf(2.0f,((float(i)*0.001f)-5.0f)+0.022f);//0.0557f

        float kfc  = freq/(sr*0.5f); // sr is half the actual filter sampling rate
        float kf   = freq/sr;
        float kfcr = 1.8730f*(kfc*kfc*kfc) + 0.4955f*(kfc*kfc) - 0.6490f*kfc + 0.9988f;
        kacr_tbl[i] = -3.9364f*(kfc*kfc) + 1.8409f*kfc + 0.9968f;//c
        float x  = -2.0f * ONEPI * kfcr * kf;
        float exp_out  = expf(x);
        k2vg_tbl[i]=1.0f-exp_out;

        float kfco  = freq/(sr2*0.5f); // sr is half the actual filter sampling rate
        float kfo   = freq/sr2;
        float kfcro = 1.8730f*(kfco*kfco*kfco) + 0.4955f*(kfco*kfco) - 0.6490f*kfco + 0.9988f;
        kacro_tbl[i] = -3.9364f*(kfco*kfco) + 1.8409f*kfco + 0.9968f;//c
        float xo  = -2.0f * ONEPI * kfcro * kfo;
        float exp_outo  = expf(xo);
        k2vgo_tbl[i]=1.0f-exp_outo;


        }

    }


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



    cut0=-100.0f;

    delay_inA=0.0f;

    for( int n=0; n<NCoef+1; n++)
    {
        yA2[n]=0.0f; //output samples
        xA2[n]=0.0f; //input samples
        yD2[n]=0.0f; //output samples
        xD2[n]=0.0f; //input samples
    }

    kr=0.0f;
    o=0.0f;

    x0=0.0f;
    y0=0.0f;
    y1=0.0f;
    y2=0.0f;
    y3=0.0f;
    y4=0.0f;

    oldx0=0.0f;
    oldy0=0.0f;
    oldy1=0.0f;
    oldy2=0.0f;
    oldy3=0.0f;

    ly0=0.0f;
    ly1=0.0f;
    ly2=0.0f;
    ly3=0.0f;
    loldx0=0.0f;
    loldy0=0.0f;
    loldy1=0.0f;
    loldy2=0.0f;

    hy0=0.0f;
    hy1=0.0f;
    hy2=0.0f;
    hy3=0.0f;
    holdx0=0.0f;
    holdy0=0.0f;
    holdy1=0.0f;
    holdy2=0.0f;

    amf=0.0f;
    az1=0.0f;
    az2=0.0f;
    az3=0.0f;
    az4=0.0f;
    az5=0.0f;
    ay1=0.0f;
    ay2=0.0f;
    ay3=0.0f;
    ay4=0.0f;


    low=0.0f;
    band=0.0f;
    high=0.0f;
    notch=0.0f;

    low2=0.0f;
    band2=0.0f;
    high2=0.0f;
    notch2=0.0f;


	return MpBase::open();
}
void CK_Filters::sub_process( int bufferOffset, int sampleFrames )
{
	// BYPASS

	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();

        for( int s = sampleFrames ; s > 0 ; s-- )
        {

        *out =*in;

		in++;
		cutoff++;
		resonance++;
        out++;
    }
}
void CK_Filters::sub_process_1( int bufferOffset, int sampleFrames )
{
	// RES OFF

	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();

        for( int s = sampleFrames ; s > 0 ; s-- )
        { // main loop

        if( *cutoff !=cut0)
        {
        cut0 =*cutoff;
        cut_in=*cutoff;
        if(cut_in>0.99f)cut_in=0.99f;
        if(cut_in<0.0f)cut_in=0.0f;
        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        k0 =fk0_tbl[fk0]+(fract_fk0*(fk0_tbl[fk0+1]-fk0_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        //p0 =fp0_tbl[fk0]+(fract_fk0*(fp0_tbl[fk0+1]-fp0_tbl[fk0]));
        }

		x0=*in;

                    y0= (x0+oldx0)*p0 - (k0*y0);

                    y1= (y0+oldy0)*p0 - (k0*y1);

                    y2= (y1+oldy1)*p0 - (k0*y2);

                    y3= (y2+oldy2)*p0 - (k0*y3);



        *out = y3;

		oldx0=x0;
		oldy0=y0;
		oldy1=y1;
		oldy2=y2;

		in++;
		cutoff++;
		resonance++;
        out++;
	}

}
void CK_Filters::sub_process_5( int bufferOffset, int sampleFrames )
{
//Mg*

	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
	float *sv = bufferOffset + pinSVFPoles.getBuffer();

        for( int s = sampleFrames ; s > 0 ; s-- )
        {

        if( *cutoff !=cut0 || *resonance !=res0)
        {
        cut0 =*cutoff;
        res0=res1=*resonance;
        res1=res0*0.935f;
        cut_in=*cutoff-0.0009f;
        if(cut_in>1.039f)cut_in=1.039f;
        if(cut_in<0.0f)cut_in=0.0f;
        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        k0 =ofk0_tbl[fk0]+(fract_fk0*(ofk0_tbl[fk0+1]-ofk0_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =oflo0_tbl[fk0]+(fract_fk0*(oflo0_tbl[fk0+1]-oflo0_tbl[fk0]));
        r0=res1*1.1f*scale0;
        }



         float upA=(*in - delay_inA)/2.0f;

                   for (int i=1; i<2+1; i++)
                   {

                    up_overA[i]=iirU_2((upA*i)+delay_inA);

                    x0=up_overA[i]-(r0*y3);

                    x0=ftanh4(x0);

                    y0= (x0+oldx0)*p0 - (k0*y0);

                    y1= (y0+oldy0)*p0 - (k0*y1);

                    y2= (y1+oldy1)*p0 - (k0*y2);

                    y3= (y2+oldy2)*p0 - (k0*y3);


                    oldx0=x0;
                    oldy0=y0;
                    oldy1=y1;
                    oldy2=y2;


                    over[i]=iirD_2(y3); //i

                    }

                    //*out=over[1];
                    *out=(*sv*((4.f-1)*res0)+1)*over[1];


                    oldx0=x0;
                    oldy0=y0;
                    oldy1=y1;
                    oldy2=y2;


        delay_inA=*in;

		in++;
		cutoff++;
		resonance++;
        out++;
        sv++;
    }
}
void CK_Filters::sub_process_6( int bufferOffset, int sampleFrames )
{
//MgS*

	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
	float *sv = bufferOffset + pinSVFPoles.getBuffer();

        for( int s = sampleFrames ; s > 0 ; s-- )
        {

        if( *cutoff !=cut0 || *resonance !=res0 )
        {
        cut0 =*cutoff;
        res0=*resonance;
        cut_in=*cutoff-0.00395f;
        if(cut_in>1.039f)cut_in=1.039f;
        if(cut_in<0.0f)cut_in=0.0f;
        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        k0 =ofk0_tbl[fk0]+(fract_fk0*(ofk0_tbl[fk0+1]-ofk0_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =oflo0_tbl[fk0]+(fract_fk0*(oflo0_tbl[fk0+1]-oflo0_tbl[fk0]));
        r0=res0*1.1f*scale0;
        }

         float upA=(*in - delay_inA)/2.0f;

                   for (int i=1; i<2+1; i++)
                   {
                   up_overA[i]=iirU_2((upA*i)+delay_inA);

                    x0=up_overA[i]-r0*y3;

                    y0= (x0+oldx0)*p0 - (k0*y0);

                    y1= (y0+oldy0)*p0 - (k0*y1);

                    y2= (y1+oldy1)*p0 - (k0*y2);

                    y3= (y2+oldy2)*p0 - (k0*y3);


                    y3=y3-(res0*(y3*y3*y3)/6.0f);



                    oldx0=x0;
                    oldy0=y0;
                    oldy1=y1;
                    oldy2=y2;

                    over[i]=iirD_2(y3);

                    }

                    //*out=over[1];
                    *out=(*sv*((4.f-1)*res0)+1)*over[1];



        delay_inA=*in;

		in++;
		cutoff++;
		resonance++;
        out++;
        sv++;
    }
}
void CK_Filters::sub_process_7( int bufferOffset, int sampleFrames )
{
	// HP

	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();

    for( int s = sampleFrames ; s > 0 ; s-- )
    {

        if( *cutoff !=cut0 || *resonance !=res0 )
        {

        cut0=*cutoff;
        res0=*resonance;
        float cut_in=*cutoff;
        if(cut_in>1.039f)cut_in=1.039f;
        if(cut_in<0.0f)cut_in=0.0f;
        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);


        hk0=hp_k0_tbl[fk0]+(fract_fk0*(hp_k0_tbl[fk0+1]-hp_k0_tbl[fk0]));
        hp0=(hk0+1.0f)*0.5f;

        }


   		hx0=*in;

        if(hx0>2.0f)hx0=2.0f;
        if(hx0<-2.0f)hx0=-2.0f;

                    hy0= (hx0-holdx0)*hp0 + (hk0*hy0);

                    hy1= (hy0-holdy0)*hp0 + (hk0*hy1);

                    hy2= (hy1-holdy1)*hp0 + (hk0*hy2);

                    hy3= (hy2-holdy2)*hp0 + (hk0*hy3);

        *out =hy3;

 		holdx0=hx0;
		holdy2=hy2;
		holdy0=hy0;
		holdy1=hy1;


		in++;
		cutoff++;
		resonance++;
        out++;

    }
}
void CK_Filters::sub_process_8( int bufferOffset, int sampleFrames )
{
	// SV LP

	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
    float *mix = bufferOffset + pinSVFPoles.getBuffer();

    for( int s = sampleFrames ; s > 0 ; s-- )
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
        //if(q>0.98f)q=0.98f;
        //if(q<0.0f)q=0.0f;
		diff= 1.f - q;
        //float q2=(-*resonance * *resonance)+*resonance;
        //if(q2>0.998f)q2=0.998f;
        //if(q2<0.0f)q2=0.0f;
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
void CK_Filters::sub_process_9( int bufferOffset, int sampleFrames )
{
	// SV HP

	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
    float *mix = bufferOffset + pinSVFPoles.getBuffer();

    for( int s = sampleFrames ; s > 0 ; s-- )
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
        //if(q>0.98f)q=0.98f;
        //if(q<0.0f)q=0.0f;
		diff= 1.f - q;
        //float q2=(-*resonance * *resonance)+*resonance;
        //if(q2>0.998f)q2=0.998f;
        //if(q2<0.0f)q2=0.0f;
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
void CK_Filters::sub_process_10( int bufferOffset, int sampleFrames )
{
	// SV BP

	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
    float *mix = bufferOffset + pinSVFPoles.getBuffer();

    for( int s = sampleFrames ; s > 0 ; s-- )
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
        //if(q>0.98f)q=0.98f;
        //if(q<0.0f)q=0.0f;
		diff= 1.f - q;
        //float q2=(-*resonance * *resonance)+*resonance;
        //if(q2>0.998f)q2=0.998f;
        //if(q2<0.0f)q2=0.0f;
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
void CK_Filters::sub_process_11( int bufferOffset, int sampleFrames )
{
	// SV BR

	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
    float *mix = bufferOffset + pinSVFPoles.getBuffer();


    for( int s = sampleFrames ; s > 0 ; s-- )
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
}
void CK_Filters::sub_process_12( int bufferOffset, int sampleFrames )
{
//SVLPR
	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
    float *mix = bufferOffset + pinSVFPoles.getBuffer();


        for( int s = sampleFrames ; s > 0 ; s-- )
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
void CK_Filters::sub_process_13( int bufferOffset, int sampleFrames )
{
//SVHPR
	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
    float *mix = bufferOffset + pinSVFPoles.getBuffer();


        for( int s = sampleFrames ; s > 0 ; s-- )
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
        }




        low += f * band;
        if(low>SVL)low=SVL;
        if(low<-SVL)low=-SVL;
		high = (*in*0.25f) - low - band * diff;
		band += high * f;

        low2 += f2 * band2;
		high2 = high  - low2 - band2;
		band2 += high2 * f2;

        //o=(((*mix*0.98f)*(low2-low))+low)*mulr;//(mix*(b-a))+a
        o=(((*mix*0.98f)*(high2-high))+high)*mulr;//(mix*(b-a))+a
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
void CK_Filters::sub_process_14( int bufferOffset, int sampleFrames )
{
//SVBPR
	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
    float *mix = bufferOffset + pinSVFPoles.getBuffer();


        for( int s = sampleFrames ; s > 0 ; s-- )
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
void CK_Filters::sub_process_15( int bufferOffset, int sampleFrames )
{
//SVBRR
	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
    float *mix = bufferOffset + pinSVFPoles.getBuffer();


        for( int s = sampleFrames ; s > 0 ; s-- )
        {

        if(*cutoff !=cut0 )
        {
        cut0 = *cutoff;
        //res0 = *resonance;

        cut_in=*cutoff;
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
void CK_Filters::sub_process_4( int bufferOffset, int sampleFrames )
{
//Ladder MgL*

	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
	float *sv = bufferOffset + pinSVFPoles.getBuffer();

    for( int s = sampleFrames ; s > 0 ; s-- )
	{


        if( *cutoff !=cut0 || *resonance !=res0 )
        {
        cut0=*cutoff;
        cutL=cut0;
        res0=*resonance;
        //resL=res0;
        //resL=res0*1.105272f;
        resL=res0*1.14f;
        //resL=res0*1.0f;
        if(cutL>1.05f)cutL=1.05f;
        if(cutL<0.0f)cutL=0.0f;
        float float_volt=cutL*10000.0f;
        int int_volt=int(float_volt);
        float fract_volt=float_volt-float(int_volt);
        kacr =kacro_tbl[int_volt]+(fract_volt*(kacro_tbl[int_volt+1]-kacro_tbl[int_volt]));
        k2vg =k2vgo_tbl[int_volt]+(fract_volt*(k2vgo_tbl[int_volt+1]-k2vgo_tbl[int_volt]));

        }
        float upA=(*in - delay_inA)/2.0f;

                   for (int i=1; i<2+1; i++)
                   {

                   up_overA[i]=iirU_2((upA*i)+delay_inA);


                    ay1  = az1 + k2vg * ( ftanh4(up_overA[i]- 4.0f*resL*amf*kacr) - ftanh4(az1) );
                    az1  = ay1;
                    ay2  = az2 + k2vg * (ftanh4(ay1)-ftanh4(az2));
                    az2  = ay2;
                    ay3  = az3 + k2vg * (ftanh4(ay2)-ftanh4(az3));
                    az3  = ay3;
                    ay4  = az4 + k2vg * (ftanh4(ay3)-ftanh4(az4));
                    az4  = ay4;

                    // 1/2-sample delay for phase compensation
                    amf  = (ay4+az5)*0.5f;
                    az5  = ay4;//lp ok
                    over[i]=iirD_2(amf); //i
                    }


                    //*out=over[1];
                    *out=(*sv*((4.f-1)*res0)+1)*over[1];

        delay_inA=*in;

		in++;
		cutoff++;
		resonance++;
        out++;
        sv++;
    }


}
void CK_Filters::sub_process_3( int bufferOffset, int sampleFrames )
{
//Ladder MgL

	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
	float *sv = bufferOffset + pinSVFPoles.getBuffer();

    for( int s = sampleFrames ; s > 0 ; s-- )
	{


        if( *cutoff !=cut0 || *resonance !=res0 )
        {
        cut0=*cutoff;
        cutL=cut0;
        res0=*resonance;
        //resL=res0;
        //resL=res0*1.105272f;
        resL=res0*1.14f;
        if(cutL>1.04f)cutL=1.04f;
        if(cutL<0.0f)cutL=0.0f;
        float float_volt=cutL*10000.0f;
        int int_volt=int(float_volt);
        float fract_volt=float_volt-float(int_volt);
        kacr =kacr_tbl[int_volt]+(fract_volt*(kacr_tbl[int_volt+1]-kacr_tbl[int_volt]));
        k2vg =k2vg_tbl[int_volt]+(fract_volt*(k2vg_tbl[int_volt+1]-k2vg_tbl[int_volt]));//***

        }


                    ay1  = az1 + k2vg * ( ftanh4(*in - 4.0f*resL*amf*kacr) - ftanh4(az1) );
                    az1  = ay1;
                    ay2  = az2 + k2vg * (ftanh4(ay1)-ftanh4(az2));
                    az2  = ay2;
                    ay3  = az3 + k2vg * (ftanh4(ay2)-ftanh4(az3));
                    az3  = ay3;
                    ay4  = az4 + k2vg * (ftanh4(ay3)-ftanh4(az4));
                    az4  = ay4;

                    // 1/2-sample delay for phase compensation
                    amf  = (ay4+az5)*0.5f;
                    az5  = ay4;//lp ok

                    //*out=over[1];
                    *out=(*sv*((4.f-1)*res0)+1)*amf;


        //delay_inA=*in;

		in++;
		cutoff++;
		resonance++;
        out++;
        sv++;
    }


}
void CK_Filters::sub_process_2( int bufferOffset, int sampleFrames )
{
//Ladder MgLnr

	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();
	float *sv = bufferOffset + pinSVFPoles.getBuffer();


    for( int s = sampleFrames ; s > 0 ; s-- )
	{

        if( *cutoff !=cut0 || *resonance !=res0 )
        {
        cutL=cut0=*cutoff;
        res0=*resonance;
        //res=res_m*1.20f;//over
        resL=res0*0.965f;
        //resL=res0*0.99f;
        if(cutL>1.04f)cutL=1.04f;
        if(cutL<0.0f)cutL=0.0f;
        float float_volt=cutL*10000.0f;
        int int_volt=int(float_volt);
        float fract_volt=float_volt-float(int_volt);
        //kacr2 =kacr2_tbl[int_volt]+(fract_volt*(kacr2_tbl[int_volt+1]-kacr2_tbl[int_volt]));
        //k2vg2 =k2vg2_tbl[int_volt]+(fract_volt*(k2vg2_tbl[int_volt+1]-k2vg2_tbl[int_volt]));
        kacr =kacr_tbl[int_volt]+(fract_volt*(kacr_tbl[int_volt+1]-kacr_tbl[int_volt]));
        k2vg =k2vg_tbl[int_volt]+(fract_volt*(k2vg_tbl[int_volt+1]-k2vg_tbl[int_volt]));
        }

        //float upA=(*in - delay_inA)/2.0f;

                   //for (int i=1; i<2+1; i++)
                   //{

                   //up_overA[i]=iirU_2((upA*i)+delay_inA);


                    ay1  = az1 + k2vg * ( (*in- 4.0f*resL*amf*kacr) - az1 );
                    az1  = ay1;
                    ay2  = az2 + k2vg * (ay1-az2);
                    az2  = ay2;
                    ay3  = az3 + k2vg * (ay2-az3);
                    az3  = ay3;
                    ay4  = az4 + k2vg * (ay3-az4);
                    az4  = ay4;



                    // 1/2-sample delay for phase compensation
                    amf  = (ay4+az5)*0.5f;
                    az5  = ay4;//lp ok
                    float tmp = ftanhs2(amf,*resonance);
                    //float tmp = ftanh2(amf);
                    //over[i]=iirD_2(tmp);
                   //}

                    //*out=over[1];
                    *out=(*sv*((4.f-1)*res0)+1)*tmp;

        //delay_inA=*in;




		in++;
		cutoff++;
		resonance++;
        out++;
        sv++;
    }

}
void CK_Filters::sub_process_0( int bufferOffset, int sampleFrames )
{

	float* out	= bufferOffset + pinOut.getBuffer();

    for( int s = sampleFrames ; s > 0 ; s-- )
    {

        *out=0.0f;
        out++;

	}
}
void CK_Filters::onSetPins(void)
{

        if( pinVoiceReset.isUpdated() && pinVoiceReset>0.0f)
        {
        ResetVoice();
        }


        if((!pinIn.isStreaming() && pinIn == 0.f) || pinOnOff )
        {
        ResetVoice();
        run=false;
        pinOut.setStreaming(false);
        SET_PROCESS(&CK_Filters::sub_process_0);
        setSleep(true);
        }
        else
        {
            if(res_mode.isUpdated() || run==false)
            {

                run=true;

                ResetVoice();

                pinOut.setStreaming(true);

                if(res_mode==0)SET_PROCESS(&CK_Filters::sub_process);//Bypass
                if(res_mode==1)SET_PROCESS(&CK_Filters::sub_process_1);//NoRes

                if(res_mode==2)SET_PROCESS(&CK_Filters::sub_process_2);//MgLnr
                if(res_mode==3)SET_PROCESS(&CK_Filters::sub_process_3);//MgL
                if(res_mode==4)SET_PROCESS(&CK_Filters::sub_process_4);//MgL*

                if(res_mode==5)SET_PROCESS(&CK_Filters::sub_process_5);//Mg*
                if(res_mode==6)SET_PROCESS(&CK_Filters::sub_process_6);//MgS*

                if(res_mode==7)SET_PROCESS(&CK_Filters::sub_process_7);//Hp

                if(res_mode==8)SET_PROCESS(&CK_Filters::sub_process_8);//SvLP
                if(res_mode==9)SET_PROCESS(&CK_Filters::sub_process_9);//SvHp
                if(res_mode==10)SET_PROCESS(&CK_Filters::sub_process_10);//SvBp
                if(res_mode==11)SET_PROCESS(&CK_Filters::sub_process_11);//SvBr

                if(res_mode==12)SET_PROCESS(&CK_Filters::sub_process_12);//SvLPR
                if(res_mode==13)SET_PROCESS(&CK_Filters::sub_process_13);//SvHpR
                if(res_mode==14)SET_PROCESS(&CK_Filters::sub_process_14);//SvBpR
                if(res_mode==15)SET_PROCESS(&CK_Filters::sub_process_15);//SvBrR

                setSleep(false);
            }
        }
}
float CK_Filters::ftanh(float sample)
{
    if(sample>1.0f)sample=1.0f;
    if(sample<-1.0f)sample=-1.0f;
    float x=(sample+1.0f)*10000.0f;
    int int_x=int(x);
    float fract_x=x-float(int_x);
    float x_tanh =tanh_tbl[int_x]+(fract_x*(tanh_tbl[int_x+1]-tanh_tbl[int_x]));
    return x_tanh;
}
float CK_Filters::ftanh2(float sample)
{
    if(sample>1.5f)sample=1.5f;
    if(sample<-1.5f)sample=-1.5f;
    float x=((sample/1.5f)+1.0f)*10000.0f;
    int int_x=int(x);
    float fract_x=x-float(int_x);
    float x_tanh =tanh_tbl[int_x]+(fract_x*(tanh_tbl[int_x+1]-tanh_tbl[int_x]));
    return x_tanh*1.5f;
}
float CK_Filters::ftanhs2(float sample,float dist)
{
    float in=sample;
    if(sample>2.0f)sample=2.0f;
    if(sample<-2.0f)sample=-2.0f;
    float x=((sample/2.0f)+1.0f)*10000.0f;
    int int_x=int(x);
    float fract_x=x-float(int_x);
    float x_tanh =tanh_tbl[int_x]+(fract_x*(tanh_tbl[int_x+1]-tanh_tbl[int_x]));
    x_tanh*=2.0f;

    //float soft=((sample/2.f)*(x_tanh-in))+in;
    float soft=(dist*(x_tanh-in))+in;
    return soft;
}
float CK_Filters::ftanh4(float sample)
{
    if(sample>4.0f)sample=4.0f;
    if(sample<-4.0f)sample=-4.0f;
    float x=((sample/4.0f)+1.0f)*10000.0f;
    int int_x=int(x);
    float fract_x=x-float(int_x);
    float x_tanh =tanh_tbl[int_x]+(fract_x*(tanh_tbl[int_x+1]-tanh_tbl[int_x]));
    return x_tanh*4.0f;
}
float CK_Filters::iirU_2(float NewSample)
{
    int n;
    //shift the old samples
    for(n=NCoef; n>0; n--) {
       xA2[n] = xA2[n-1];
       yA2[n] = yA2[n-1];
    }
    //Calculate the new output
    xA2[0] = NewSample;
    yA2[0] = ACoef2[0] * xA2[0];

    for(n=1; n<=NCoef; n++)yA2[0] += ACoef2[n] * xA2[n] - BCoef2[n] * yA2[n];

    return yA2[0]*0.95f;
}
float CK_Filters::iirD_2(float NewSample)
{
    int n;
    //shift the old samples
    for(n=NCoef; n>0; n--) {
       xD2[n] = xD2[n-1];
       yD2[n] = yD2[n-1];
    }
    //Calculate the new output
    xD2[0] = NewSample;
    yD2[0] = ACoef2[0] * xD2[0];

    for(n=1; n<=NCoef; n++)yD2[0] += ACoef2[n] * xD2[n] - BCoef2[n] * yD2[n];

    return yD2[0]*0.95f;
}
void CK_Filters::ResetVoice()
{

    cut0=-100.f;
    res0=-100.f;

    delay_inA=0.0f;

    for( int n=0; n<NCoef+1; n++)
    {
        yA2[n]=0.0f; //output samples
        xA2[n]=0.0f; //input samples
        yD2[n]=0.0f; //output samples
        xD2[n]=0.0f; //input samples

    }

    kr=0.0f;
    o=0.0f;


    x0=0.0f;
    y0=0.0f;
    y1=0.0f;
    y2=0.0f;
    y3=0.0f;
    y4=0.0f;//
    oldx0=0.0f;
    oldy0=0.0f;
    oldy1=0.0f;
    oldy2=0.0f;
    oldy3=0.0f;

    ly0=0.0f;
    ly1=0.0f;
    ly2=0.0f;
    ly3=0.0f;
    loldx0=0.0f;
    loldy0=0.0f;
    loldy1=0.0f;
    loldy2=0.0f;

    hy0=0.0f;
    hy1=0.0f;
    hy2=0.0f;
    hy3=0.0f;
    holdx0=0.0f;
    holdy0=0.0f;
    holdy1=0.0f;
    holdy2=0.0f;

    amf=0.0f;
    az1=0.0f;
    az2=0.0f;
    az3=0.0f;
    az4=0.0f;
    az5=0.0f;
    ay1=0.0f;
    ay2=0.0f;
    ay3=0.0f;
    ay4=0.0f;

    low=0.0f;
    band=0.0f;
    high=0.0f;
    notch=0.0f;

    low2=0.0f;
    band2=0.0f;
    high2=0.0f;
    notch2=0.0f;

}

