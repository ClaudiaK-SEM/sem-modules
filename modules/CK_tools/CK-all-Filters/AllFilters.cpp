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

#include "AllFilters.h"
#include "math.h"
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

    const double ACoef4[NCoef+1] = {
        0.00000181304578931014,
        0.00001450436631448116,
        0.00005076528210068406,
        0.00010153056420136812,
        0.00012691320525171014,
        0.00010153056420136812,
        0.00005076528210068406,
        0.00001450436631448116,
        0.00000181304578931014
    };

    const double BCoef4[NCoef+1] = {
        1.00000000000000000000,
        -6.59235207600965630000,
        19.73558619351161500000,
        -34.94728712084968000000,
        39.96587148192154600000,
        -30.19400773738576400000,
        14.70933961900473900000,
        -4.22467872016003910000,
        0.54798974073199846000
    };
/*
    const double ACoef8[NCoef+1] = {
        0.00000001061136450579,
        0.00000008489091604633,
        0.00000029711820616217,
        0.00000059423641232434,
        0.00000074279551540542,
        0.00000059423641232434,
        0.00000029711820616217,
        0.00000008489091604633,
        0.00000001061136450579
    };

    const double BCoef8[NCoef+1] = {
        1.00000000000000000000,
        -7.41819944497597120000,
        24.28170337313563500000,
        -45.79603224918552900000,
        54.42326557826229600000,
        -41.72424550978183800000,
        20.15088518220861100000,
        -5.60469722125069050000,
        0.68732292155578545000
    };

*/

REGISTER_PLUGIN ( AllFilters, L"CK-FILTERS" );

AllFilters::AllFilters( IMpUnknown* host ) : MpBase( host )
{
	// Register pins.
	initializePin( 0, pinIn );
	initializePin( 1, pinCutoff );
	initializePin( 2, pinRes );
	initializePin( 3, pinOut );
	initializePin( 4, res_mode );
	initializePin( 5, pinSVFPoles );
    initializePin( 6, pinVoiceReset );


}
int32_t AllFilters::open()
{
    run=false;


	// fix for race conditions.
	static std::mutex safeInit;
	std::lock_guard<std::mutex> lock(safeInit);

   int32_t need_initialise;
	getHost()->allocateSharedMemory( L"CK77p_CS_ok0-pitch", (void**) &fk0_tbl, getSampleRate(), 10552*sizeof(float), need_initialise );

    int32_t need_initialise1;
	getHost()->allocateSharedMemory( L"CK77p_CS_ok0_2-pitch", (void**) &ofk0_tbl, getSampleRate(), 10552*sizeof(float), need_initialise1 );

    int32_t need_initialise2;
	getHost()->allocateSharedMemory( L"CX77p_os0-pitch", (void**) &fs0_tbl, getSampleRate(), 10552*sizeof(float), need_initialise2 );

    int32_t need_initialise3;
	getHost()->allocateSharedMemory( L"CK77p_oslo0-pitch", (void**) &oflo0_tbl, getSampleRate(), 10552*sizeof(float), need_initialise3 );

    int32_t need_initialise5;
	getHost()->allocateSharedMemory( L"CK77p_oslo1-pitch", (void**) &oflo1_tbl, getSampleRate(), 10552*sizeof(float), need_initialise5 );

    int32_t need_initialise7;
	getHost()->allocateSharedMemory( L"CK77p_ohp-pitch", (void**) &hp_k0_tbl, getSampleRate(), 10552*sizeof(float), need_initialise7 );

    int32_t need_initialise8;
	getHost()->allocateSharedMemory( L"CK77p_osloo1-pitch", (void**) &floo1_tbl, getSampleRate(), 10552*sizeof(float), need_initialise8 );

    int32_t need_initialise14;
	getHost()->allocateSharedMemory( L"CK77p_osflo4-pitch", (void**) &oflo04_tbl, getSampleRate(), 10552*sizeof(float), need_initialise14 );

    int32_t need_initialise15;
	getHost()->allocateSharedMemory( L"CK77p_osk04-pitch", (void**) &ofk04_tbl, getSampleRate(), 10552*sizeof(float), need_initialise15 );


    int32_t need_initialise11;
	getHost()->allocateSharedMemory( L"CK77p_CS_o_Tanh", (void**) &tanh_tbl, getSampleRate(), 20002*sizeof(float), need_initialise11 );

    int32_t need_initialise12;
	getHost()->allocateSharedMemory( L"CK77p_CS_SV1_volt", (void**) &fv_tbl, getSampleRate(), 10002*sizeof(float), need_initialise12 );

    int32_t need_initialise13;
	getHost()->allocateSharedMemory( L"CK77p_CS_SV1_res", (void**) &rv_tbl, getSampleRate(), 10002*sizeof(float), need_initialise13 );

    int32_t need_initialise9;
	getHost()->allocateSharedMemory( L"CK77p_CS_ok0_18_b", (void**) &k_tbl, getSampleRate(), 10402*sizeof(float), need_initialise9 );

    int32_t need_initialise10;
	getHost()->allocateSharedMemory( L"CK77p_CS_or0_18_b", (void**) &r_tbl, getSampleRate(), 10402*sizeof(float), need_initialise10 );

    int32_t need_initialise16;
	getHost()->allocateSharedMemory( L"CK77p_CS_ok0_18_bo", (void**) &k_tblo, getSampleRate(), 10402*sizeof(float), need_initialise16 );

    int32_t need_initialise17;
	getHost()->allocateSharedMemory( L"CK77p_CS_or0_18_bo", (void**) &r_tblo, getSampleRate(), 10402*sizeof(float), need_initialise17 );


    int32_t need_initialise18;
	getHost()->allocateSharedMemory( L"CK77p_okacr", (void**) &kacr_tbl, getSampleRate(), 10552*sizeof(float), need_initialise18 );

    int32_t need_initialise19;
	getHost()->allocateSharedMemory( L"CK77p_ok2vg", (void**) &k2vg_tbl, getSampleRate(), 10552*sizeof(float), need_initialise19 );

    int32_t need_initialise20;
	getHost()->allocateSharedMemory( L"CK77p_okacro", (void**) &kacro_tbl, getSampleRate(), 10552*sizeof(float), need_initialise20 );

    int32_t need_initialise21;
	getHost()->allocateSharedMemory( L"CK77p_ok2vgo", (void**) &k2vgo_tbl, getSampleRate(), 10552*sizeof(float), need_initialise21 );

    int32_t need_initialise22;
	getHost()->allocateSharedMemory( L"CK77p_okacro4", (void**) &kacro4_tbl, getSampleRate(), 10552*sizeof(float), need_initialise22 );

    int32_t need_initialise23;
	getHost()->allocateSharedMemory( L"CK77p_ok2vgo4", (void**) &k2vgo4_tbl, getSampleRate(), 10552*sizeof(float), need_initialise23 );


	if(need_initialise15)
    {

			float sr=getSampleRate();
            float sr2=getSampleRate()*2.0f;
            float sr4=getSampleRate()*4.0f;

  			float low = 440.0f*powf(2.0f,((1.0f-0.152999f)-5.0f));

			for (int i=0; i<10551; i++)
			{
 			float cut = 440.0f*powf(2.0f,(((float(i)*0.001f)-0.152999f)-5.0f));

			float cuthp=cut;
			if(i<1001 && sr>96000.0f )cuthp=low;

			float f=2.0f*cut/sr;
			float k=3.516399f*f-0.525050f*f*f-1.0f;
            fk0_tbl[i] = k;                                             //0
            float p=(k+1.0f)*0.5f;
            //fp0_tbl[i] = p;
            fs0_tbl[i]= expf((1.449986f-p)*1.229991f)-0.930001f;        //2
            //fhi0_tbl[i]=5.0f*expf(-0.3f-p*1.5f);
            //floo0_tbl[i]=expf(2.8f-p*1.6f);
            floo1_tbl[i]=expf(3.15f-p*1.75f);                           //8

            float hpcut=(sr/2.0f)-cuthp;
			float fhp=2.0f*hpcut/sr;
			float khp=3.6f*fhp-1.5999f*fhp*fhp-1.0f;

            hp_k0_tbl[i]=khp;                                           //7

			float f2=2.0f*cut/sr2;
			float k2=3.516399f*f2-0.525050f*f2*f2-1.0f;
            ofk0_tbl[i] = k2;                                           //1
            float p2=(k2+1.0f)*0.5f;
            oflo0_tbl[i]=-1.3f+expf(-((p2*0.5f)-0.78f)*2.2f);//over     //3
            oflo1_tbl[i]=expf(3.15f-p2*1.75f);//5.7=8.7 ladder          //5
            //ofse0_tbl[i]=expf(1.395f-p2*1.38f);//over

			float f4=2.0f*cut/sr4;
			float k4=3.516399f*f4-0.525050f*f4*f4-1.0f;
            ofk04_tbl[i] = k4;                                           //15
            float p4=(k4+1.0f)*0.5f;
            oflo04_tbl[i]=expf(3.15f-p4*1.75f);//5.7=8.7 ladder          //14


			}
    }

	if(need_initialise17)
    {

			float sr=getSampleRate();
            float sr2=getSampleRate()*2.0f;

			for (int i=0; i<10401; i++)
			{
            float cut=(float(i)*0.001f)-0.4f;
            float volt=cut+((cut-9.1f)*0.093f);

            if(volt>10.0f)volt=10.0f;
            if(volt<0.0f)volt=0.0f;
            float cut1 = 440.0f*powf(2.0f,volt-5.0f);

            float r=(20.8f-((cut*2.0f)-7.8f))*0.102f;
            if(r<0.0f)r=0.0f;

            float f=2.0f*cut1/sr;
            float k=3.20f*f-0.71f*f*f-0.9987f;
            k_tbl[i]=k;//k0                             //9
            float p=(k+1.0f)*0.5f;//r0
            r_tbl[i]=(expf(3.15f-p*1.75f))*r;           //10

            float f2=2.0f*cut1/sr2;
            float k2=3.20f*f2-0.71f*f2*f2-0.9987f;
            k_tblo[i]=k2;                               //16
            float p2=(k2+1.0f)*0.5f;
            r_tblo[i]=(expf(3.15f-p2*1.75f))*r;         //17

            }
    }

    if(need_initialise11)
    {
             for (int i=0; i<20001; i++)//11001
            {
            float x2=(i*0.0001f)-1.0f;
            tanh_tbl[i]= tanhf(x2);                                 //11
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
            //rv_tbl[i]=(0.0000145f  * r * r + 0.02f * r + 6.0f)*0.1f;//0.1f=512 table_size
            //float r=float(i*0.0001f);
            //rv_tbl[i]=0.2f*(expf(r*2.0f) + 2.5f);
            }
    }

	if(need_initialise23)
    {

        float sr=getSampleRate();
        float sr2=getSampleRate()*2.0f;
        float sr4=getSampleRate()*4.0f;

        for (int i=0; i<10501; i++)//10601=over , 10111=no over
        {
        float freq =440.0f*powf(2.0f,((float(i)*0.001f)-5.0f)+0.022f);//0.0557f
        //float freq =440.0f*powf(2.0f,(float(i)*0.001f)-5.0f);//0.0557f

        float kfc  = freq/(sr*0.5f); // sr is half the actual filter sampling rate
        float kf   = freq/sr;
        float kfcr = 1.8730f*(kfc*kfc*kfc) + 0.4955f*(kfc*kfc) - 0.6490f*kfc + 0.9988f;
        kacr_tbl[i] = -3.9364f*(kfc*kfc) + 1.8409f*kfc + 0.9968f;//c                                //18
        float x  = -2.0f * ONEPI * kfcr * kf;
        float exp_out  = expf(x);
        k2vg_tbl[i]=1.0f-exp_out;                                                                   //19

        float kfco  = freq/(sr2*0.5f); // sr is half the actual filter sampling rate
        float kfo   = freq/sr2;
        float kfcro = 1.8730f*(kfco*kfco*kfco) + 0.4955f*(kfco*kfco) - 0.6490f*kfco + 0.9988f;
        kacro_tbl[i] = -3.9364f*(kfco*kfco) + 1.8409f*kfco + 0.9968f;//c                            //20
        float xo  = -2.0f * ONEPI * kfcro * kfo;
        float exp_outo  = expf(xo);
        k2vgo_tbl[i]=1.0f-exp_outo;                                                                 //21

        float kfco4  = freq/(sr4*0.5f); // sr is half the actual filter sampling rate
        float kfo4   = freq/sr4;
        float kfcro4 = 1.8730f*(kfco4*kfco4*kfco4) + 0.4955f*(kfco4*kfco4) - 0.6490f*kfco4 + 0.9988f;
        kacro4_tbl[i] = -3.9364f*(kfco4*kfco4) + 1.8409f*kfco4 + 0.9968f;//c                        //22
        float xo4  = -2.0f * ONEPI * kfcro4 * kfo4;
        float exp_outo4  = expf(xo4);
        k2vgo4_tbl[i]=1.0f-exp_outo4;                                                               //23


        }
    }
/*
//tests without lookup tables

        float sr=getSampleRate();
        float sr2=getSampleRate()*2.0f;
        float sr4=getSampleRate()*4.0f;

        float low = 440.0f*powf(2.0f,((1.0f-0.152999f)-5.0f));

        for (int i=0; i<10551; i++)
        {
        float cut = 440.0f*powf(2.0f,(((float(i)*0.001f)-0.152999f)-5.0f));

        float cuthp=cut;
        if(i<1001 && sr>96000.0f )cuthp=low;

        float f=2.0f*cut/sr;
        float k=3.516399f*f-0.525050f*f*f-1.0f;
        fk0_tbl[i] = k;                                             //0
        float p=(k+1.0f)*0.5f;
        //fp0_tbl[i] = p;
        fs0_tbl[i]= expf((1.449986f-p)*1.229991f)-0.930001f;        //2
        //fhi0_tbl[i]=5.0f*expf(-0.3f-p*1.5f);
        //floo0_tbl[i]=expf(2.8f-p*1.6f);
        floo1_tbl[i]=expf(3.15f-p*1.75f);                           //8

        float hpcut=(sr/2.0f)-cuthp;
        float fhp=2.0f*hpcut/sr;
        float khp=3.6f*fhp-1.5999f*fhp*fhp-1.0f;

        hp_k0_tbl[i]=khp;                                           //7

        float f2=2.0f*cut/sr2;
        float k2=3.516399f*f2-0.525050f*f2*f2-1.0f;
        ofk0_tbl[i] = k2;                                           //1
        float p2=(k2+1.0f)*0.5f;
        oflo0_tbl[i]=-1.3f+expf(-((p2*0.5f)-0.78f)*2.2f);//over     //3
        oflo1_tbl[i]=expf(3.15f-p2*1.75f);//5.7=8.7 ladder          //5
        //ofse0_tbl[i]=expf(1.395f-p2*1.38f);//over

        float f4=2.0f*cut/sr4;
        float k4=3.516399f*f4-0.525050f*f4*f4-1.0f;
        ofk04_tbl[i] = k4;                                           //15
        float p4=(k4+1.0f)*0.5f;
        oflo04_tbl[i]=expf(3.15f-p4*1.75f);//5.7=8.7 ladder          //14
        }

        for (int i=0; i<10401; i++)
        {
        float cut=(float(i)*0.001f)-0.4f;
        float volt=cut+((cut-9.1f)*0.093f);

        if(volt>10.0f)volt=10.0f;
        if(volt<0.0f)volt=0.0f;
        float cut1 = 440.0f*powf(2.0f,volt-5.0f);

        float r=(20.8f-((cut*2.0f)-7.8f))*0.102f;
        if(r<0.0f)r=0.0f;

        float f=2.0f*cut1/sr;
        float k=3.20f*f-0.71f*f*f-0.9987f;
        k_tbl[i]=k;//k0                             //9
        float p=(k+1.0f)*0.5f;//r0
        r_tbl[i]=(expf(3.15f-p*1.75f))*r;           //10

        float f2=2.0f*cut1/sr2;
        float k2=3.20f*f2-0.71f*f2*f2-0.9987f;
        k_tblo[i]=k2;                               //16
        float p2=(k2+1.0f)*0.5f;
        r_tblo[i]=(expf(3.15f-p2*1.75f))*r;         //17
        }


        for (int i=0; i<20001; i++)//11001
        {
        float x2=(i*0.0001f)-1.0f;
        tanh_tbl[i]= tanhf(x2);                                 //11
        //tanh2_tbl[i]=tanhf(x2*2.0f);
        }

        for(int i=0;i<10001;i++)
        {
        float volt=float(i*0.001f);
        float f0=ONEPI*(440.0f*powf(2.0f,(volt-5.0f)))/sr;
        fv_tbl[i] =2.0f*sinf(f0);                                                       //12
        float r=float(i*0.0512f);
        rv_tbl[i]=(0.000025f  * r * r + 0.009f * r + 7.8f)*0.1f;//0.1f=512 table_size   //13
        //rv_tbl[i]=(0.0000145f  * r * r + 0.02f * r + 6.0f)*0.1f;//0.1f=512 table_size
        //float r=float(i*0.0001f);
        //rv_tbl[i]=0.2f*(expf(r*2.0f) + 2.5f);
        }

        for (int i=0; i<10501; i++)//10601=over , 10111=no over
        {
        float freq =440.0f*powf(2.0f,((float(i)*0.001f)-5.0f)+0.022f);//0.0557f
        //float freq =440.0f*powf(2.0f,(float(i)*0.001f)-5.0f);//0.0557f

        float kfc  = freq/(sr*0.5f); // sr is half the actual filter sampling rate
        float kf   = freq/sr;
        float kfcr = 1.8730f*(kfc*kfc*kfc) + 0.4955f*(kfc*kfc) - 0.6490f*kfc + 0.9988f;
        kacr_tbl[i] = -3.9364f*(kfc*kfc) + 1.8409f*kfc + 0.9968f;//c                                //18
        float x  = -2.0f * ONEPI * kfcr * kf;
        float exp_out  = expf(x);
        k2vg_tbl[i]=1.0f-exp_out;                                                                   //19

        float kfco  = freq/(sr2*0.5f); // sr is half the actual filter sampling rate
        float kfo   = freq/sr2;
        float kfcro = 1.8730f*(kfco*kfco*kfco) + 0.4955f*(kfco*kfco) - 0.6490f*kfco + 0.9988f;
        kacro_tbl[i] = -3.9364f*(kfco*kfco) + 1.8409f*kfco + 0.9968f;//c                            //20
        float xo  = -2.0f * ONEPI * kfcro * kfo;
        float exp_outo  = expf(xo);
        k2vgo_tbl[i]=1.0f-exp_outo;                                                                 //21

        float kfco4  = freq/(sr4*0.5f); // sr is half the actual filter sampling rate
        float kfo4   = freq/sr4;
        float kfcro4 = 1.8730f*(kfco4*kfco4*kfco4) + 0.4955f*(kfco4*kfco4) - 0.6490f*kfco4 + 0.9988f;
        kacro4_tbl[i] = -3.9364f*(kfco4*kfco4) + 1.8409f*kfco4 + 0.9968f;//c                        //22
        float xo4  = -2.0f * ONEPI * kfcro4 * kfo4;
        float exp_outo4  = expf(xo4);
        k2vgo4_tbl[i]=1.0f-exp_outo4;                                                               //23


        }

*/


    corr[0]=-0.001000f;
    corr[1]=-0.001000f;
    corr[2]=-0.000907f;
    corr[3]=-0.000907f;
    corr[4]=-0.000873f;
    corr[5]=-0.000793f;
    corr[6]=-0.000633f;
    corr[7]=-0.000323f;
    corr[8]=0.000150f;
    corr[9]=0.000644f;
    corr[10]=-0.000317f;
    corr[11]=-0.000317f;


    corr2[0]=0.000868f;
    corr2[1]=0.000868f;
    corr2[2]=0.000868f;
    corr2[3]=0.000465f;
    corr2[4]=0.000049f;
    corr2[5]=-0.000221f;
    corr2[6]=-0.000334f;
    corr2[7]=-0.000207f;
    corr2[8]=0.000185f;
    corr2[9]=0.000645f;
    corr2[10]=-0.000317f;
    corr2[11]=-0.000317f;

    //corr res
    corr3[0]=0.174f;
    corr3[1]=0.174f;
    corr3[2]=0.174f;
    corr3[3]=0.174f;
    corr3[4]=0.174f;
    corr3[5]=0.173f;
    corr3[6]=0.172f;
    corr3[7]=0.171f;
    corr3[8]=0.170f;
    corr3[9]=0.170f;

    if(getSampleRate()>44100.0f)
    {
    corr3[10]=0.179f;
    corr3[11]=0.179f;
    cr=0.918f;
    }
    else
    {
    corr3[10]=0.18146f;
    corr3[11]=0.18146f;
    cr=0.93f;
    }


    //x4
    corr4[0]=-0.000522f;  //0.541-ok
    corr4[1]=-0.000522f;
    corr4[2]=-0.000564f;
    corr4[3]=-0.000593f;
    corr4[4]=-0.000605f;
    corr4[5]=-0.000636f;
    corr4[6]=-0.000605f;
    corr4[7]=-0.000497f;
    corr4[8]=-0.000236f;
    corr4[9]= 0.000226f;
    corr4[10]=0.000666f;
    corr4[11]=0.000666f;


    corr5[0]= 0.002648f;
    corr5[1]= 0.002648f;
    corr5[2]= 0.002304f;
    corr5[3]= 0.001766f;
    corr5[4]= 0.001201f;
    corr5[5]= 0.000746f;
    corr5[6]= 0.000343f;
    corr5[7]= 0.000054f;
    corr5[8]=-0.000009f;
    corr5[9]= 0.000248f;
    corr5[10]=0.000659f;
    corr5[11]=0.000659f;


    cvol[0]= 1.0f;
    cvol[1]= 1.0f;
    cvol[2]= 1.0f;
    cvol[3]= 1.0f;
    cvol[4]= 1.0f;
    cvol[5]= 0.75f;
    cvol[6]= 0.5f;
    cvol[7]= 0.5f;
    cvol[8]= 0.5f;
    cvol[9]= 0.5f;
    cvol[10]= 0.5f;
    cvol[11]= 0.5f;

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



    cut0=-100.f;
    res0=-100.f;

    delay_inA=0.0f;

    for( int n=0; n<NCoef+1; n++)
    {
        yA2[n]=0.0f; //output samples
        xA2[n]=0.0f; //input samples
        yD2[n]=0.0f; //output samples
        xD2[n]=0.0f; //input samples

        yA4[n]=0; //output samples
        xA4[n]=0; //input samples
        yD4[n]=0; //output samples
        xD4[n]=0; //input samples
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
void AllFilters::sub_process( int bufferOffset, int sampleFrames )
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
void AllFilters::sub_process_1( int bufferOffset, int sampleFrames )
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
void AllFilters::sub_process_2( int bufferOffset, int sampleFrames )
{
	// HQ

	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();

        for( int s = sampleFrames ; s > 0 ; s-- )
        {

        if( *cutoff !=cut0 || *resonance !=res0 )
        {
        cut0 =*cutoff;
        res0=*resonance;
        cut_in=*cutoff;
        if(cut_in>0.99f)cut_in=0.99f;
        if(cut_in<0.0f)cut_in=0.0f;
        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        k0 =fk0_tbl[fk0]+(fract_fk0*(fk0_tbl[fk0+1]-fk0_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        //p0 =fp0_tbl[fk0]+(fract_fk0*(fp0_tbl[fk0+1]-fp0_tbl[fk0]));
        scale0 =fs0_tbl[fk0]+(fract_fk0*(fs0_tbl[fk0+1]-fs0_tbl[fk0]));
        r0=res0*scale0*0.9f;
        }

        lx0=*in;
        if(res0>0.0f)
        {
 		x0=*in-(r0*y3);
        if(x0>15.0f)x0=15.0f;
        if(x0<-15.0f)x0=-15.0f;
        if(fabsf(x0)>0.5f)x0=x0-(res0*0.015f*((x0*x0*x0)/6.0f));

                    y0= (x0+oldx0)*p0 - (k0*y0);

                    y1= (y0+oldy0)*p0 - (k0*y1);

                    y2= (y1+oldy1)*p0 - (k0*y2);

                    y3= (y2+oldy2)*p0 - (k0*y3);

		oldx0=x0;
		oldy0=y0;
		oldy1=y1;
		oldy2=y2;
        }

                    ly0= (lx0+loldx0)*p0 - (k0*ly0);

                    ly1= (ly0+loldy0)*p0 - (k0*ly1);

                    ly2= (ly1+loldy1)*p0 - (k0*ly2);

                    ly3= (ly2+loldy2)*p0 - (k0*ly3);

        //x =ly3;
        *out=ly3;
		if(res0>0.0f)
        {
        float tmp=((y3-ly3)*0.15f)+ly3;
		*out=(((2-1)*res0)+1)*tmp;
        }




		loldx0=lx0;
		loldy0=ly0;
		loldy1=ly1;
		loldy2=ly2;

		in++;
		cutoff++;
		resonance++;
        out++;
    }



}
void AllFilters::sub_process_3( int bufferOffset, int sampleFrames )
{
	// HQM

	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();

        for( int s = sampleFrames ; s > 0 ; s-- )
        {

        if( *cutoff !=cut0 || *resonance !=res0 )
        {
        cut0 =*cutoff;
        res0=*resonance;
        cut_in=*cutoff;
        if(cut_in>0.99f)cut_in=0.99f;
        if(cut_in<0.0f)cut_in=0.0f;
        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        k0 =fk0_tbl[fk0]+(fract_fk0*(fk0_tbl[fk0+1]-fk0_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        //p0 =fp0_tbl[fk0]+(fract_fk0*(fp0_tbl[fk0+1]-fp0_tbl[fk0]));
        scale0 =fs0_tbl[fk0]+(fract_fk0*(fs0_tbl[fk0+1]-fs0_tbl[fk0]));
        r0=res0*0.74f*scale0;

        }

        lx0=*in;

        if(lx0>1.5f)lx0=1.5f;
        if(lx0<-1.5f)lx0=-1.5f;

                    ly0= (lx0+loldx0)*p0 - (k0*ly0);

                    ly1= (ly0+loldy0)*p0 - (k0*ly1);

                    ly2= (ly1+loldy1)*p0 - (k0*ly2);

                    ly3= (ly2+loldy2)*p0 - (k0*ly3);

		loldx0=lx0;
		loldy0=ly0;
		loldy1=ly1;
		loldy2=ly2;

        if(res0>0.0f)
        {

 		x0=*in-(r0*y3);

        if(x0>1.5f)x0=1.5f;
        if(x0<-1.5f)x0=-1.5f;
        if(fabsf(x0)>0.5f)x0=x0-(res0*0.015f*((x0*x0*x0)/6.0f));

                    y0= (x0+oldx0)*p0 - (k0*y0);

                    y1= (y0+oldy0)*p0 - (k0*y1);

                    y2= (y1+oldy1)*p0 - (k0*y2);

                    y3= (y2+oldy2)*p0 - (k0*y3);

		oldx0=x0;
		oldy0=y0;
		oldy1=y1;
		oldy2=y2;

        }
        //*out =(((y3*res0*3.2f)-ly3)+ly3)/(1.0f+(res0*2.0f));


        //*out =((y3*res0*3.2f)+ly3)/(1.0f+(res0*2.0f));
        float tmp=((y3*res0*3.2f)+ly3)/(1.0f+(res0*2.0f));
		*out=(((2-1)*res0)+1)*tmp;


		in++;
		cutoff++;
		resonance++;
        out++;
    }
}
void AllFilters::sub_process_4( int bufferOffset, int sampleFrames )
{
	// LoM

	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();

        for( int s = sampleFrames ; s > 0 ; s-- )
        {

        if( *cutoff !=cut0 || *resonance !=res0 )
        {
        cut0 =*cutoff;
        res0=*resonance;
        cut_in=*cutoff;
        if(cut_in>0.99f)cut_in=0.99f;
        if(cut_in<0.0f)cut_in=0.0f;
        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        k0 =fk0_tbl[fk0]+(fract_fk0*(fk0_tbl[fk0+1]-fk0_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        //p0 =fp0_tbl[fk0]+(fract_fk0*(fp0_tbl[fk0+1]-fp0_tbl[fk0]));
        scale0 =floo1_tbl[fk0]+(fract_fk0*(floo1_tbl[fk0+1]-floo1_tbl[fk0]));
        //scale0 =floo0_tbl[fk0]+(fract_fk0*(floo0_tbl[fk0+1]-floo0_tbl[fk0]));

        //res1=res0*0.775f;//0.2325f*1.32f=0.3069f
        r0=res0*0.171f*scale0;
        }



		x0=(*in/(1.0f+res0))-r0*y3;
		//x0=sat5(x0);
        if(x0>2.0f)x0=2.0f;
        if(x0<-2.0f)x0=-2.0f;

                    y0= (x0+oldx0)*p0 - (k0*y0);

                    y1= (y0+oldy0)*p0 - (k0*y1);

                    y2= (y1+oldy1)*p0 - (k0*y2);

                    y3= (y2+oldy2)*p0 - (k0*y3);

		//y3=y3-(0.01f*(res0*(y3*y3*y3)/6.0f));
		//y3=sat(y3);

        //if(y3>1.f)y3=1.f;
        //if(y3<-1.f)y3=-1.f;

		*out = y3;
		//*out=(((2-1)*res0)+1)*y3;


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
void AllFilters::sub_process_5( int bufferOffset, int sampleFrames )
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
void AllFilters::sub_process_6( int bufferOffset, int sampleFrames )
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
void AllFilters::sub_process_7( int bufferOffset, int sampleFrames )
{
	// LP 18 OVER

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

        cut_in=cut0;
        if(cut_in>1.039f)cut_in=1.039f;
        if(cut_in<0.0f)cut_in=0.0f;

        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        k0 =k_tblo[fk0]+(fract_fk0*(k_tblo[fk0+1]-k_tblo[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =r_tblo[fk0]+(fract_fk0*(r_tblo[fk0+1]-r_tblo[fk0]));
        r0=res0*0.171f*scale0;

        }

         float in_m=*in*(1.0f+(res0*6.65f));
         float upA=(in_m - delay_inA)/2.0f;

                   for (int i=1; i<2+1; i++)
                   {

                   //up_overA[i]=(upA*i)+delay_inA;
                   up_overA[i]=iirU_2((upA*i)+delay_inA);

                 x0=(up_overA[i]/(1.0f+res0))-r0*y2;



                x0=ftanh4(x0);
               //if(x0>2.0f)x0=2.0f;
               //if(x0<-2.0f)x0=-2.0f;

                    y0= (x0+oldx0)*p0 - (k0*y0);

                    y1= (y0+oldy0)*p0 - (k0*y1);

                    y2= (y1+oldy1)*p0 - (k0*y2);




                    over[i]=iirD_2(y2); //i
                    }

                    *out=over[1];
                    //*out=(((1.5f-1)*res0)+1)*over[1];


        delay_inA=in_m ;

		oldx0=x0;
		oldy0=y0;
		oldy1=y1;

		in++;
		cutoff++;
		resonance++;
        out++;
    }
}
void AllFilters::sub_process_8( int bufferOffset, int sampleFrames )
{
	// LP 18 OVER 02

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
        res1=*resonance*0.2f;
        cut_in=cut0;
        if(cut_in>1.039f)cut_in=1.039f;
        if(cut_in<0.0f)cut_in=0.0f;
        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        k0 =k_tblo[fk0]+(fract_fk0*(k_tblo[fk0+1]-k_tblo[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =r_tblo[fk0]+(fract_fk0*(r_tblo[fk0+1]-r_tblo[fk0]));
        r0=res1*scale0;
        //r0=res0*0.171f*scale0;
        //r0=res0*scale0;
        }

         float in_m=*in*3.0f*(1.0f+(res1*6.65f));
         float upA=(in_m - delay_inA)/2.0f;

                   for (int i=1; i<2+1; i++)
                   {

                   //up_overA[i]=(upA*i)+delay_inA;
                   //up_overA[i]=iirU((upA*i)+delay_inA);
                   up_overA[i]=iirU_2((upA*i)+delay_inA);

                   x0=(up_overA[i]/(1.0f+res0))-r0*y2;



                    x0=ftanhs(x0);
        //if(x0>2.0f)x0=2.0f;
        //if(x0<-2.0f)x0=-2.0f;

                    y0= (x0+oldx0)*p0 - (k0*y0);

                    y1= (y0+oldy0)*p0 - (k0*y1);

                    y2= (y1+oldy1)*p0 - (k0*y2);


                    //*out = y2;


                    over[i]=iirD_2(y2); //i
                    //over[i]=iirD(amf); //i
                    }

                    //*out=over[1]*0.5f;
                    *out=(((1.75f-1.f)*res0)+1.f)*over[1];

                    //*out=over[1];
                    //*out=amf;


        delay_inA=in_m;

		oldx0=x0;
		oldy0=y0;
		oldy1=y1;

		in++;
		cutoff++;
		resonance++;
        out++;
    }
}
void AllFilters::sub_process_9( int bufferOffset, int sampleFrames )
{
	// LP 18

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

        cut_in=*cutoff;
        if(cut_in>1.039f)cut_in=1.039f;
        if(cut_in<0.0f)cut_in=0.0f;
        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        k0 =k_tbl[fk0]+(fract_fk0*(k_tbl[fk0+1]-k_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =r_tbl[fk0]+(fract_fk0*(r_tbl[fk0+1]-r_tbl[fk0]));
        r0=res0*0.171f*scale0;
        }

        float in_m=*in*(1.0f+(res0*6.65f));
		x0=(in_m/(1.0f+res0))-r0*y2;
        if(x0>2.0f)x0=2.0f;
        if(x0<-2.0f)x0=-2.0f;

                    y0= (x0+oldx0)*p0 - (k0*y0);

                    y1= (y0+oldy0)*p0 - (k0*y1);

                    y2= (y1+oldy1)*p0 - (k0*y2);



                   *out = y2;
                   //*out=(((1.5f-1)*res0)+1)*y2;


		oldx0=x0;
		oldy0=y0;
		oldy1=y1;

		in++;
		cutoff++;
		resonance++;
        out++;
    }

}
void AllFilters::sub_process_10( int bufferOffset, int sampleFrames )
{
	// LP18 HP

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
        k0 =k_tbl[fk0]+(fract_fk0*(k_tbl[fk0+1]-k_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =r_tbl[fk0]+(fract_fk0*(r_tbl[fk0+1]-r_tbl[fk0]));

        hk0=hp_k0_tbl[fk0]+(fract_fk0*(hp_k0_tbl[fk0+1]-hp_k0_tbl[fk0]));
        hp0=(hk0+1.0f)*0.5f;

        r0=res0*0.171f*scale0;
        }

        float in_m=*in*(1.0f+(res0*6.65f));
		x0=(in_m/(1.0f+res0))-r0*y2;
        if(x0>2.0f)x0=2.0f;
        if(x0<-2.0f)x0=-2.0f;

                    y0= (x0+oldx0)*p0 - (k0*y0);

                    y1= (y0+oldy0)*p0 - (k0*y1);

                    y2= (y1+oldy1)*p0 - (k0*y2);



		float m =y2;

		oldx0=x0;
		oldy0=y0;
		oldy1=y1;



   		hx0=m;

        //if(hx0>2.0f)hx0=2.0f;
        //if(hx0<-2.0f)hx0=-2.0f;



                    hy0= (hx0-holdx0)*hp0 + (hk0*hy0);

                    hy1= (hy0-holdy0)*hp0 + (hk0*hy1);

                    hy2= (hy1-holdy1)*hp0 + (hk0*hy2);

                    hy3= (hy2-holdy2)*hp0 + (hk0*hy3);



                    //*out =hy3;
                    *out=(((1.5f-1)*res0)+1)*hy3;


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
void AllFilters::sub_process_11( int bufferOffset, int sampleFrames )
{
	// LP18 HP2

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
        k0 =k_tbl[fk0]+(fract_fk0*(k_tbl[fk0+1]-k_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =r_tbl[fk0]+(fract_fk0*(r_tbl[fk0+1]-r_tbl[fk0]));

        hk0=hp_k0_tbl[fk0]+(fract_fk0*(hp_k0_tbl[fk0+1]-hp_k0_tbl[fk0]));
        hp0=(hk0+1.0f)*0.5f;

        r0=res0*0.171f*scale0;
        }

        float in_m=*in*(1.0f+(res0*6.65f));
		x0=(in_m/(1.0f+res0))-r0*y2;
        if(x0>2.0f)x0=2.0f;
        if(x0<-2.0f)x0=-2.0f;

                    y0= (x0+oldx0)*p0 - (k0*y0);

                    y1= (y0+oldy0)*p0 - (k0*y1);

                    y2= (y1+oldy1)*p0 - (k0*y2);

		float m =y2;

		oldx0=x0;
		oldy0=y0;
		oldy1=y1;



   		hx0=m;

        //if(hx0>2.0f)hx0=2.0f;
        //if(hx0<-2.0f)hx0=-2.0f;


                    hy0= (hx0-holdx0)*hp0 + (hk0*hy0);

                    hy1= (hy0-holdy0)*hp0 + (hk0*hy1);

                    hy2= (hy1-holdy1)*hp0 + (hk0*hy2);


		//hy3=(hy2*hp0)-(holdy2*hp0)+(hk0*hy3);

        //*out =hy2;
        *out=(((1.5f-1)*res0)+1)*hy2;

 		holdx0=hx0;
		//holdy2=hy2;
		holdy0=hy0;
		holdy1=hy1;


		in++;
		cutoff++;
		resonance++;
        out++;

    }
}
void AllFilters::sub_process_12( int bufferOffset, int sampleFrames )
{
	// LP 12

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
        k0 =k_tbl[fk0]+(fract_fk0*(k_tbl[fk0+1]-k_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =r_tbl[fk0]+(fract_fk0*(r_tbl[fk0+1]-r_tbl[fk0]));
        r0=res0*0.171f*scale0;
        }

        float in_m=*in*(1.0f+(res0*6.65f));
		x0=(in_m/(1.0f+res0))-r0*y2;
        if(x0>2.0f)x0=2.0f;
        if(x0<-2.0f)x0=-2.0f;

                    y0= (x0+oldx0)*p0 - (k0*y0);

                    y1= (y0+oldy0)*p0 - (k0*y1);

                    y2= (y1+oldy1)*p0 - (k0*y2);

		float tmp = y1*(1.0f-(0.5f*res0));
        *out=(((1.5f-1)*res0)+1)*tmp;

		oldx0=x0;
		oldy0=y0;
		oldy1=y1;

		in++;
		cutoff++;
		resonance++;
        out++;
    }
}
void AllFilters::sub_process_13( int bufferOffset, int sampleFrames )
{
	// LP 12 HP

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
        k0 =k_tbl[fk0]+(fract_fk0*(k_tbl[fk0+1]-k_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =r_tbl[fk0]+(fract_fk0*(r_tbl[fk0+1]-r_tbl[fk0]));

        hk0=hp_k0_tbl[fk0]+(fract_fk0*(hp_k0_tbl[fk0+1]-hp_k0_tbl[fk0]));
        hp0=(hk0+1.0f)*0.5f;

        r0=res0*0.171f*scale0;
        }

        float in_m=*in*(1.0f+(res0*6.65f));
		x0=(in_m/(1.0f+res0))-r0*y2;
        if(x0>2.0f)x0=2.0f;
        if(x0<-2.0f)x0=-2.0f;

                    y0= (x0+oldx0)*p0 - (k0*y0);

                    y1= (y0+oldy0)*p0 - (k0*y1);

                    y2= (y1+oldy1)*p0 - (k0*y2);

		float m =y1;

		oldx0=x0;
		oldy0=y0;
		oldy1=y1;



   		hx0=m;

        //if(hx0>2.0f)hx0=2.0f;
        //if(hx0<-2.0f)hx0=-2.0f;

                    hy0= (hx0-holdx0)*hp0 + (hk0*hy0);

                    hy1= (hy0-holdy0)*hp0 + (hk0*hy1);

                    hy2= (hy1-holdy1)*hp0 + (hk0*hy2);

                    hy3= (hy2-holdy2)*hp0 + (hk0*hy3);

        float tmp =hy3*(1.0f-(0.5f*res0));
        *out=(((1.5f-1)*res0)+1)*tmp;

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
void AllFilters::sub_process_14( int bufferOffset, int sampleFrames )
{
	// LP 12 HP2

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
        k0 =k_tbl[fk0]+(fract_fk0*(k_tbl[fk0+1]-k_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =r_tbl[fk0]+(fract_fk0*(r_tbl[fk0+1]-r_tbl[fk0]));

        hk0=hp_k0_tbl[fk0]+(fract_fk0*(hp_k0_tbl[fk0+1]-hp_k0_tbl[fk0]));
        hp0=(hk0+1.0f)*0.5f;

        r0=res0*0.171f*scale0;
        }

        float in_m=*in*(1.0f+(res0*6.65f));
		x0=(in_m/(1.0f+res0))-r0*y2;
        if(x0>2.0f)x0=2.0f;
        if(x0<-2.0f)x0=-2.0f;

                    y0= (x0+oldx0)*p0 - (k0*y0);

                    y1= (y0+oldy0)*p0 - (k0*y1);

                    y2= (y1+oldy1)*p0 - (k0*y2);

		float m =y1;

		oldx0=x0;
		oldy0=y0;
		oldy1=y1;



   		hx0=m;

        //if(hx0>2.0f)hx0=2.0f;
        //if(hx0<-2.0f)hx0=-2.0f;

                    hy0= (hx0-holdx0)*hp0 + (hk0*hy0);

                    hy1= (hy0-holdy0)*hp0 + (hk0*hy1);

                    hy2= (hy1-holdy1)*hp0 + (hk0*hy2);

		//hy3=(hy2*hp0)-(holdy2*hp0)+(hk0*hy3);

        float tmp =hy2*(1.0f-(0.5f*res0));
        *out=(((1.5f-1)*res0)+1)*tmp;


 		holdx0=hx0;
		//holdy2=hy2;
		holdy0=hy0;
		holdy1=hy1;


		in++;
		cutoff++;
		resonance++;
        out++;
    }
}
void AllFilters::sub_process_15( int bufferOffset, int sampleFrames )
{
	// LP 6

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
        k0 =k_tbl[fk0]+(fract_fk0*(k_tbl[fk0+1]-k_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =r_tbl[fk0]+(fract_fk0*(r_tbl[fk0+1]-r_tbl[fk0]));
        r0=res0*0.171f*scale0;
        }

        float in_m=*in*(1.0f+(res0*6.65f));
		x0=(in_m/(1.0f+res0))-r0*y2;
        if(x0>2.0f)x0=2.0f;
        if(x0<-2.0f)x0=-2.0f;

                    y0= (x0+oldx0)*p0 - (k0*y0);

                    y1= (y0+oldy0)*p0 - (k0*y1);

                    y2= (y1+oldy1)*p0 - (k0*y2);

        float tmp =y0*(1.0f-(0.5f*res0));
        *out=(((1.5f-1)*res0)+1)*tmp;



		oldx0=x0;
		oldy0=y0;
		oldy1=y1;

		in++;
		cutoff++;
		resonance++;
        out++;
    }
}
void AllFilters::sub_process_16( int bufferOffset, int sampleFrames )
{
	// LP 6 HP

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
        k0 =k_tbl[fk0]+(fract_fk0*(k_tbl[fk0+1]-k_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =r_tbl[fk0]+(fract_fk0*(r_tbl[fk0+1]-r_tbl[fk0]));

        hk0=hp_k0_tbl[fk0]+(fract_fk0*(hp_k0_tbl[fk0+1]-hp_k0_tbl[fk0]));
        hp0=(hk0+1.0f)*0.5f;

        r0=res0*0.171f*scale0;
        }

        float in_m=*in*(1.0f+(res0*6.65f));
		x0=(in_m/(1.0f+res0))-r0*y2;
        if(x0>2.0f)x0=2.0f;
        if(x0<-2.0f)x0=-2.0f;

                    y0= (x0+oldx0)*p0 - (k0*y0);

                    y1= (y0+oldy0)*p0 - (k0*y1);

                    y2= (y1+oldy1)*p0 - (k0*y2);

		float m =y0;

		oldx0=x0;
		oldy0=y0;
		oldy1=y1;



   		hx0=m;

        //if(hx0>2.0f)hx0=2.0f;
        //if(hx0<-2.0f)hx0=-2.0f;

                    hy0= (hx0-holdx0)*hp0 + (hk0*hy0);

                    hy1= (hy0-holdy0)*hp0 + (hk0*hy1);

                    hy2= (hy1-holdy1)*hp0 + (hk0*hy2);

                    hy3= (hy2-holdy2)*hp0 + (hk0*hy3);

        float tmp =hy3*(1.0f-(0.5f*res0));
        *out=(((1.5f-1)*res0)+1)*tmp;


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
void AllFilters::sub_process_17( int bufferOffset, int sampleFrames )
{
	// LP 6 HP2

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
        k0 =k_tbl[fk0]+(fract_fk0*(k_tbl[fk0+1]-k_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =r_tbl[fk0]+(fract_fk0*(r_tbl[fk0+1]-r_tbl[fk0]));

        hk0=hp_k0_tbl[fk0]+(fract_fk0*(hp_k0_tbl[fk0+1]-hp_k0_tbl[fk0]));
        hp0=(hk0+1.0f)*0.5f;

        r0=res0*0.171f*scale0;
        }

        float in_m=*in*(1.0f+(res0*6.65f));
		x0=(in_m/(1.0f+res0))-r0*y2;
        if(x0>2.0f)x0=2.0f;
        if(x0<-2.0f)x0=-2.0f;

                    y0= (x0+oldx0)*p0 - (k0*y0);

                    y1= (y0+oldy0)*p0 - (k0*y1);

                    y2= (y1+oldy1)*p0 - (k0*y2);

		float m =y0;

		oldx0=x0;
		oldy0=y0;
		oldy1=y1;



   		hx0=m;

        //if(hx0>2.0f)hx0=2.0f;
        //if(hx0<-2.0f)hx0=-2.0f;

                    hy0= (hx0-holdx0)*hp0 + (hk0*hy0);

                    hy1= (hy0-holdy0)*hp0 + (hk0*hy1);

                    hy2= (hy1-holdy1)*hp0 + (hk0*hy2);
		//hy3=(hy2*hp0)-(holdy2*hp0)+(hk0*hy3);

        //*out =hy2*(1.0f-(0.5f*res0));
        float tmp =hy2*(1.0f-(0.5f*res0));
        *out=(((1.5f-1)*res0)+1)*tmp;


 		holdx0=hx0;
		//holdy2=hy2;
		holdy0=hy0;
		holdy1=hy1;


		in++;
		cutoff++;
		resonance++;
        out++;
    }
}
void AllFilters::sub_process_18( int bufferOffset, int sampleFrames )
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
void AllFilters::sub_process_19( int bufferOffset, int sampleFrames )
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
void AllFilters::sub_process_20( int bufferOffset, int sampleFrames )
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
void AllFilters::sub_process_21( int bufferOffset, int sampleFrames )
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
void AllFilters::sub_process_22( int bufferOffset, int sampleFrames )
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
void AllFilters::sub_process_23( int bufferOffset, int sampleFrames )
{
	// CS LP

	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();

    for( int s = sampleFrames ; s > 0 ; s-- )
    {

        if(*cutoff !=cut0 || *resonance !=res0)
        {
        cut0 = *cutoff;
        res0 = *resonance;

        cut_in=*cutoff;
        if(cut_in>0.9f )cut_in=0.9f;
        if(cut_in<0.0f)cut_in=0.0f;

        float q=*resonance*0.88f;
		diff= 1.f - q;

        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        f =fv_tbl[fk0]+(fract_fk0*(fv_tbl[fk0+1]-fv_tbl[fk0]));

        float float_rk0=q*10000.0f;
        int rk0=int(float_rk0);
        if(f>rv_tbl[rk0] )f=rv_tbl[rk0];
        }


        low += f * band;
		high = *in - low - band * diff;
		band += high * f;

        *out=low;


		in++;
		cutoff++;
		resonance++;
        out++;
        //mix++;
	}
}
void AllFilters::sub_process_24( int bufferOffset, int sampleFrames )
{
	// CS HP

	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();

    for( int s = sampleFrames ; s > 0 ; s-- )
    {

        if(*cutoff !=cut0 || *resonance !=res0)
        {
        cut0 = *cutoff;
        res0 = *resonance;

        cut_in=*cutoff;
        if(cut_in>0.9f )cut_in=0.9f;
        if(cut_in<0.0f)cut_in=0.0f;

        float q=*resonance*0.88f;
		diff= 1.f - q;
        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        f =fv_tbl[fk0]+(fract_fk0*(fv_tbl[fk0+1]-fv_tbl[fk0]));

        float float_rk0=q*10000.0f;
        int rk0=int(float_rk0);
        if(f>rv_tbl[rk0] )f=rv_tbl[rk0];

        }


        low += f * band;
		high = *in - low - band * diff;
		band += high * f;

        *out=high;

		in++;
		cutoff++;
		resonance++;
        out++;
        //mix++;
	}
}
void AllFilters::sub_process_25( int bufferOffset, int sampleFrames )
{
	// CS BP

	float* in	= bufferOffset + pinIn.getBuffer();
	float* cutoff	= bufferOffset + pinCutoff.getBuffer();
	float* resonance	= bufferOffset + pinRes.getBuffer();
	float* out	= bufferOffset + pinOut.getBuffer();

    for( int s = sampleFrames ; s > 0 ; s-- )
    {

        if(*cutoff !=cut0 || *resonance !=res0)
        {
        cut0 = *cutoff;
        res0 = *resonance;

        cut_in=*cutoff;
        if(cut_in>0.9f )cut_in=0.9f;
        if(cut_in<0.0f)cut_in=0.0f;

        float q=*resonance*0.88f;
		diff= 1.f - q;
        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        f =fv_tbl[fk0]+(fract_fk0*(fv_tbl[fk0+1]-fv_tbl[fk0]));

        float float_rk0=q*10000.0f;
        int rk0=int(float_rk0);
        if(f>rv_tbl[rk0] )f=rv_tbl[rk0];

        }


        low += f * band;
		high = *in - low - band * diff;
		band += high * f;

        //*out=(*mix*(band-*in))+*in;//(mix*(b-a))+a;
        *out=band;

		in++;
		cutoff++;
		resonance++;
        out++;
        //mix++;
	}
}
void AllFilters::sub_process_26( int bufferOffset, int sampleFrames )
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
void AllFilters::sub_process_27( int bufferOffset, int sampleFrames )
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
void AllFilters::sub_process_28( int bufferOffset, int sampleFrames )
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
void AllFilters::sub_process_29( int bufferOffset, int sampleFrames )
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
void AllFilters::sub_process_30( int bufferOffset, int sampleFrames )
{
//synthi

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
        if(*resonance !=res0)
        {
        res0=*resonance;
        float cv=res0*10.0f;
        int iv=int(cv);
        float fcv=cv-float(iv);
        cor_vol=cvol[iv]+(fcv*(cvol[iv+1]-cvol[iv]));
        }

        float cut_in=*cutoff;
        if(cut_in>1.054f)cut_in=1.054f;
        if(cut_in<0.0f)cut_in=0.0f;

        float ci=cut_in*10.0f;
        int ic=int(ci);
        float fc=ci-float(ic);

        float cor_cut=corr[ic]+(fc*(corr[ic+1]-corr[ic]));
        float cor_cut2=corr2[ic]+(fc*(corr2[ic+1]-corr2[ic]));
        float cor_res=corr3[ic]+(fc*(corr3[ic+1]-corr3[ic]));
        cut_in+=(*sv*(cor_cut2-cor_cut))+cor_cut;

        if(cut_in<0.0f)cut_in=0.0f;

        cut2=cut_in;

        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        k0 =ofk0_tbl[fk0]+(fract_fk0*(ofk0_tbl[fk0+1]-ofk0_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =oflo1_tbl[fk0]+(fract_fk0*(oflo1_tbl[fk0+1]-oflo1_tbl[fk0]));
        r0=res0*scale0*cor_res*cr;
        }


        float upA=(*in- delay_inA)/2.0f;

        float m=*sv-cut2;
        if(m<0.0f)m=0.0f;

                   for (int i=1; i<2+1; i++)
                   {
                    up_overA[i]=iirU_2((upA*i)+delay_inA);

                    float co=(1.079f*(y4-y3)+y3);
                    x0=up_overA[i]-r0*co;
                    x0=ftanhx(x0,1.687f,res0,m);

                    y0=x0*p0+oldx0*p0-k0*y0;
                    y1=y0*p0+oldy0*p0-k0*y1;
                    y2=y1*p0+oldy1*p0-k0*y2;
                    y3=y2*p0+oldy2*p0-k0*y3;
                    y4=y3*p0+oldy3*p0-k0*y4;

                    over[i]=iirD_2(y3); //i

                    oldx0=x0;
                    oldy0=y0;
                    oldy1=y1;
                    oldy2=y2;
                    oldy3=y3;
                    }


        delay_inA=*in;


		*out=over[1]*cor_vol;

		in++;
		cutoff++;
		resonance++;
        out++;
        //level++;
        sv++;
        //mix++;

    }
}
void AllFilters::sub_process_31( int bufferOffset, int sampleFrames )
{
//vcs3*

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
        if(*resonance !=res0)
        {
        res0=*resonance;
        float cv=res0*10.0f;
        int iv=int(cv);
        float fcv=cv-float(iv);
        cor_vol=cvol[iv]+(fcv*(cvol[iv+1]-cvol[iv]));
        }

        float cut_in=*cutoff;
        if(cut_in>1.054f)cut_in=1.054f;
        if(cut_in<0.0f)cut_in=0.0f;

        float ci=cut_in*10.0f;
        int ic=int(ci);
        float fc=ci-float(ic);

        float cor_cut=corr[ic]+(fc*(corr[ic+1]-corr[ic]));
        float cor_cut2=corr2[ic]+(fc*(corr2[ic+1]-corr2[ic]));
        float cor_res=corr3[ic]+(fc*(corr3[ic+1]-corr3[ic]));
        cut_in+=(*sv*(cor_cut2-cor_cut))+cor_cut;

        if(cut_in<0.0f)cut_in=0.0f;

        cut2=cut_in;

        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);
        k0 =ofk0_tbl[fk0]+(fract_fk0*(ofk0_tbl[fk0+1]-ofk0_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =oflo1_tbl[fk0]+(fract_fk0*(oflo1_tbl[fk0+1]-oflo1_tbl[fk0]));

        r0=res0*scale0*cor_res;//1.012*0.05*0.95

        }


        float upA=(*in- delay_inA)/2.0f;
        float m=*sv-cut2;
        if(m<0.0f)m=0.0f;

                   for (int i=1; i<2+1; i++)
                   {
                    up_overA[i]=iirU_2((upA*i)+delay_inA);

                    x0=up_overA[i]-r0*y3;
                    x0=ftanhx(x0,3.38f,res0,m);

                    y0=x0*p0+oldx0*p0-k0*y0;
                    y1=y0*p0+oldy0*p0-k0*y1;
                    y2=y1*p0+oldy1*p0-k0*y2;
                    y3=y2*p0+oldy2*p0-k0*y3;
                    //y4=y3*p0+oldy3*p0-k0*y4;

                    over[i]=iirD_2(y2); //i

                    oldx0=x0;
                    oldy0=y0;
                    oldy1=y1;
                    oldy2=y2;
                    //oldy3=y3;
                    }


        delay_inA=*in;


		*out=over[1]*cor_vol;

		in++;
		cutoff++;
		resonance++;
        out++;
        //level++;
        sv++;
        //mix++;

    }
}
void AllFilters::sub_process_32( int bufferOffset, int sampleFrames )
{
//Ladder soft over // not used

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
        if(cutL>1.05f)cutL=1.05f;
        if(cutL<0.0f)cutL=0.0f;
        float float_volt=cutL*10000.0f;
        int int_volt=int(float_volt);
        float fract_volt=float_volt-float(int_volt);
        kacr =kacro_tbl[int_volt]+(fract_volt*(kacro_tbl[int_volt+1]-kacro_tbl[int_volt]));
        k2vg =k2vgo_tbl[int_volt]+(fract_volt*(k2vgo_tbl[int_volt+1]-k2vgo_tbl[int_volt]));
        //kacr =kacr_tbl[int_volt]+(fract_volt*(kacr_tbl[int_volt+1]-kacr_tbl[int_volt]));
        //k2vg =k2vg_tbl[int_volt]+(fract_volt*(k2vg_tbl[int_volt+1]-k2vg_tbl[int_volt]));
        }

        float upA=(*in - delay_inA)/2.0f;

                   for (int i=1; i<2+1; i++)
                   {

                   up_overA[i]=iirU_2((upA*i)+delay_inA);


                    ay1  = az1 + k2vg * ( (up_overA[i] - 4.0f*resL*amf*kacr) - az1 );
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
                    over[i]=iirD_2(tmp);
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
void AllFilters::sub_process_33( int bufferOffset, int sampleFrames )
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
void AllFilters::sub_process_34( int bufferOffset, int sampleFrames )
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
void AllFilters::sub_process_35( int bufferOffset, int sampleFrames )
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

void AllFilters::subProcess_synthiq(int bufferOffset, int sampleFrames)
{
//synthi

	float *in = bufferOffset + pinIn.getBuffer();
	float *cutoff = bufferOffset + pinCutoff.getBuffer();
	float *resonance = bufferOffset + pinRes.getBuffer();
	float *out	= bufferOffset + pinOut.getBuffer();
	//float *level = bufferOffset + pinLevel.getBuffer();
	float *sv = bufferOffset + pinSVFPoles.getBuffer();
	//float *mix = bufferOffset + pinMixIn.getBuffer();




	while (--sampleFrames >= 0) {

        if( *cutoff !=cut0 || *resonance !=res0 )
        {

        cut0=*cutoff;

        if(*resonance !=res0 )
        {
        res0=*resonance;
        float cv=res0*10.0f;
        int iv=int(cv);
        float fcv=cv-float(iv);
        cor_vol=cvol[iv]+(fcv*(cvol[iv+1]-cvol[iv]));
        }

        float cut_in=*cutoff;
        if(cut_in>1.054f)cut_in=1.054f;
        if(cut_in<0.0f)cut_in=0.0f;

        float ci=cut_in*10.0f;
        int ic=int(ci);
        float fc=ci-float(ic);

        float cor_cut=corr4[ic]+(fc*(corr4[ic+1]-corr4[ic]));
        float cor_cut2=corr5[ic]+(fc*(corr5[ic+1]-corr5[ic]));
        cut_in+=(*sv*(cor_cut2-cor_cut))+cor_cut;

        if(cut_in<0.0f)cut_in=0.0f;

        cut2=cut_in;

        c=2.0972f-cut_in;
        //c=2.18f-cut_in;
        c*=6.0f;

        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);

        k0 =ofk04_tbl[fk0]+(fract_fk0*(ofk04_tbl[fk0+1]-ofk04_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =oflo04_tbl[fk0]+(fract_fk0*(oflo04_tbl[fk0+1]-oflo04_tbl[fk0]));

        r0=res0*0.024035f*scale0;//1.012*(0.05*0.95)*0.5

        }

        //c=*mix-cut2;
        //c*=6.0f;
        //float l=1.0f;
        float ti=*in;
        double upA=(ti- delay_inA)/4.0f;
        //float m=1.0f-cut2;
        float m=*sv-cut2;
        if(m<0.0f)m=0.0f;

                   for (int i=1; i<4+1; i++)
                   {
                    up_overA[i]=iirU_4((upA*i)+delay_inA);

                    float co=(0.7753f*(y4-y3)+y3);
                    x0=up_overA[i]-(c*r0)*co;
                    x0=ftanhx(x0,4.0f,res0,m);

                    y0=x0*p0+oldx0*p0-k0*y0;
                    y1=y0*p0+oldy0*p0-k0*y1;
                    y2=y1*p0+oldy1*p0-k0*y2;
                    y3=y2*p0+oldy2*p0-k0*y3;
                    y4=y3*p0+oldy3*p0-k0*y4;

                    //y2=y2-(res1*(y2*y2*y2)/6.0f);
                    over[i]=iirD_4(y3); //i

                    oldx0=x0;
                    oldy0=y0;
                    oldy1=y1;
                    oldy2=y2;
                    oldy3=y3;
                    }


        delay_inA=ti;

		*out=over[1]*cor_vol;



		in++;
		cutoff++;
		resonance++;
        out++;
        //level++;
        sv++;
        //mix++;

    }
}
void AllFilters::subProcess_30q(int bufferOffset, int sampleFrames)
{
//vcs3 30*

	float *in = bufferOffset + pinIn.getBuffer();
	float *cutoff = bufferOffset + pinCutoff.getBuffer();
	float *resonance = bufferOffset + pinRes.getBuffer();
	float *out	= bufferOffset + pinOut.getBuffer();
	float *sv = bufferOffset + pinSVFPoles.getBuffer();


	while (--sampleFrames >= 0) {

        if( *cutoff !=cut0 || *resonance !=res0 )
        {

        cut0=*cutoff;

        if(*resonance !=res0 )
        {
        res0=*resonance;
        float cv=res0*10.0f;
        int iv=int(cv);
        float fcv=cv-float(iv);
        cor_vol=cvol[iv]+(fcv*(cvol[iv+1]-cvol[iv]));
        }

        float cut_in=*cutoff;
        if(cut_in>1.054f)cut_in=1.054f;
        if(cut_in<0.0f)cut_in=0.0f;

        float ci=cut_in*10.0f;
        int ic=int(ci);
        float fc=ci-float(ic);

        float cor_cut=corr4[ic]+(fc*(corr4[ic+1]-corr4[ic]));
        float cor_cut2=corr5[ic]+(fc*(corr5[ic+1]-corr5[ic]));
        cut_in+=(*sv*(cor_cut2-cor_cut))+cor_cut;

        if(cut_in<0.0f)cut_in=0.0f;

        cut2=cut_in;

        c=2.18f-cut_in;
        c*=6.0f;

        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);

        k0 =ofk04_tbl[fk0]+(fract_fk0*(ofk04_tbl[fk0+1]-ofk04_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =oflo04_tbl[fk0]+(fract_fk0*(oflo04_tbl[fk0+1]-oflo04_tbl[fk0]));

        r0=res0*0.024035f*scale0;//1.012*(0.05*0.95)*0.5

        }

        //c=*mix-cut2;
        //c*=6.0f;
        //float l=1.0f;

        float ti=*in;
        double upA=(ti- delay_inA)/4.0f;
        //float m=1.0f-cut2;
        float m=*sv-cut2;
        if(m<0.0f)m=0.0f;

                   for (int i=1; i<4+1; i++)
                   {
                    up_overA[i]=iirU_4((upA*i)+delay_inA);

                    x0=up_overA[i]-(c*r0)*y3;
                    x0=ftanhx(x0,7.5f,res0,m);

                    y0=x0*p0+oldx0*p0-k0*y0;
                    y1=y0*p0+oldy0*p0-k0*y1;
                    y2=y1*p0+oldy1*p0-k0*y2;
                    y3=y2*p0+oldy2*p0-k0*y3;
                    y4=y3*p0+oldy3*p0-k0*y4;

                    //y2=y2-(res1*(y2*y2*y2)/6.0f);
                    over[i]=iirD_4(y4); //i

                    oldx0=x0;
                    oldy0=y0;
                    oldy1=y1;
                    oldy2=y2;
                    oldy3=y3;
                    }


        delay_inA=ti;

		*out=over[1]*cor_vol;


		in++;
		cutoff++;
		resonance++;
        out++;

        sv++;


    }
}
void AllFilters::subProcess_24q(int bufferOffset, int sampleFrames)
{
//vcs3 24*

	float *in = bufferOffset + pinIn.getBuffer();
	float *cutoff = bufferOffset + pinCutoff.getBuffer();
	float *resonance = bufferOffset + pinRes.getBuffer();
	float *out	= bufferOffset + pinOut.getBuffer();

	float *sv = bufferOffset + pinSVFPoles.getBuffer();


	while (--sampleFrames >= 0) {

        if( *cutoff !=cut0 || *resonance !=res0 )
        {

        cut0=*cutoff;

        if(*resonance !=res0 )
        {
        res0=*resonance;
        float cv=res0*10.0f;
        int iv=int(cv);
        float fcv=cv-float(iv);
        cor_vol=cvol[iv]+(fcv*(cvol[iv+1]-cvol[iv]));
        }

        float cut_in=*cutoff;
        if(cut_in>1.054f)cut_in=1.054f;
        if(cut_in<0.0f)cut_in=0.0f;

        float ci=cut_in*10.0f;
        int ic=int(ci);
        float fc=ci-float(ic);

        float cor_cut=corr4[ic]+(fc*(corr4[ic+1]-corr4[ic]));
        float cor_cut2=corr5[ic]+(fc*(corr5[ic+1]-corr5[ic]));
        cut_in+=(*sv*(cor_cut2-cor_cut))+cor_cut;

        if(cut_in<0.0f)cut_in=0.0f;

        cut2=cut_in;

        c=2.18f-cut_in;
        c*=6.0f;

        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);

        k0 =ofk04_tbl[fk0]+(fract_fk0*(ofk04_tbl[fk0+1]-ofk04_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =oflo04_tbl[fk0]+(fract_fk0*(oflo04_tbl[fk0+1]-oflo04_tbl[fk0]));

        r0=res0*0.024035f*scale0;//1.012*(0.05*0.95)*0.5


        }

        //c=*mix-cut2;
        //c*=6.0f;
        //float l=1.0f;

        float ti=*in;
        double upA=(ti- delay_inA)/4.0f;
        //float m=1.0f-cut2;
        float m=*sv-cut2;
        if(m<0.0f)m=0.0f;

                   for (int i=1; i<4+1; i++)
                   {
                    up_overA[i]=iirU_4((upA*i)+delay_inA);

                    x0=up_overA[i]-(c*r0)*y3;
                    x0=ftanhx(x0,5.0f,res0,m);

                    y0=x0*p0+oldx0*p0-k0*y0;
                    y1=y0*p0+oldy0*p0-k0*y1;
                    y2=y1*p0+oldy1*p0-k0*y2;
                    y3=y2*p0+oldy2*p0-k0*y3;
                    //y4=y3*p0+oldy3*p0-k0*y4;

                    //y2=y2-(res1*(y2*y2*y2)/6.0f);
                    over[i]=iirD_4(y3); //i

                    oldx0=x0;
                    oldy0=y0;
                    oldy1=y1;
                    oldy2=y2;
                    //oldy3=y3;
                    }


        delay_inA=ti;

		*out=over[1]*cor_vol;


		in++;
		cutoff++;
		resonance++;
        out++;

        sv++;


    }
}
void AllFilters::subProcess_18q(int bufferOffset, int sampleFrames)
{
//vcs3q*

	float *in = bufferOffset + pinIn.getBuffer();
	float *cutoff = bufferOffset + pinCutoff.getBuffer();
	float *resonance = bufferOffset + pinRes.getBuffer();
	float *out	= bufferOffset + pinOut.getBuffer();
	float *sv = bufferOffset + pinSVFPoles.getBuffer();


	while (--sampleFrames >= 0) {

        if( *cutoff !=cut0 || *resonance !=res0 )
        {

        cut0=*cutoff;

        if(*resonance !=res0 )
        {
        res0=*resonance;
        float cv=res0*10.0f;
        int iv=int(cv);
        float fcv=cv-float(iv);
        cor_vol=cvol[iv]+(fcv*(cvol[iv+1]-cvol[iv]));
        }

        float cut_in=*cutoff;
        if(cut_in>1.054f)cut_in=1.054f;
        if(cut_in<0.0f)cut_in=0.0f;

        float ci=cut_in*10.0f;
        int ic=int(ci);
        float fc=ci-float(ic);

        float cor_cut=corr4[ic]+(fc*(corr4[ic+1]-corr4[ic]));
        float cor_cut2=corr5[ic]+(fc*(corr5[ic+1]-corr5[ic]));
        cut_in+=(*sv*(cor_cut2-cor_cut))+cor_cut;

        if(cut_in<0.0f)cut_in=0.0f;

        cut2=cut_in;

        c=2.18f-cut_in;
        c*=6.0f;

        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);

        k0 =ofk04_tbl[fk0]+(fract_fk0*(ofk04_tbl[fk0+1]-ofk04_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =oflo04_tbl[fk0]+(fract_fk0*(oflo04_tbl[fk0+1]-oflo04_tbl[fk0]));

        r0=res0*0.024035f*scale0;//1.012*(0.05*0.95)*0.5

        }

        //c=*mix-cut2;
        //c*=6.0f;
        //float l=1.0f;
        float ti=*in;
        double upA=(ti- delay_inA)/4.0f;
        float m=*sv-cut2;
        //float m=1.0f-cut2;
        if(m<0.0f)m=0.0f;

                   for (int i=1; i<4+1; i++)
                   {
                    up_overA[i]=iirU_4((upA*i)+delay_inA);

                    x0=up_overA[i]-(c*r0)*y3;
                    x0=ftanhx(x0,3.75f,res0,m);

                    y0=x0*p0+oldx0*p0-k0*y0;
                    y1=y0*p0+oldy0*p0-k0*y1;
                    y2=y1*p0+oldy1*p0-k0*y2;
                    y3=y2*p0+oldy2*p0-k0*y3;

                    //y2=y2-(res1*(y2*y2*y2)/6.0f);
                    over[i]=iirD_4(y2); //i

                    oldx0=x0;
                    oldy0=y0;
                    oldy1=y1;
                    oldy2=y2;

                    }


        delay_inA=ti;

		*out=over[1]*cor_vol;


		in++;
		cutoff++;
		resonance++;
        out++;
        sv++;


    }
}
void AllFilters::subProcess_12q(int bufferOffset, int sampleFrames)
{
//vcs3 12*
	float *in = bufferOffset + pinIn.getBuffer();
	float *cutoff = bufferOffset + pinCutoff.getBuffer();
	float *resonance = bufferOffset + pinRes.getBuffer();
	float *out	= bufferOffset + pinOut.getBuffer();
	float *sv = bufferOffset + pinSVFPoles.getBuffer();


	while (--sampleFrames >= 0) {

        if( *cutoff !=cut0 || *resonance !=res0 )
        {

        cut0=*cutoff;

        if(*resonance !=res0 )
        {
        res0=*resonance;
        float cv=res0*10.0f;
        int iv=int(cv);
        float fcv=cv-float(iv);
        cor_vol=cvol[iv]+(fcv*(cvol[iv+1]-cvol[iv]));
        }

        float cut_in=*cutoff;
        if(cut_in>1.054f)cut_in=1.054f;
        if(cut_in<0.0f)cut_in=0.0f;

        float ci=cut_in*10.0f;
        int ic=int(ci);
        float fc=ci-float(ic);

        float cor_cut=corr4[ic]+(fc*(corr4[ic+1]-corr4[ic]));
        float cor_cut2=corr5[ic]+(fc*(corr5[ic+1]-corr5[ic]));
        cut_in+=(*sv*(cor_cut2-cor_cut))+cor_cut;

        if(cut_in<0.0f)cut_in=0.0f;

        cut2=cut_in;

        c=2.18f-cut_in;
        c*=6.0f;

        float float_fk0=cut_in*10000.0f;
        int fk0=int(float_fk0);
        float fract_fk0=float_fk0-float(fk0);

        k0 =ofk04_tbl[fk0]+(fract_fk0*(ofk04_tbl[fk0+1]-ofk04_tbl[fk0]));
        p0=(k0+1.0f)*0.5f;
        scale0 =oflo04_tbl[fk0]+(fract_fk0*(oflo04_tbl[fk0+1]-oflo04_tbl[fk0]));

        r0=res0*0.024035f*scale0;//1.012*(0.05*0.95)*0.5

        }

        //c=*mix-cut2;
        //c*=6.0f;
        //float l=1.0f;
        float ti=*in;
        double upA=(ti- delay_inA)/4.0f;
        //float m=1.0f-cut2;
        float m=*sv-cut2;
        if(m<0.0f)m=0.0f;

                   for (int i=1; i<4+1; i++)
                   {
                    up_overA[i]=iirU_4((upA*i)+delay_inA);

                    x0=up_overA[i]-(c*r0)*y3;
                    //x0=ftanhx(x0,2.625f,res0,m);
                    x0=ftanhx(x0,3.75f,res0,m);

                    y0=x0*p0+oldx0*p0-k0*y0;
                    y1=y0*p0+oldy0*p0-k0*y1;
                    y2=y1*p0+oldy1*p0-k0*y2;
                    y3=y2*p0+oldy2*p0-k0*y3;
                    //y4=y3*p0+oldy3*p0-k0*y4;

                    over[i]=iirD_4(y1); //i

                    oldx0=x0;
                    oldy0=y0;
                    oldy1=y1;
                    oldy2=y2;
                    //oldy3=y3;
                    }


        delay_inA=ti;

		*out=over[1]*cor_vol;


		in++;
		cutoff++;
		resonance++;
        out++;

        sv++;


    }
}
void AllFilters::sub_process_0( int bufferOffset, int sampleFrames )
{

	float* out	= bufferOffset + pinOut.getBuffer();

    for( int s = sampleFrames ; s > 0 ; s-- )
    {

        *out=0.0f;
        out++;

	}
}

void AllFilters::onSetPins(void)
{



        if( pinVoiceReset.isUpdated() && pinVoiceReset>0.0f)
        {
        ResetVoice();
        //cut0=-100.f;
        //res0=-100.f;
        //run=false;
        }


        if((!pinIn.isStreaming() && pinIn == 0.f) || res_mode==5 )
        {
        ResetVoice();
        //cut0=-100.f;
        //res0=-100.f;
        run=false;
        pinOut.setStreaming(false);
        SET_PROCESS(&AllFilters::sub_process_0);
        setSleep(true);

        }
        else
        {
            if(res_mode.isUpdated() || run==false)
            {

                run=true;

                ResetVoice();
                //cut0=-100.f;
                //res0=-100.f;
                pinOut.setStreaming(true);
                if(res_mode==0)SET_PROCESS(&AllFilters::sub_process_19);//SvLP
                if(res_mode==1)SET_PROCESS(&AllFilters::sub_process_20);//SvHp
                if(res_mode==2)SET_PROCESS(&AllFilters::sub_process_21);//SvBp
                if(res_mode==3)SET_PROCESS(&AllFilters::sub_process_22);//SvBr
                if(res_mode==4)SET_PROCESS(&AllFilters::sub_process_35);//MgLnr
                //5=off->Korg
                if(res_mode==6)SET_PROCESS(&AllFilters::sub_process_34);//MgL
                if(res_mode==7)SET_PROCESS(&AllFilters::sub_process_33);//MgL*
                if(res_mode==8)SET_PROCESS(&AllFilters::sub_process_5);//Mg*
                if(res_mode==9)SET_PROCESS(&AllFilters::sub_process_6);//MgS*

                if(res_mode==10)SET_PROCESS(&AllFilters::sub_process_2);//Hq
                if(res_mode==11)SET_PROCESS(&AllFilters::sub_process_3);//HqM
                if(res_mode==12)SET_PROCESS(&AllFilters::sub_process_4);//LoM

                if(res_mode==13)SET_PROCESS(&AllFilters::sub_process_9);//Lp18
                if(res_mode==14)SET_PROCESS(&AllFilters::sub_process_7);//Lp18*
                if(res_mode==15)SET_PROCESS(&AllFilters::sub_process_8);//Lp18d*
                if(res_mode==16)SET_PROCESS(&AllFilters::sub_process_10);//Bp18a
                if(res_mode==17)SET_PROCESS(&AllFilters::sub_process_11);//Bp18b

                if(res_mode==18)SET_PROCESS(&AllFilters::sub_process_12);//LP12
                if(res_mode==19)SET_PROCESS(&AllFilters::sub_process_13);//Bp12a
                if(res_mode==20)SET_PROCESS(&AllFilters::sub_process_14);//Bp12b

                if(res_mode==21)SET_PROCESS(&AllFilters::sub_process_15);//Lp6
                if(res_mode==22)SET_PROCESS(&AllFilters::sub_process_16);//Bp6a
                if(res_mode==23)SET_PROCESS(&AllFilters::sub_process_17);//Bp6b

                if(res_mode==24)SET_PROCESS(&AllFilters::sub_process_18);//Hp

                if(res_mode==25)SET_PROCESS(&AllFilters::sub_process_23);//SvLP CS
                if(res_mode==26)SET_PROCESS(&AllFilters::sub_process_24);//SvHP CS
                if(res_mode==27)SET_PROCESS(&AllFilters::sub_process_25);//SvHP CS

                if(res_mode==28)SET_PROCESS(&AllFilters::sub_process_26);//SvLPR
                if(res_mode==29)SET_PROCESS(&AllFilters::sub_process_27);//SvHpR
                if(res_mode==30)SET_PROCESS(&AllFilters::sub_process_28);//SvBpR
                if(res_mode==31)SET_PROCESS(&AllFilters::sub_process_29);//SvBrR

                if(res_mode==32)SET_PROCESS(&AllFilters::sub_process_30);//Synthi*
                if(res_mode==33)SET_PROCESS(&AllFilters::sub_process_31);//Vcs3*
                if(res_mode==34)SET_PROCESS(&AllFilters::subProcess_synthiq);
                if(res_mode==35)SET_PROCESS(&AllFilters::subProcess_30q);
                if(res_mode==36)SET_PROCESS(&AllFilters::subProcess_24q);
                if(res_mode==37)SET_PROCESS(&AllFilters::subProcess_18q);
                if(res_mode==38)SET_PROCESS(&AllFilters::subProcess_12q);

                if(res_mode==39)SET_PROCESS(&AllFilters::sub_process_1);//NoRes



                setSleep(false);
                }
        }

}

float AllFilters::ftanh(float sample)
{
    if(sample>1.0f)sample=1.0f;
    if(sample<-1.0f)sample=-1.0f;
    float x=(sample+1.0f)*10000.0f;
    int int_x=int(x);
    float fract_x=x-float(int_x);
    float x_tanh =tanh_tbl[int_x]+(fract_x*(tanh_tbl[int_x+1]-tanh_tbl[int_x]));
    return x_tanh;
}
float AllFilters::ftanh2(float sample)
{
    if(sample>1.5f)sample=1.5f;
    if(sample<-1.5f)sample=-1.5f;
    float x=((sample/1.5f)+1.0f)*10000.0f;
    int int_x=int(x);
    float fract_x=x-float(int_x);
    float x_tanh =tanh_tbl[int_x]+(fract_x*(tanh_tbl[int_x+1]-tanh_tbl[int_x]));
    return x_tanh*1.5f;
}

float AllFilters::ftanh4(float sample)
{
    if(sample>4.0f)sample=4.0f;
    if(sample<-4.0f)sample=-4.0f;
    float x=((sample/4.0f)+1.0f)*10000.0f;
    int int_x=int(x);
    float fract_x=x-float(int_x);
    float x_tanh =tanh_tbl[int_x]+(fract_x*(tanh_tbl[int_x+1]-tanh_tbl[int_x]));
    return x_tanh*4.0f;
}

float AllFilters::ftanhs(float sample)
{
    float in=sample;
    if(sample>1.0f)sample=1.0f;
    if(sample<-1.0f)sample=-1.0f;
    //float half=sample*0.5f;
    float x=(sample+1.0f)*10000.0f;
    int int_x=int(x);
    float fract_x=x-float(int_x);
    float x_tanh =tanh_tbl[int_x]+(fract_x*(tanh_tbl[int_x+1]-tanh_tbl[int_x]));
    float mul=sample;
    if(sample<0.0f)mul*=-1.0f;
    float soft=(mul*(x_tanh-in))+in;
    return soft;
}

float AllFilters::ftanhs2(float sample,float dist)
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

float AllFilters::ftanhx(float sample, float r, float dist, float color)
{
    if(sample>r)sample=r;
    if(sample<-r)sample=-r;
    float div=sample/r;
    float x=(div+1.0f)*10000.0f;
    int int_x=int(x);
    float fract_x=x-float(int_x);
    float x_tanh =tanh_tbl[int_x]+(fract_x*(tanh_tbl[int_x+1]-tanh_tbl[int_x]));
    //float saw=x_tanh-(div*0.1f);
    float saw=div;
    if(saw>0.0f)saw=0.0f;
    saw+= x_tanh;
    x_tanh=(color*(saw-x_tanh))+x_tanh;//harmoniques paires vers paires et impaires
    float mul=div;
    if(div<0.0f)mul*=-1.0f;
    float sft=(mul*(x_tanh-div))+div;//distortion en fonction du niveau , div=sans dist
    //x_tanh=(soft*(x_tanh-sft))+sft; //dosage de la distortion en fonction du niveau
    //float s=(dist*(x_tanh-div))+div;//dosage de la distortion
    float s=(dist*(sft-div))+div;//dosage de la distortion
    return s*r;
}

//x2
float AllFilters::iirU_2(float NewSample)
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

float AllFilters::iirD_2(float NewSample)
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
//*4 6 order
float AllFilters::iirU_4(double NewSample)
{
    int n;
    //shift the old samples
    for(n=NCoef; n>0; n--) {
       xA4[n] = xA4[n-1];
       yA4[n] = yA4[n-1];
    }
    //Calculate the new output
    xA4[0] = NewSample;
    yA4[0] = ACoef4[0] * xA4[0];

    for(n=1; n<=NCoef; n++)yA4[0] += ACoef4[n] * xA4[n] - BCoef4[n] * yA4[n];

    return float(yA4[0]);
}
float AllFilters::iirD_4(float NewSample)
{
    int n;
    //shift the old samples
    for(n=NCoef; n>0; n--) {
       xD4[n] = xD4[n-1];
       yD4[n] = yD4[n-1];
    }
    //Calculate the new output
    xD4[0] = NewSample;
    yD4[0] = ACoef4[0] * xD4[0];

    for(n=1; n<=NCoef; n++)yD4[0] += ACoef4[n] * xD4[n] - BCoef4[n] * yD4[n];

    return float(yD4[0]);
}
void AllFilters::ResetVoice()
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

        yA4[n]=0; //output samples
        xA4[n]=0; //input samples
        yD4[n]=0; //output samples
        xD4[n]=0; //input samples
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

