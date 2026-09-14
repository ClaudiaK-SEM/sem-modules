#include "../se_sdk3/mp_sdk_audio.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <cstdint>

#define NCoef 8

/*
PolyBLEP Waveform generator ported from the Jesusonic code by Tale
http://www.taletn.com/reaper/mono_synth/

Permission has been granted to release this port under the WDL/IPlug license:

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

Original Jeff's files: "Oscillator" from SDK3 of SynthEdit 1.5552, 08/08/2024.
http://www.synthedit.com/files/se_sdk.zip.
The original code is copied at the file bottom.

Claudia Kalensky (KX77FREE) 09 2024 modifications list:
- Added Voltage to Hertz and Sine tables.
- Added PW and Phase Modulation, same amount of SE oscillator.
- Added static mode process.
- Added four gate modes: Free, Gate, GateVoice, Voice.
- Added C.Kalensky's oversampling IIR filters (built with Winfilter), all oscillator cycles are oversampled up x16.

- Added Internal Sync and External Sync processes.
To use Internal Sync you must connect and use SyncPitch input with same values of the pitch of master oscillator.
External Sync is less quality vs SE oscillator in oversampled container, that's works but I need to find a good
way to calculate the Master pitch based on the Sync input signal, I did that but the phase of Master oscillator is always changing
and that increases the aliasing noise...
So waiting I find something good I set the reset phase to 0 but that increases the aliasing noise vs the Internal Sync.
Internal Sync quality with x4 oversampling (96k*4=384k) is similar or better of SE oscillator in oversampled x8 container
(48k*8=384k, SE uses always a multiple of 44.1k or 48k when you set its oversampling factor).
Internal Sync uses less CPU use because only the wave cycle are oversampled and only one down filter is used.
With the SE PolyMod KX and the SE Modular ME plugins, I use only Internal Sync, never the External Sync with this module.
I limited the oversampling factor to 16 and when the samplerate is equal or upper than 176.4k, this one is divised by two
to optimize the plugin CPU use ^^.

- Added low pass filter to smooth or more the waveforms.
- Added pitch voltage limit choice: 10, 10.3, 10.5.
- Added Pink and White noise processes from Jeff's HD oscillator, no changes.
- Fully tested on SE PolyMod KX and SE Modular ME vst3 plugins.

The modified sources are available on :
http://kx77free.free.fr/data/Some-KX77FREE-sem-source-code.zip

The releases are done with Code::Blocks and MSV 2022, Stable SDK, this module is included inside the
X64 CK KX77FREE Tools.sem

*/

using namespace gmpi;

const double TWO_PI = 2 * M_PI;

template<typename T>
inline T square_number(const T &x) {
	return x * x;
}

template<typename T>
inline int64_t bitwiseOrZero(const T &t) {
	return static_cast<int64_t>(t) | 0;
}

// Adapted from "Phaseshaping PolyBlepOsc Algorithms for Musical Sound
// Synthesis" by Jari Kleimola, Victor Lazzarini, Joseph Timoney, and Vesa
// Valimaki.
// http://www.acoustics.hut.fi/publications/papers/smc2010-phaseshaping/

inline double poly_blep(double t,double dt ){
        // 0 <= t < 1
        if (t < dt) {
            t /= dt;
            return t+t - t*t - 1.0;
        }
        // -1 < t < 0
        else if (t > 1.0 - dt) {
            t = (t - 1.0) / dt;
            return t*t + t+t + 1.0;
        }
        // 0 otherwise
        else return 0.0;
}

inline double blep(double t, double dt) {
	if (t < dt) {
		return -square_number(t / dt - 1);
	}
	else if (t > 1 - dt) {
		return square_number((t - 1) / dt + 1);
	}
	else {
		return 0;
	}
}

// Derived from blep().
inline double blamp(double t, double dt) {
	if (t < dt) {
		t = t / dt - 1;
		return -1 / 3.0 * square_number(t) * t;
	}
	else if (t > 1 - dt) {
		t = (t - 1) / dt + 1;
		return 1 / 3.0 * square_number(t) * t;
	}
	else {
		return 0;
	}
}

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


class PolyBlepOsc : public MpBase2
{
	AudioInPin pinPitch;
	AudioInPin pinPulseWidth;
	AudioInPin pinPhaseMod;
	AudioInPin pinSyncIn;
	AudioInPin pinSyncPitch;
	IntInPin pinWaveform;
	IntInPin pinSyncMode;
	IntInPin pinOverFactor;
	BoolInPin pinBypass;
	IntInPin pinResetMode;
	AudioOutPin pinAudioOut;
	FloatInPin pinVoiceActive;
    IntInPin pinVoltLimit;
    AudioInPin pinCutOff;



	/*enum Waveform {
		SINE,
		COSINE,
		TRIANGLE,
		SQUARE,
		RECTANGLE,
		SAWTOOTH,
		RAMP,
		MODIFIED_TRIANGLE,
		MODIFIED_SQUARE,
		HALF_WAVE_RECTIFIED_SINE,
		FULL_WAVE_RECTIFIED_SINE,
		TRIANGULAR_PULSE,
		TRAPEZOID_FIXED,
		TRAPEZOID_VARIABLE
	};*/

	//Waveform waveform = SQUARE;
	//double sampleRate = 44100;
	//double amplitude = 1; // Frequency dependent gain [0.0..1.0]
	//double pulseWidth = 0.5; // [0.0..1.0]


    int zeroSamplesCounter = 0;//need for sleep process like OscillatorHD

    //int init=1;

    // to be sure that the if() pin tests  works !!
    int m_Waveform=-1;// -1 doesn't exist in the enum lists
    int m_SyncMode=-1;
    int m_OverFactor=-1;
    bool m_Bypass=true;
    int m_ResetMode=-1;
    int m_VoltLimit=-1;



	unsigned int random;
	// pink noise stuff
	float buf0=0.0f;
	float buf1=0.0f;
	float buf2=0.0f;
	float buf3=0.0f;
	float buf4=0.0f;
	float buf5=0.0f;


	double FreqInc;
	double FreqIncSync;
	double t = 0.0; // The current phase [0.0..1.0) of the PolyBlepOsc.
	double Mt = 0.0; // The current phase [0.0..1.0) of the PolyBlepOsc.
	//double Ft = 0.0; // The current phase [0.0..1.0) of the PolyBlepOsc.


    //for testing sync pitch
    //int countF=0;
    //double Mfreq;
    //double OverSpl;


    int loopI;
    int loopJ;
    int OverFactor;
    double sampleRate;
    //double OneHertz;

    float prev_tuning=-100.f;
    float prev_tuning2=-100.f;


    double volt_tbl[20608];
    double sine_tbl[10508];
    float p6_tbl[10508];

    //double saw_tbl[10008];

 	//for shared look up tables
 	//double* volt_tbl = {};
	//double* sine_tbl = {};
	//float* p6_tbl = {};


    float mcut=-100.f;

    float fp6;
    float yp6;


    double delay_inI;
    double delay_inJ;
    double msync;
    float VoltLimit=10.33333333333f;

    double AyD8[NCoef+1]; //output samples
    double AxD8[NCoef+1]; //input samples
    double AyD4[NCoef+1]; //output samples
    double AxD4[NCoef+1];
    double AyD2[NCoef+1]; //output samples
    double AxD2[NCoef+1];

    double ByD8[NCoef+1]; //output samples
    double BxD8[NCoef+1]; //input samples
    double ByD4[NCoef+1]; //output samples
    double BxD4[NCoef+1];
    double ByD2[NCoef+1]; //output samples
    double BxD2[NCoef+1];

    double overA[16];
    double overB[16];

    double AyU8[NCoef+1]; //output samples
    double AxU8[NCoef+1]; //input samples
    double AyU4[NCoef+1]; //output samples
    double AxU4[NCoef+1];
    double AyU2[NCoef+1]; //output samples
    double AxU2[NCoef+1];

    double ByU8[NCoef+1]; //output samples
    double BxU8[NCoef+1]; //input samples
    double ByU4[NCoef+1]; //output samples
    double BxU4[NCoef+1];
    double ByU2[NCoef+1]; //output samples
    double BxU2[NCoef+1];

    double UoverA[16];
    double UoverB[16];



public:
	PolyBlepOsc()
	{
		initializePin( pinPitch );
		initializePin( pinPulseWidth );
        initializePin( pinPhaseMod );
		initializePin( pinSyncIn );
		initializePin( pinSyncPitch );
        initializePin( pinWaveform );
		initializePin( pinSyncMode );
		initializePin( pinOverFactor );
		initializePin( pinBypass );
		initializePin( pinResetMode );
		initializePin( pinAudioOut );
		initializePin( pinVoiceActive );
		initializePin( pinVoltLimit );
        initializePin( pinCutOff );
	}

	int32_t MP_STDCALL open() override
	{

		Volt();


        /*//test of shared lookup tables, actually for testing SE 1.5566
        int32_t needInitialize = 0;
        getHost()->allocateSharedMemory( L"CK_osc_volt", (void**)&volt_tbl, getSampleRate(), 20608*sizeof(double), needInitialize );

        int32_t needInitialize1 = 0;
        getHost()->allocateSharedMemory( L"CK_osc_sine", (void**)&sine_tbl, getSampleRate(), 10508*sizeof(double), needInitialize1 );

        int32_t needInitialize2 = 0;
        getHost()->allocateSharedMemory( L"CK_LP6_coef", (void**)&p6_tbl, getSampleRate(), 10508*sizeof(float), needInitialize2 );

        //only one test to optimize the look up tables build
        if( needInitialize )
        {
        sampleRate=getSampleRate();
        double OneHertz =1.0 /sampleRate; //f = 1 hertz
            for(int i=-10000;i<10600;i++)//10.5 volts = 10500 + 100 secure spl
            {
            double volt=double(i)*0.001f;
            volt_tbl[i+10000]=(440.0*pow(2.0,volt-5.0)*OneHertz);
            }

            for(int i=0;i<10501;i++)// no pb for sine_tbl to calculate up to 1.05
            {
            float volt=float(i*0.001);
            float f=440.0f*powf(2.0f,(volt-5.0f));
            float maxf=f/sampleRate;
            p6_tbl[i] = expf(-TWO_PI * maxf);
            double x=double(i*0.0001);
            sine_tbl[i]=sin(TWO_PI * x);
            //saw_tbl[i]=x;
            }
        }*/

		ResetF();

		return MpBase2::open();
	}

    void Volt()
    {
    sampleRate=getSampleRate();
    double OneHertz =1.0 /sampleRate; //f = 1 hertz

        for(int i=-10000;i<10600;i++)//10.5 volts = 10500 + 100 secure spl
        {
        double volt=double(i)*0.001f;
        volt_tbl[i+10000]=(440.0*pow(2.0,volt-5.0)*OneHertz);
        }

        for(int i=0;i<10501;i++)// no pb for sine_tbl to calculate up to 1.05
        {
        float volt=float(i*0.001);
        float f=440.0f*powf(2.0f,(volt-5.0f));
        float maxf=f/sampleRate;
        p6_tbl[i] = expf(-TWO_PI * maxf);
        double x=double(i*0.0001);
        sine_tbl[i]=sin(TWO_PI * x);
        //saw_tbl[i]=x;
        }
    }

    void ResetF()
    {

        msync=0;
        delay_inI=0;
        delay_inJ=0;
        yp6=0.0f;

        int n;
        //shift the old samples
        for(n=NCoef; n>0; n--)
        {
        AxD2[n] = 0.;
        AyD2[n] = 0.;
        AxD4[n] = 0.;
        AyD4[n] = 0.;
        AxD8[n] = 0.;
        AyD8[n] = 0.;
        BxD2[n] = 0.;
        ByD2[n] = 0.;
        BxD4[n] = 0.;
        ByD4[n] = 0.;
        BxD8[n] = 0.;
        ByD8[n] = 0.;

        AxU2[n] = 0.;
        AyU2[n] = 0.;
        AxU4[n] = 0.;
        AyU4[n] = 0.;
        AxU8[n] = 0.;
        AyU8[n] = 0.;
        BxU2[n] = 0.;
        ByU2[n] = 0.;
        BxU4[n] = 0.;
        ByU4[n] = 0.;
        BxU8[n] = 0.;
        ByU8[n] = 0.;

       }
    }

void subProcessSine(int sampleFrames)
{
		// get pointers to in/output buffers.


		auto pitch = getBuffer(pinPitch);
		auto pulseWidth = getBuffer(pinPulseWidth);
		auto phaseMod = getBuffer(pinPhaseMod);
		auto syncIn = getBuffer(pinSyncIn);
		auto syncPitch = getBuffer(pinSyncPitch);
		auto audioOut = getBuffer(pinAudioOut);
		auto cutoff = getBuffer(pinCutOff);

		for (int s = sampleFrames; s > 0; --s)
		{


                if (*pitch != prev_tuning && *pitch>=-1.0f )
                {
                prev_tuning = *pitch;
                float tuning2=*pitch*10.0f;
                if(tuning2>=VoltLimit)tuning2=VoltLimit;

                double fvoltconv=10000.0f+(tuning2*1000.0f);
                int voltconv=int(fvoltconv);
                double fract_voltconv=fvoltconv-double(voltconv);
                FreqInc =volt_tbl[voltconv]+(fract_voltconv*(volt_tbl[voltconv+1]-volt_tbl[voltconv]));
                FreqInc/=OverFactor;
                }

                 if (*syncPitch != prev_tuning2 && *syncPitch>=-1.0f && pinSyncMode==1)
                {
                prev_tuning2 = *syncPitch;
                float tuning2=*syncPitch*10.0f;
                if(tuning2>=VoltLimit)tuning2=VoltLimit;

                double fvoltconv=10000.0f+(tuning2*1000.0f);
                int voltconv=int(fvoltconv);
                double fract_voltconv=fvoltconv-double(voltconv);
                FreqIncSync =volt_tbl[voltconv]+(fract_voltconv*(volt_tbl[voltconv+1]-volt_tbl[voltconv]));
                FreqIncSync/=OverFactor;
                }

                if(*cutoff !=mcut)
                {
                mcut = *cutoff;
                float cut_in=*cutoff;
                if(cut_in>1.05f )cut_in=1.05f;
                if(cut_in<0.0f)cut_in=0.0f;

                float float_fk0=cut_in*10000.0f;
                int fk0=int(float_fk0);
                float fract_fk0=float_fk0-float(fk0);
                fp6 =p6_tbl[fk0]+(fract_fk0*(p6_tbl[fk0+1]-p6_tbl[fk0]));
                }


                float PM=-(*phaseMod*0.5f);
                if(PM<-0.5f)PM=-0.5f;
                if(PM>0.5f)PM=0.5f;


                if(pinResetMode==1 || pinResetMode==2)
                {
                    if (msync<=0.0f && *syncIn>0.0f)Mt= t = 0;
                }


                for (int j=1; j<2+1; j++)
                {


                    for (int i=1; i<loopI+1; i++)
                    {


                        if(pinSyncMode==1 && Mt < FreqIncSync)t = Mt*(FreqInc/FreqIncSync);


                        double Pht=t+PM;
                        Pht -= bitwiseOrZero(Pht);

                        double s=Pht*10000.0f;
                        int ints=int(s);
                        double fracts=s-double(ints);
                        double y = (sine_tbl[ints]+(fracts*(sine_tbl[ints+1]-sine_tbl[ints])))*0.5;
                        //double y = (std::sin(TWO_PI *Pht))*0.5;


                        t += FreqInc;
                        t -= bitwiseOrZero(t);
                        Mt += FreqIncSync;
                        Mt -= bitwiseOrZero(Mt);


                    overA[i]=y;
                    if(OverFactor==4)overA[i]=AiirD_2(y);
                    if(OverFactor==8)overA[i]=AiirD_4(y);
                    if(OverFactor==16)overA[i]=AiirD_8(y);
                    }
                overB[j]=BiirD_2(overA[1]);
                }

               //*audioOut=overB[1];
                yp6 = float(overB[1]) + fp6 * ( yp6 - float(overB[1]) );
                *audioOut = yp6;

                msync=*syncIn;

			// Increment buffer pointers.
			++pitch;
			++pulseWidth;
			++phaseMod;
			++syncIn;
			++syncPitch;
			++audioOut;
            ++cutoff;
		}
}
void subProcessSaw(int sampleFrames)
{
		// get pointers to in/output buffers.
		auto pitch = getBuffer(pinPitch);
		auto pulseWidth = getBuffer(pinPulseWidth);
		auto phaseMod = getBuffer(pinPhaseMod);
		auto syncIn = getBuffer(pinSyncIn);
		auto syncPitch = getBuffer(pinSyncPitch);
		auto audioOut = getBuffer(pinAudioOut);
		auto cutoff = getBuffer(pinCutOff);

		for (int s = sampleFrames; s > 0; --s)
		{


                if (*pitch != prev_tuning && *pitch>=-1.0f )
                {
                prev_tuning = *pitch;
                float tuning2=*pitch*10.0f;
                if(tuning2>=VoltLimit)tuning2=VoltLimit;

                double fvoltconv=10000.0f+(tuning2*1000.0f);
                int voltconv=int(fvoltconv);
                double fract_voltconv=fvoltconv-double(voltconv);
                FreqInc =volt_tbl[voltconv]+(fract_voltconv*(volt_tbl[voltconv+1]-volt_tbl[voltconv]));
                FreqInc/=OverFactor;
                }

                if (*syncPitch != prev_tuning2 && *syncPitch>=-1.0f && pinSyncMode==1)
                {
                prev_tuning2 = *syncPitch;
                float tuning2=*syncPitch*10.0f;
                if(tuning2>=VoltLimit)tuning2=VoltLimit;

                double fvoltconv=10000.0f+(tuning2*1000.0f);
                int voltconv=int(fvoltconv);
                double fract_voltconv=fvoltconv-double(voltconv);
                FreqIncSync =volt_tbl[voltconv]+(fract_voltconv*(volt_tbl[voltconv+1]-volt_tbl[voltconv]));
                FreqIncSync/=OverFactor;
                }

                if(*cutoff !=mcut)
                {
                mcut = *cutoff;
                float cut_in=*cutoff;
                if(cut_in>1.05f )cut_in=1.05f;
                if(cut_in<0.0f)cut_in=0.0f;

                float float_fk0=cut_in*10000.0f;
                int fk0=int(float_fk0);
                float fract_fk0=float_fk0-float(fk0);
                fp6 =p6_tbl[fk0]+(fract_fk0*(p6_tbl[fk0+1]-p6_tbl[fk0]));
                }


                float PM=-(*phaseMod*0.5f);
                if(PM<-0.5f)PM=-0.5f;
                if(PM>0.5f)PM=0.5f;

                if(pinResetMode==1 || pinResetMode==2)
                {
                    if (msync<=0.0f && *syncIn>0.0f)Mt= t = 0;
                }

                for (int j=1; j<2+1; j++)
                {

                    for (int i=1; i<loopI+1; i++)
                    {

                        if(pinSyncMode ==1 && Mt < FreqIncSync)t = Mt*(FreqInc/FreqIncSync);

                        double Pht=t+PM;
                        Pht -= bitwiseOrZero(Pht);

                        double _t = Pht + 0.5;
                        _t -= bitwiseOrZero(_t);

                        //double y = 2 * _t - 1;
                        //y -= blep(_t, FreqInc);
                        //y*=0.5;

                         //double y = 2 * _t - 1;
                        //y -= blep(_t, FreqInc);
                        //y*=-0.5;

                        double y = 1 - 2 * _t;
                        y += blep(_t, FreqInc);
                        y*=0.5;


                        t += FreqInc;
                        Mt += FreqIncSync;
                        t -= bitwiseOrZero(t);
                        Mt -= bitwiseOrZero(Mt);


                    overA[i]=y;
                    if(OverFactor==4)overA[i]=AiirD_2(y);
                    if(OverFactor==8)overA[i]=AiirD_4(y);
                    if(OverFactor==16)overA[i]=AiirD_8(y);
                    }
                overB[j]=BiirD_2(overA[1]);
                }


               //*audioOut=overB[1];
                yp6 = float(overB[1]) + fp6 * ( yp6 - float(overB[1]) );
                *audioOut = yp6;

                msync=*syncIn;


			// Increment buffer pointers.
			++pitch;
			++pulseWidth;
			++phaseMod;
			++syncIn;
			++syncPitch;
			++audioOut;
            ++cutoff;
		}
}
void subProcessRamp(int sampleFrames)
{
		// get pointers to in/output buffers.
		auto pitch = getBuffer(pinPitch);
		auto pulseWidth = getBuffer(pinPulseWidth);
		auto phaseMod = getBuffer(pinPhaseMod);
		auto syncIn = getBuffer(pinSyncIn);
		auto syncPitch = getBuffer(pinSyncPitch);
		auto audioOut = getBuffer(pinAudioOut);
		auto cutoff = getBuffer(pinCutOff);

		for (int s = sampleFrames; s > 0; --s)
		{

                if (*pitch != prev_tuning && *pitch>=-1.0f )
                {
                prev_tuning = *pitch;
                float tuning2=*pitch*10.0f;
                if(tuning2>=VoltLimit)tuning2=VoltLimit;

                double fvoltconv=10000.0f+(tuning2*1000.0f);
                int voltconv=int(fvoltconv);
                double fract_voltconv=fvoltconv-double(voltconv);
                FreqInc =volt_tbl[voltconv]+(fract_voltconv*(volt_tbl[voltconv+1]-volt_tbl[voltconv]));
                FreqInc/=OverFactor;
                }

                if (*syncPitch != prev_tuning2 && *syncPitch>=-1.0f && pinSyncMode==1)
                {
                prev_tuning2 = *syncPitch;
                float tuning2=*syncPitch*10.0f;
                if(tuning2>=VoltLimit)tuning2=VoltLimit;

                double fvoltconv=10000.0f+(tuning2*1000.0f);
                int voltconv=int(fvoltconv);
                double fract_voltconv=fvoltconv-double(voltconv);
                FreqIncSync =volt_tbl[voltconv]+(fract_voltconv*(volt_tbl[voltconv+1]-volt_tbl[voltconv]));
                FreqIncSync/=OverFactor;
                }

                if(*cutoff !=mcut)
                {
                mcut = *cutoff;
                float cut_in=*cutoff;
                if(cut_in>1.05f )cut_in=1.05f;
                if(cut_in<0.0f)cut_in=0.0f;

                float float_fk0=cut_in*10000.0f;
                int fk0=int(float_fk0);
                float fract_fk0=float_fk0-float(fk0);
                fp6 =p6_tbl[fk0]+(fract_fk0*(p6_tbl[fk0+1]-p6_tbl[fk0]));
                }

                float PM=-(*phaseMod*0.5f);
                if(PM<-0.5f)PM=-0.5f;
                if(PM>0.5f)PM=0.5f;

                if(pinResetMode==1 || pinResetMode==2)
                {
                    if (msync<=0.0f && *syncIn>0.0f)Mt= t = 0;
                }

                for (int j=1; j<2+1; j++)
                {

                    for (int i=1; i<loopI+1; i++)
                    {

                        if(pinSyncMode ==1 && Mt < FreqIncSync)t = Mt*(FreqInc/FreqIncSync);



                        double Pht=t+PM;
                        Pht -= bitwiseOrZero(Pht);

                        double _t = Pht + 0.5;
                        _t -= bitwiseOrZero(_t);

                        //double y = 2 * _t - 1;
                        //y -= blep(_t, FreqInc);
                        //y*=-0.5;

                        //double y = 2 * _t - 1;
                        //y -= blep(_t, FreqInc);
                        //y*=-0.5;

                        double y = 1 - 2 * _t;
                        y += blep(_t, FreqInc);
                        y*=-0.5;


                        t += FreqInc;
                        Mt += FreqIncSync;
                        t -= bitwiseOrZero(t);
                        Mt -= bitwiseOrZero(Mt);



                    overA[i]=y;
                    if(OverFactor==4)overA[i]=AiirD_2(y);
                    if(OverFactor==8)overA[i]=AiirD_4(y);
                    if(OverFactor==16)overA[i]=AiirD_8(y);
                    }
                overB[j]=BiirD_2(overA[1]);
                }

               //*audioOut=overB[1];
                yp6 = float(overB[1]) + fp6 * ( yp6 - float(overB[1]) );
                *audioOut = yp6;

                msync=*syncIn;


			// Increment buffer pointers.
			++pitch;
			++pulseWidth;
			++phaseMod;
			++syncIn;
			++syncPitch;
			++audioOut;
            ++cutoff;
		}
}
void subProcessTri(int sampleFrames)
{
		// get pointers to in/output buffers.
		auto pitch = getBuffer(pinPitch);
		auto pulseWidth = getBuffer(pinPulseWidth);
		auto phaseMod = getBuffer(pinPhaseMod);
		auto syncIn = getBuffer(pinSyncIn);
		auto syncPitch = getBuffer(pinSyncPitch);
		auto audioOut = getBuffer(pinAudioOut);
		auto cutoff = getBuffer(pinCutOff);

		for (int s = sampleFrames; s > 0; --s)
		{

                if (*pitch != prev_tuning && *pitch>=-1.0f )
                {
                prev_tuning = *pitch;
                float tuning2=*pitch*10.0f;
                if(tuning2>=VoltLimit)tuning2=VoltLimit;

                double fvoltconv=10000.0f+(tuning2*1000.0f);
                int voltconv=int(fvoltconv);
                double fract_voltconv=fvoltconv-double(voltconv);
                FreqInc =volt_tbl[voltconv]+(fract_voltconv*(volt_tbl[voltconv+1]-volt_tbl[voltconv]));
                FreqInc/=OverFactor;
                }

                if (*syncPitch != prev_tuning2 && *syncPitch>=-1.0f && pinSyncMode==1)
                {
                prev_tuning2 = *syncPitch;
                float tuning2=*syncPitch*10.0f;
                if(tuning2>=VoltLimit)tuning2=VoltLimit;

                double fvoltconv=10000.0f+(tuning2*1000.0f);
                int voltconv=int(fvoltconv);
                double fract_voltconv=fvoltconv-double(voltconv);
                FreqIncSync =volt_tbl[voltconv]+(fract_voltconv*(volt_tbl[voltconv+1]-volt_tbl[voltconv]));
                FreqIncSync/=OverFactor;
                }

                if(*cutoff !=mcut)
                {
                mcut = *cutoff;
                float cut_in=*cutoff;
                if(cut_in>1.05f )cut_in=1.05f;
                if(cut_in<0.0f)cut_in=0.0f;

                float float_fk0=cut_in*10000.0f;
                int fk0=int(float_fk0);
                float fract_fk0=float_fk0-float(fk0);
                fp6 =p6_tbl[fk0]+(fract_fk0*(p6_tbl[fk0+1]-p6_tbl[fk0]));
                }

                float PM=(-*phaseMod+1.0f)*0.5f;
                if(PM<0.0f)PM=0.0f;
                if(PM>1.0f)PM=1.0f;

                if(pinResetMode==1 || pinResetMode==2)
                {
                    if (msync<=0.0f && *syncIn>0.0f)Mt= t = 0;
                }

                for (int j=1; j<2+1; j++)
                {

                    for (int i=1; i<loopI+1; i++)
                    {

                        if(pinSyncMode ==1 && Mt < FreqIncSync)t = Mt*(FreqInc/FreqIncSync);

                        double Pht=t+PM;
                        Pht -= bitwiseOrZero(Pht);

                        double t1 = Pht + 0.25;
                        t1 -= bitwiseOrZero(t1);

                        double t2 = Pht + 0.75;
                        t2 -= bitwiseOrZero(t2);

                        double y = Pht * 4;

                        if (y >= 3)
                        {
                        y -= 4;
                        }
                        else if (y > 1)
                        {
                        y = 2 - y;
                        }

                        y += 4 * FreqInc * (blamp(t1, FreqInc) - blamp(t2, FreqInc));
                        y*=-0.5;

                        t += FreqInc;
                        Mt += FreqIncSync;
                        t -= bitwiseOrZero(t);
                        Mt -= bitwiseOrZero(Mt);



                    overA[i]=y;
                    if(OverFactor==4)overA[i]=AiirD_2(y);
                    if(OverFactor==8)overA[i]=AiirD_4(y);
                    if(OverFactor==16)overA[i]=AiirD_8(y);
                    }
                overB[j]=BiirD_2(overA[1]);
                }

               //*audioOut=overB[1];
                yp6 = float(overB[1]) + fp6 * ( yp6 - float(overB[1]) );
                *audioOut = yp6;

                msync=*syncIn;


			// Increment buffer pointers.
			++pitch;
			++pulseWidth;
			++phaseMod;
			++syncIn;
			++syncPitch;
			++audioOut;
            ++cutoff;
		}
}
void subProcessPulse(int sampleFrames)
{
		// get pointers to in/output buffers.
		auto pitch = getBuffer(pinPitch);
		auto pulseWidth = getBuffer(pinPulseWidth);
		auto phaseMod = getBuffer(pinPhaseMod);
		auto syncIn = getBuffer(pinSyncIn);
		auto syncPitch = getBuffer(pinSyncPitch);
		auto audioOut = getBuffer(pinAudioOut);
		auto cutoff = getBuffer(pinCutOff);

		for (int s = sampleFrames; s > 0; --s)
		{

                if (*pitch != prev_tuning && *pitch>=-1.0f )
                {
                prev_tuning = *pitch;
                float tuning2=*pitch*10.0f;
                if(tuning2>=VoltLimit)tuning2=VoltLimit;

                double fvoltconv=10000.0f+(tuning2*1000.0f);
                int voltconv=int(fvoltconv);
                double fract_voltconv=fvoltconv-double(voltconv);
                FreqInc =volt_tbl[voltconv]+(fract_voltconv*(volt_tbl[voltconv+1]-volt_tbl[voltconv]));
                FreqInc/=OverFactor;
                }

                if (*syncPitch != prev_tuning2 && *syncPitch>=-1.0f && pinSyncMode==1)
                {
                prev_tuning2 = *syncPitch;
                float tuning2=*syncPitch*10.0f;
                if(tuning2>=VoltLimit)tuning2=VoltLimit;

                double fvoltconv=10000.0f+(tuning2*1000.0f);
                int voltconv=int(fvoltconv);
                double fract_voltconv=fvoltconv-double(voltconv);
                FreqIncSync =volt_tbl[voltconv]+(fract_voltconv*(volt_tbl[voltconv+1]-volt_tbl[voltconv]));
                FreqIncSync/=OverFactor;
                }

                if(*cutoff !=mcut)
                {
                mcut = *cutoff;
                float cut_in=*cutoff;
                if(cut_in>1.05f )cut_in=1.05f;
                if(cut_in<0.0f)cut_in=0.0f;

                float float_fk0=cut_in*10000.0f;
                int fk0=int(float_fk0);
                float fract_fk0=float_fk0-float(fk0);
                fp6 =p6_tbl[fk0]+(fract_fk0*(p6_tbl[fk0+1]-p6_tbl[fk0]));
                }

                float PW=0.5f-(*pulseWidth*0.5f);
                if(PW<0.002f)PW=0.002f;
                if(PW>0.998f)PW=0.998f;

                float PM=(-*phaseMod+1.0f)*0.5f;
                if(PM<0.0f)PM=0.0f;
                if(PM>1.0f)PM=1.0f;


                if(pinResetMode==1 || pinResetMode==2)
                {
                    if (msync<=0.0f && *syncIn>0.0f)Mt= t = 0;
                }

                for (int j=1; j<2+1; j++)
                {

                    for (int i=1; i<loopI+1; i++)
                    {

                        if(pinSyncMode ==1 && Mt < FreqIncSync)t = Mt*(FreqInc/FreqIncSync);

                        double Pht=t+PM;
                        Pht -= bitwiseOrZero(Pht);

                        double t2 = Pht + 1 - PW;
                        t2 -= bitwiseOrZero(t2);

                        double y = -2 * PW;
                        if (Pht < PW) {
                        y += 2;
                        }

                        y += blep(Pht, FreqInc) - blep(t2, FreqInc);
                        y*=-0.5;

                        t += FreqInc;
                        Mt += FreqIncSync;
                        t -= bitwiseOrZero(t);
                        Mt -= bitwiseOrZero(Mt);

                    overA[i]=y;
                    if(OverFactor==4)overA[i]=AiirD_2(y);
                    if(OverFactor==8)overA[i]=AiirD_4(y);
                    if(OverFactor==16)overA[i]=AiirD_8(y);
                    }
                overB[j]=BiirD_2(overA[1]);
                }

               //*audioOut=overB[1];
                yp6 = float(overB[1]) + fp6 * ( yp6 - float(overB[1]) );
                *audioOut = yp6;

                msync=*syncIn;


			// Increment buffer pointers.
			++pitch;
			++pulseWidth;
			++phaseMod;
			++syncIn;
			++syncPitch;
			++audioOut;
            ++cutoff;
		}
}

void subProcessSineSyncExt(int sampleFrames)
{
		// get pointers to in/output buffers.


		auto pitch = getBuffer(pinPitch);
		auto pulseWidth = getBuffer(pinPulseWidth);
		auto phaseMod = getBuffer(pinPhaseMod);
		auto syncIn = getBuffer(pinSyncIn);
		auto syncPitch = getBuffer(pinSyncPitch);
		auto audioOut = getBuffer(pinAudioOut);
		auto cutoff = getBuffer(pinCutOff);

		for (int s = sampleFrames; s > 0; --s)
		{


                if (*pitch != prev_tuning && *pitch>=-1.0f )
                {
                prev_tuning = *pitch;
                float tuning2=*pitch*10.0f;
                if(tuning2>=VoltLimit)tuning2=VoltLimit;

                double fvoltconv=10000.0f+(tuning2*1000.0f);
                int voltconv=int(fvoltconv);
                double fract_voltconv=fvoltconv-double(voltconv);
                FreqInc =volt_tbl[voltconv]+(fract_voltconv*(volt_tbl[voltconv+1]-volt_tbl[voltconv]));
                FreqInc/=OverFactor;
                }

                if(*cutoff !=mcut)
                {
                mcut = *cutoff;
                float cut_in=*cutoff;
                if(cut_in>1.05f )cut_in=1.05f;
                if(cut_in<0.0f)cut_in=0.0f;

                float float_fk0=cut_in*10000.0f;
                int fk0=int(float_fk0);
                float fract_fk0=float_fk0-float(fk0);
                fp6 =p6_tbl[fk0]+(fract_fk0*(p6_tbl[fk0+1]-p6_tbl[fk0]));
                }


                float PM=-(*phaseMod*0.5f);
                if(PM<-0.5f)PM=-0.5f;
                if(PM>0.5f)PM=0.5f;

                //(*syncIn - delay_inJ)/2;
                // It's to calculate the value between current slp (*in) and N-1 spl (delay_in) and this one is divised by sample factor.
                //Just to avoid the Up filter works on same values because it's IIR filter
                double SyncJ=(*syncIn - delay_inJ)/2;
                for (int j=1; j<2+1; j++)
                {

                double upJ=(SyncJ*j)+delay_inJ;//increment the sample in value with each part of (spl-spl N-1)
                UoverB[j]=BiirU_2(upJ);// set the spl in IIR filter buffer

                    double SyncI=(UoverB[1] - delay_inI)/OverFactor/2;
                    for (int i=1; i<loopI+1; i++)
                    {

                        double upI=(SyncI*i)+delay_inI;
                        UoverA[i]=upI;
                        if(OverFactor==4)UoverA[i]=AiirU_2(upI);
                        if(OverFactor==8)UoverA[i]=AiirU_4(upI);
                        if(OverFactor==16)UoverA[i]=AiirU_8(upI);

                        if (msync<0.0f && UoverA[i]>=0.0)t = 0;
                        /*//no signifiant SNR Alias gain when we calculate the sync pitch like internal sync
                        if(msync<0.0f && UoverA[i]>=0.0)
                        {
                        Mfreq=((OverSpl/countF)*(1.0/OverSpl)); // sync in pitch
                        t = Mt*(FreqInc/Mfreq);
                        countF=0;
                        }*/

                        double Pht=t+PM;
                        Pht -= bitwiseOrZero(Pht);

                        double s=Pht*10000.0f;
                        int ints=int(s);
                        double fracts=s-double(ints);
                        double y = (sine_tbl[ints]+(fracts*(sine_tbl[ints+1]-sine_tbl[ints])))*0.5;


                        t += FreqInc;
                        t -= bitwiseOrZero(t);

                        //Mt += Mfreq;
                        //Mt -= bitwiseOrZero(Mt);
                        //countF++;



                    msync=UoverA[i];
                    overA[i]=y;
                    if(OverFactor==4)overA[i]=AiirD_2(y);
                    if(OverFactor==8)overA[i]=AiirD_4(y);
                    if(OverFactor==16)overA[i]=AiirD_8(y);
                    }
                overB[j]=BiirD_2(overA[1]);
                delay_inI=UoverB[1];//spl N-1
                }

               //*audioOut=overB[1];
                yp6 = float(overB[1]) + fp6 * ( yp6 - float(overB[1]) );
                *audioOut = yp6;
                delay_inJ=*syncIn;//spl N-1


			// Increment buffer pointers.
			++pitch;
			++pulseWidth;
			++phaseMod;
			++syncIn;
			++syncPitch;
			++audioOut;
            ++cutoff;
		}
}
void subProcessSawSyncExt(int sampleFrames)
{
		// get pointers to in/output buffers.
		auto pitch = getBuffer(pinPitch);
		auto pulseWidth = getBuffer(pinPulseWidth);
		auto phaseMod = getBuffer(pinPhaseMod);
		auto syncIn = getBuffer(pinSyncIn);
		auto syncPitch = getBuffer(pinSyncPitch);
		auto audioOut = getBuffer(pinAudioOut);
		auto cutoff = getBuffer(pinCutOff);

		for (int s = sampleFrames; s > 0; --s)
		{


                if (*pitch != prev_tuning && *pitch>=-1.0f )
                {
                prev_tuning = *pitch;
                float tuning2=*pitch*10.0f;
                if(tuning2>=VoltLimit)tuning2=VoltLimit;

                double fvoltconv=10000.0f+(tuning2*1000.0f);
                int voltconv=int(fvoltconv);
                double fract_voltconv=fvoltconv-double(voltconv);
                FreqInc =volt_tbl[voltconv]+(fract_voltconv*(volt_tbl[voltconv+1]-volt_tbl[voltconv]));
                FreqInc/=OverFactor;
                }

                if(*cutoff !=mcut)
                {
                mcut = *cutoff;
                float cut_in=*cutoff;
                if(cut_in>1.05f )cut_in=1.05f;
                if(cut_in<0.0f)cut_in=0.0f;

                float float_fk0=cut_in*10000.0f;
                int fk0=int(float_fk0);
                float fract_fk0=float_fk0-float(fk0);
                fp6 =p6_tbl[fk0]+(fract_fk0*(p6_tbl[fk0+1]-p6_tbl[fk0]));
                }


                float PM=-(*phaseMod*0.5f);
                if(PM<-0.5f)PM=-0.5f;
                if(PM>0.5f)PM=0.5f;

                //(*syncIn - delay_inJ)/2;
                // It's to calculate the value between current slp (*in) and N-1 spl (delay_in) and this one is divised by sample factor.
                //Just to avoid the Up filter works on same values because it's IIR filter
                double SyncJ=(*syncIn - delay_inJ)/2;
                for (int j=1; j<2+1; j++)
                {

                double upJ=(SyncJ*j)+delay_inJ;//increment the sample in value with each part of (spl-spl N-1)
                UoverB[j]=BiirU_2(upJ);// set the spl in IIR filter buffer

                    double SyncI=(UoverB[1] - delay_inI)/OverFactor/2;
                    for (int i=1; i<loopI+1; i++)
                    {

                        double upI=(SyncI*i)+delay_inI;
                        UoverA[i]=upI;
                        if(OverFactor==4)UoverA[i]=AiirU_2(upI);
                        if(OverFactor==8)UoverA[i]=AiirU_4(upI);
                        if(OverFactor==16)UoverA[i]=AiirU_8(upI);


                        if (msync<0.0f && UoverA[i]>=0.0)t = 0;
                        /*//no signifiant SNR Alias gain when we calculate the sync pitch like internal sync
                        if(msync<0.0f && UoverA[i]>=0.0)
                        {
                        Mfreq=((OverSpl/countF)*(1.0/OverSpl)); // sync in pitch
                        t = Mt*(FreqInc/Mfreq);
                        countF=0;
                        }*/

                        double Pht=t+PM;
                        Pht -= bitwiseOrZero(Pht);

                        double _t = Pht + 0.5;
                        _t -= bitwiseOrZero(_t);

                        //double y = 2 * _t - 1;
                        //y -= blep(_t, FreqInc);
                        //y*=-0.5;

                        double y = 1 - 2 * _t;
                        y += blep(_t, FreqInc);
                        y*=0.5;


                        t += FreqInc;
                        t -= bitwiseOrZero(t);


                        //Mt += Mfreq;
                        //Mt -= bitwiseOrZero(Mt);
                        //countF++;

                    msync=UoverA[i];
                    overA[i]=y;
                    if(OverFactor==4)overA[i]=AiirD_2(y);
                    if(OverFactor==8)overA[i]=AiirD_4(y);
                    if(OverFactor==16)overA[i]=AiirD_8(y);
                    }
                overB[j]=BiirD_2(overA[1]);
                delay_inI=UoverB[1];//spl N-1
                }

               //*audioOut=overB[1];
                yp6 = float(overB[1]) + fp6 * ( yp6 - float(overB[1]) );
                *audioOut = yp6;
                delay_inJ=*syncIn;//spl N-1


			// Increment buffer pointers.
			++pitch;
			++pulseWidth;
			++phaseMod;
			++syncIn;
			++syncPitch;
			++audioOut;
            ++cutoff;
		}
}
void subProcessRampSyncExt(int sampleFrames)
{
		// get pointers to in/output buffers.
		auto pitch = getBuffer(pinPitch);
		auto pulseWidth = getBuffer(pinPulseWidth);
		auto phaseMod = getBuffer(pinPhaseMod);
		auto syncIn = getBuffer(pinSyncIn);
		auto syncPitch = getBuffer(pinSyncPitch);
		auto audioOut = getBuffer(pinAudioOut);
		auto cutoff = getBuffer(pinCutOff);

		for (int s = sampleFrames; s > 0; --s)
		{

                if (*pitch != prev_tuning && *pitch>=-1.0f )
                {
                prev_tuning = *pitch;
                float tuning2=*pitch*10.0f;
                if(tuning2>=VoltLimit)tuning2=VoltLimit;

                double fvoltconv=10000.0f+(tuning2*1000.0f);
                int voltconv=int(fvoltconv);
                double fract_voltconv=fvoltconv-double(voltconv);
                FreqInc =volt_tbl[voltconv]+(fract_voltconv*(volt_tbl[voltconv+1]-volt_tbl[voltconv]));
                FreqInc/=OverFactor;
                }

                if(*cutoff !=mcut)
                {
                mcut = *cutoff;
                float cut_in=*cutoff;
                if(cut_in>1.05f )cut_in=1.05f;
                if(cut_in<0.0f)cut_in=0.0f;

                float float_fk0=cut_in*10000.0f;
                int fk0=int(float_fk0);
                float fract_fk0=float_fk0-float(fk0);
                fp6 =p6_tbl[fk0]+(fract_fk0*(p6_tbl[fk0+1]-p6_tbl[fk0]));
                }

                float PM=-(*phaseMod*0.5f);
                if(PM<-0.5f)PM=-0.5f;
                if(PM>0.5f)PM=0.5f;

                //(*syncIn - delay_inJ)/2;
                // It's to calculate the value between current slp (*in) and N-1 spl (delay_in) and this one is divised by sample factor.
                //Just to avoid the Up filter works on same values because it's IIR filter
                double SyncJ=(*syncIn - delay_inJ)/2;
                for (int j=1; j<2+1; j++)
                {

                double upJ=(SyncJ*j)+delay_inJ;//increment the sample in value with each part of (spl-spl N-1)
                UoverB[j]=BiirU_2(upJ);// set the spl in IIR filter buffer

                    double SyncI=(UoverB[1] - delay_inI)/OverFactor/2;
                    for (int i=1; i<loopI+1; i++)
                    {

                        double upI=(SyncI*i)+delay_inI;
                        UoverA[i]=upI;
                        if(OverFactor==4)UoverA[i]=AiirU_2(upI);
                        if(OverFactor==8)UoverA[i]=AiirU_4(upI);
                        if(OverFactor==16)UoverA[i]=AiirU_8(upI);


                        if (msync<0.0f && UoverA[i]>=0.0)t = 0;
                        /*//no signifiant SNR Alias gain when we calculate the sync pitch like internal sync
                        if(msync<0.0f && UoverA[i]>=0.0)
                        {
                        Mfreq=((OverSpl/countF)*(1.0/OverSpl)); // sync in pitch
                        t = Mt*(FreqInc/Mfreq);
                        countF=0;
                        }*/

                        double Pht=t+PM;
                        Pht -= bitwiseOrZero(Pht);

                        double _t = Pht + 0.5;
                        _t -= bitwiseOrZero(_t);

                        //double y = 2 * _t - 1;
                        //y -= blep(_t, FreqInc);
                        //y*=0.5;

                        double y = 1 - 2 * _t;
                        y += blep(_t, FreqInc);
                        y*=-0.5;



                        t += FreqInc;
                        t -= bitwiseOrZero(t);

                        //Mt += Mfreq;
                        //Mt -= bitwiseOrZero(Mt);
                        //countF++;



                    msync=UoverA[i];
                    overA[i]=y;
                    if(OverFactor==4)overA[i]=AiirD_2(y);
                    if(OverFactor==8)overA[i]=AiirD_4(y);
                    if(OverFactor==16)overA[i]=AiirD_8(y);
                    }
                overB[j]=BiirD_2(overA[1]);
                delay_inI=UoverB[1];//spl N-1
                }

               //*audioOut=overB[1];
                yp6 = float(overB[1]) + fp6 * ( yp6 - float(overB[1]) );
                *audioOut = yp6;
                delay_inJ=*syncIn;//spl N-1

			// Increment buffer pointers.
			++pitch;
			++pulseWidth;
			++phaseMod;
			++syncIn;
			++syncPitch;
			++audioOut;
            ++cutoff;
		}
}
void subProcessTriSyncExt(int sampleFrames)
{
		// get pointers to in/output buffers.
		auto pitch = getBuffer(pinPitch);
		auto pulseWidth = getBuffer(pinPulseWidth);
		auto phaseMod = getBuffer(pinPhaseMod);
		auto syncIn = getBuffer(pinSyncIn);
		auto syncPitch = getBuffer(pinSyncPitch);
		auto audioOut = getBuffer(pinAudioOut);
		auto cutoff = getBuffer(pinCutOff);

		for (int s = sampleFrames; s > 0; --s)
		{

                if (*pitch != prev_tuning && *pitch>=-1.0f )
                {
                prev_tuning = *pitch;
                float tuning2=*pitch*10.0f;
                if(tuning2>=VoltLimit)tuning2=VoltLimit;

                double fvoltconv=10000.0f+(tuning2*1000.0f);
                int voltconv=int(fvoltconv);
                double fract_voltconv=fvoltconv-double(voltconv);
                FreqInc =volt_tbl[voltconv]+(fract_voltconv*(volt_tbl[voltconv+1]-volt_tbl[voltconv]));
                FreqInc/=OverFactor;
                }

                if(*cutoff !=mcut)
                {
                mcut = *cutoff;
                float cut_in=*cutoff;
                if(cut_in>1.05f )cut_in=1.05f;
                if(cut_in<0.0f)cut_in=0.0f;

                float float_fk0=cut_in*10000.0f;
                int fk0=int(float_fk0);
                float fract_fk0=float_fk0-float(fk0);
                fp6 =p6_tbl[fk0]+(fract_fk0*(p6_tbl[fk0+1]-p6_tbl[fk0]));
                }

                float PM=(-*phaseMod+1.0f)*0.5f;
                if(PM<0.0f)PM=0.0f;
                if(PM>1.0f)PM=1.0f;

                //(*syncIn - delay_inJ)/2;
                // It's to calculate the value between current slp (*in) and N-1 spl (delay_in) and this one is divised by sample factor.
                //Just to avoid the Up filter works on same values because it's IIR filter
                double SyncJ=(*syncIn - delay_inJ)/2;
                for (int j=1; j<2+1; j++)
                {

                double upJ=(SyncJ*j)+delay_inJ;//increment the sample in value with each part of (spl-spl N-1)
                UoverB[j]=BiirU_2(upJ);// set the spl in IIR filter buffer

                    double SyncI=(UoverB[1] - delay_inI)/OverFactor/2;
                    for (int i=1; i<loopI+1; i++)
                    {

                        double upI=(SyncI*i)+delay_inI;
                        UoverA[i]=upI;
                        if(OverFactor==4)UoverA[i]=AiirU_2(upI);
                        if(OverFactor==8)UoverA[i]=AiirU_4(upI);
                        if(OverFactor==16)UoverA[i]=AiirU_8(upI);


                        if (msync<0.0f && UoverA[i]>=0.0)t = 0;
                        /*//no signifiant SNR Alias gain when we calculate the sync pitch like internal sync
                        if(msync<0.0f && UoverA[i]>=0.0)
                        {
                        Mfreq=((OverSpl/countF)*(1.0/OverSpl)); // sync in pitch
                        t = Mt*(FreqInc/Mfreq);
                        countF=0;
                        }*/


                        double Pht=t+PM;
                        Pht -= bitwiseOrZero(Pht);

                        double t1 = Pht + 0.25;
                        t1 -= bitwiseOrZero(t1);

                        double t2 = Pht + 0.75;
                        t2 -= bitwiseOrZero(t2);

                        double y = Pht * 4;

                        if (y >= 3)
                        {
                        y -= 4;
                        }
                        else if (y > 1)
                        {
                        y = 2 - y;
                        }

                        y += 4 * FreqInc * (blamp(t1, FreqInc) - blamp(t2, FreqInc));
                        y*=-0.5;

                        t += FreqInc;
                        t -= bitwiseOrZero(t);

                        //Mt += Mfreq;
                        //Mt -= bitwiseOrZero(Mt);
                        //countF++;



                    msync=UoverA[i];
                    overA[i]=y;
                    if(OverFactor==4)overA[i]=AiirD_2(y);
                    if(OverFactor==8)overA[i]=AiirD_4(y);
                    if(OverFactor==16)overA[i]=AiirD_8(y);
                    }
                overB[j]=BiirD_2(overA[1]);
                delay_inI=UoverB[1];//spl N-1
                }

               //*audioOut=overB[1];
                yp6 = float(overB[1]) + fp6 * ( yp6 - float(overB[1]) );
                *audioOut = yp6;
                delay_inJ=*syncIn;//spl N-1


			// Increment buffer pointers.
			++pitch;
			++pulseWidth;
			++phaseMod;
			++syncIn;
			++syncPitch;
			++audioOut;
            ++cutoff;
		}
}
void subProcessPulseSyncExt(int sampleFrames)
{
		// get pointers to in/output buffers.
		auto pitch = getBuffer(pinPitch);
		auto pulseWidth = getBuffer(pinPulseWidth);
		auto phaseMod = getBuffer(pinPhaseMod);
		auto syncIn = getBuffer(pinSyncIn);
		auto syncPitch = getBuffer(pinSyncPitch);
		auto audioOut = getBuffer(pinAudioOut);
		auto cutoff = getBuffer(pinCutOff);

		for (int s = sampleFrames; s > 0; --s)
		{

                if (*pitch != prev_tuning && *pitch>=-1.0f )
                {
                prev_tuning = *pitch;
                float tuning2=*pitch*10.0f;
                if(tuning2>=VoltLimit)tuning2=VoltLimit;

                double fvoltconv=10000.0f+(tuning2*1000.0f);
                int voltconv=int(fvoltconv);
                double fract_voltconv=fvoltconv-double(voltconv);
                FreqInc =volt_tbl[voltconv]+(fract_voltconv*(volt_tbl[voltconv+1]-volt_tbl[voltconv]));
                FreqInc/=OverFactor;
                }


                if(*cutoff !=mcut)
                {
                mcut = *cutoff;
                float cut_in=*cutoff;
                if(cut_in>1.05f )cut_in=1.05f;
                if(cut_in<0.0f)cut_in=0.0f;

                float float_fk0=cut_in*10000.0f;
                int fk0=int(float_fk0);
                float fract_fk0=float_fk0-float(fk0);
                fp6 =p6_tbl[fk0]+(fract_fk0*(p6_tbl[fk0+1]-p6_tbl[fk0]));
                }

                float PW=0.5f-(*pulseWidth*0.5f);
                if(PW<0.002f)PW=0.002f;
                if(PW>0.998f)PW=0.998f;

                float PM=(-*phaseMod+1.0f)*0.5f;
                if(PM<0.0f)PM=0.0f;
                if(PM>1.0f)PM=1.0f;


                //(*syncIn - delay_inJ)/2;
                // It's to calculate the value between current slp (*in) and N-1 spl (delay_in) and this one is divised by sample factor.
                //Just to avoid the Up filter works on same values because it's IIR filter
                double SyncJ=(*syncIn - delay_inJ)/2;
                for (int j=1; j<2+1; j++)
                {

                double upJ=(SyncJ*j)+delay_inJ;//increment the sample in value with each part of (spl-spl N-1)
                UoverB[j]=BiirU_2(upJ);// set the spl in IIR filter buffer

                    double SyncI=(UoverB[1] - delay_inI)/OverFactor/2;
                    for (int i=1; i<loopI+1; i++)
                    {

                        double upI=(SyncI*i)+delay_inI;
                        UoverA[i]=upI;
                        if(OverFactor==4)UoverA[i]=AiirU_2(upI);
                        if(OverFactor==8)UoverA[i]=AiirU_4(upI);
                        if(OverFactor==16)UoverA[i]=AiirU_8(upI);


                        if (msync<0.0f && UoverA[i]>=0.0)t = 0;
                        /*//no signifiant SNR Alias gain when we calculate the sync pitch like internal sync
                        if(msync<0.0f && UoverA[i]>=0.0)
                        {
                        Mfreq=((OverSpl/countF)*(1.0/OverSpl)); // sync in pitch
                        t = Mt*(FreqInc/Mfreq);
                        countF=0;
                        }*/


                        double Pht=t+PM;
                        Pht -= bitwiseOrZero(Pht);

                        double t2 = Pht + 1 - PW;
                        t2 -= bitwiseOrZero(t2);

                        double y = -2 * PW;
                        if (Pht < PW) {
                        y += 2;
                        }

                        y += blep(Pht, FreqInc) - blep(t2, FreqInc);
                        y*=-0.5;

                        t += FreqInc;
                        t -= bitwiseOrZero(t);

                        //Mt += Mfreq;
                        //Mt -= bitwiseOrZero(Mt);
                        //countF++;

                    msync=UoverA[i];
                    overA[i]=y;
                    if(OverFactor==4)overA[i]=AiirD_2(y);
                    if(OverFactor==8)overA[i]=AiirD_4(y);
                    if(OverFactor==16)overA[i]=AiirD_8(y);
                    }
                overB[j]=BiirD_2(overA[1]);
                delay_inI=UoverB[1];//spl N-1
                }

               //*audioOut=overB[1];
                yp6 = float(overB[1]) + fp6 * ( yp6 - float(overB[1]) );
                *audioOut = yp6;
                delay_inJ=*syncIn;//spl N-1


			// Increment buffer pointers.
			++pitch;
			++pulseWidth;
			++phaseMod;
			++syncIn;
			++syncPitch;
			++audioOut;
            ++cutoff;
		}
}

void subProcessWhiteNoise( int sampleFrames )
{
    auto audioOut = getBuffer(pinAudioOut);

	unsigned int itemp;
	unsigned int idum = random;
	const unsigned int jflone = 0x3f800000; // see 'numerical recipies in c' pg 285
	const unsigned int jflmsk = 0x007fffff;

	for( int s = sampleFrames; s > 0; --s )
	{
		idum = idum * 1664525L + 1013904223L;// use mask to quickly convert integer to float between -0.5 and 0.5
		itemp = jflone | (jflmsk & idum);
		*audioOut++ = (*(float*)&itemp) - 1.5f;
	}

	random = idum; // store new random number
}

void subProcessPinkNoise( int sampleFrames )
{
    auto audioOut = getBuffer(pinAudioOut);

	unsigned int itemp;
	unsigned int idum = random;
	const unsigned int jflone = 0x3f800000; // see 'numerical recipies in c' pg 285
	const unsigned int jflmsk = 0x007fffff;

	for( int s = sampleFrames; s > 0; --s )
	{
		idum = idum * 1664525L + 1013904223L;
		// use mask to quickly convert integer to float between -0.5 and 0.5
		itemp = jflone | (jflmsk & idum);
		float white = (*(float*)&itemp) - 1.5f;

		// filtering white noise.
		buf0 = 0.997f * buf0 + 0.029591f * white;
		buf1 = 0.985f * buf1 + 0.032534f * white;
		buf2 = 0.950f * buf2 + 0.048056f * white;
		buf3 = 0.850f * buf3 + 0.090579f * white;
		buf4 = 0.620f * buf4 + 0.108990f * white;
		buf5 = 0.250f * buf5 + 0.255784f * white;
		*audioOut++ = buf0 + buf1 + buf2 + buf3 + buf4 + buf5;
	}

	random = idum; // store new random number
}
//**********************************************************************************/
double AiirD_8(double NewSample)
{
    int n;
    //shift the old samples
    for(n=NCoef; n>0; n--)
    {
       AxD8[n] = AxD8[n-1];
       AyD8[n] = AyD8[n-1];
    }

    //Calculate the new output
    AxD8[0] = NewSample;
    AyD8[0] = ACoef8[0] * AxD8[0];

    for(n=1; n<=NCoef; n++)AyD8[0] += ACoef8[n] * AxD8[n] - BCoef8[n] * AyD8[n];

    return AyD8[0];
}
double AiirD_4(double NewSample)
{
    int n;
    //shift the old samples
    for(n=NCoef; n>0; n--)
    {
       AxD4[n] = AxD4[n-1];
       AyD4[n] = AyD4[n-1];
    }

    //Calculate the new output
    AxD4[0] = NewSample;
    AyD4[0] = ACoef4[0] * AxD4[0];

    for(n=1; n<=NCoef; n++)AyD4[0] += ACoef4[n] * AxD4[n] - BCoef4[n] * AyD4[n];

    return AyD4[0];
}
double AiirD_2(double NewSample)
{
    int n;
    //shift the old samples
    for(n=NCoef; n>0; n--)
    {
       AxD2[n] = AxD2[n-1];
       AyD2[n] = AyD2[n-1];
    }

    //Calculate the new output
    AxD2[0] = NewSample;
    AyD2[0] = ACoef2[0] * AxD2[0];

    for(n=1; n<=NCoef; n++)AyD2[0] += ACoef2[n] * AxD2[n] - BCoef2[n] * AyD2[n];

    return AyD2[0];
    //return AyD2[0];
}

double BiirD_8(double NewSample)
{
    int n;
    //shift the old samples
    for(n=NCoef; n>0; n--)
    {
       BxD8[n] = BxD8[n-1];
       ByD8[n] = ByD8[n-1];
    }

    //Calculate the new output
    BxD8[0] = NewSample;
    ByD8[0] = ACoef8[0] * BxD8[0];

    for(n=1; n<=NCoef; n++)ByD8[0] += ACoef8[n] * BxD8[n] - BCoef8[n] * ByD8[n];

    return ByD8[0];
}
double BiirD_4(double NewSample)
{
    int n;
    //shift the old samples
    for(n=NCoef; n>0; n--)
    {
       BxD4[n] = BxD4[n-1];
       ByD4[n] = ByD4[n-1];
    }

    //Calculate the new output
    BxD4[0] = NewSample;
    ByD4[0] = ACoef4[0] * BxD4[0];

    for(n=1; n<=NCoef; n++)ByD4[0] += ACoef4[n] * BxD4[n] - BCoef4[n] * ByD4[n];

    return ByD4[0];
}
double BiirD_2(double NewSample)
{
    int n;
    //shift the old samples
    for(n=NCoef; n>0; n--)
    {
       BxD2[n] = BxD2[n-1];
       ByD2[n] = ByD2[n-1];
    }

    //Calculate the new output
    BxD2[0] = NewSample;
    ByD2[0] = ACoef2[0] * BxD2[0];

    for(n=1; n<=NCoef; n++)ByD2[0] += ACoef2[n] * BxD2[n] - BCoef2[n] * ByD2[n];

    return ByD2[0];
    //return AyD2[0];
}

//**********************************************************//

double AiirU_8(double NewSample)
{
    int n;
    //shift the old samples
    for(n=NCoef; n>0; n--)
    {
       AxU8[n] = AxU8[n-1];
       AyU8[n] = AyU8[n-1];
    }

    //Calculate the new output
    AxU8[0] = NewSample;
    AyU8[0] = ACoef8[0] * AxU8[0];

    for(n=1; n<=NCoef; n++)AyU8[0] += ACoef8[n] * AxU8[n] - BCoef8[n] * AyU8[n];

    return AyU8[0];
}
double AiirU_4(double NewSample)
{
    int n;
    //shift the old samples
    for(n=NCoef; n>0; n--)
    {
       AxU4[n] = AxU4[n-1];
       AyU4[n] = AyU4[n-1];
    }

    //Calculate the new output
    AxU4[0] = NewSample;
    AyU4[0] = ACoef4[0] * AxU4[0];

    for(n=1; n<=NCoef; n++)AyU4[0] += ACoef4[n] * AxU4[n] - BCoef4[n] * AyU4[n];

    return AyU4[0];
}
double AiirU_2(double NewSample)
{
    int n;
    //shift the old samples
    for(n=NCoef; n>0; n--)
    {
       AxU2[n] = AxU2[n-1];
       AyU2[n] = AyU2[n-1];
    }

    //Calculate the new output
    AxU2[0] = NewSample;
    AyU2[0] = ACoef2[0] * AxU2[0];

    for(n=1; n<=NCoef; n++)AyU2[0] += ACoef2[n] * AxU2[n] - BCoef2[n] * AyU2[n];

    return AyU2[0];
    //return AyU2[0];
}

double BiirU_8(double NewSample)
{
    int n;
    //shift the old samples
    for(n=NCoef; n>0; n--)
    {
       BxU8[n] = BxU8[n-1];
       ByU8[n] = ByU8[n-1];
    }

    //Calculate the new output
    BxU8[0] = NewSample;
    ByU8[0] = ACoef8[0] * BxU8[0];

    for(n=1; n<=NCoef; n++)ByU8[0] += ACoef8[n] * BxU8[n] - BCoef8[n] * ByU8[n];

    return ByU8[0];
}
double BiirU_4(double NewSample)
{
    int n;
    //shift the old samples
    for(n=NCoef; n>0; n--)
    {
       BxU4[n] = BxU4[n-1];
       ByU4[n] = ByU4[n-1];
    }

    //Calculate the new output
    BxU4[0] = NewSample;
    ByU4[0] = ACoef4[0] * BxU4[0];

    for(n=1; n<=NCoef; n++)ByU4[0] += ACoef4[n] * BxU4[n] - BCoef4[n] * ByU4[n];

    return ByU4[0];
}
double BiirU_2(double NewSample)
{
    int n;
    //shift the old samples
    for(n=NCoef; n>0; n--)
    {
       BxU2[n] = BxU2[n-1];
       ByU2[n] = ByU2[n-1];
    }

    //Calculate the new output
    BxU2[0] = NewSample;
    ByU2[0] = ACoef2[0] * BxU2[0];

    for(n=1; n<=NCoef; n++)ByU2[0] += ACoef2[n] * BxU2[n] - BCoef2[n] * ByU2[n];

    return ByU2[0];
    //return AyU2[0];
}

//**********************************************************//

void subProcessSleep(int sampleFrames)
{

  	   /* //old way to set the output buffer to 0
  	    auto pitch = getBuffer(pinPitch);
		auto pulseWidth = getBuffer(pinPulseWidth);
		auto phaseMod = getBuffer(pinPhaseMod);
		auto syncIn = getBuffer(pinSyncIn);
		auto syncPitch = getBuffer(pinSyncPitch);
		auto audioOut = getBuffer(pinAudioOut);
		auto cutoff = getBuffer(pinCutOff);

		for (int s = sampleFrames; s > 0; --s)
		{
            *audioOut=0.f;
			++pitch;
			++pulseWidth;
			++phaseMod;
			++syncIn;
			++syncPitch;
			++audioOut;
            ++cutoff;
		}*/

    // OscillatorHD sleep process
    if (zeroSamplesCounter > getBlockSize())
	{
		SET_PROCESS2(&PolyBlepOsc::subProcessNothing);
	}
	else
	{
		auto audioOut = getBuffer(pinAudioOut);

		for (int s = sampleFrames; s > 0; --s)
		{
			*audioOut++ = 0.f;
		}

		zeroSamplesCounter += sampleFrames;
	}


}

virtual void onSetPins() override
{

    if( pinOverFactor != m_OverFactor)
    {
    m_OverFactor=pinOverFactor;

    //ResetF();
    mcut=-100.f;
    prev_tuning=-100.f;
    prev_tuning2=-100.f;
    t = 0;
    Mt= 0;


        if(sampleRate>=176400) // limit oversampling to 16 *96 k 1.536 ghz
        {
            //if(pinOverFactor<16)OverFactor=pinOverFactor;
            //else
            //{
            OverFactor=pinOverFactor/2;
            if(OverFactor<2)OverFactor=2;
            //}

        }
        else
        {
        OverFactor=pinOverFactor;
        }

        loopI=OverFactor/2;
        //OverSpl=sampleRate*OverFactor;//for testing sync pitch

    }

    if( pinVoltLimit!= m_VoltLimit)
    {
    m_VoltLimit=pinVoltLimit;


        switch (pinVoltLimit)
        {
        case 0:
            VoltLimit=10.f;
            break;
        case 1:
            VoltLimit=10.33333333333f;
            break;
        case 2:
            VoltLimit=10.5f;
            break;
        }

    }



    if( pinVoiceActive.isUpdated() && pinVoiceActive>0.0f && pinResetMode >= 2)
    {
    mcut=-100.f;
    prev_tuning=-100.f;
    prev_tuning2=-100.f;
	t = 0;
    Mt= 0;
    }


    //if(pinBypass !=m_Bypass && m_Bypass==false)//pinBypass==true
    if(pinBypass)
    {
    m_Bypass=true;

        mcut=-100.f;
        prev_tuning=-100.f;
        prev_tuning2=-100.f;
        t = 0;
        Mt= 0;
        zeroSamplesCounter = 0; // OscillatorHD sleep counter
        SET_PROCESS2(&PolyBlepOsc::subProcessSleep);
        pinAudioOut.setStreaming(false);
        setSleep(true);
    }
    else
    {

        if( pinWaveform != m_Waveform || pinSyncMode != m_SyncMode || pinResetMode != m_ResetMode || (!pinBypass && m_Bypass==true))
        {

                    m_Bypass=false;
                    m_Waveform=pinWaveform;
                    m_SyncMode=pinSyncMode;
                    m_ResetMode=pinResetMode;

                   // ResetF();
                    mcut=-100.f;
                    prev_tuning=-100.f;
                    prev_tuning2=-100.f;
                    t = 0;
                    Mt= 0;

                    switch (pinWaveform)
                    {
                    case 0:
                        //setWaveform(SINE);
                        if(pinSyncMode !=2)setSubProcess(&PolyBlepOsc::subProcessSine);
                        else
                        setSubProcess(&PolyBlepOsc::subProcessSineSyncExt);
                        break;

                    case 1:
                        //setWaveform(RAMP);
                        if(pinSyncMode !=2)setSubProcess(&PolyBlepOsc::subProcessRamp);
                        else
                        setSubProcess(&PolyBlepOsc::subProcessRampSyncExt);
                        break;

                    case 2:
                        //setWaveform(SAWTOOTH);
                        if(pinSyncMode !=2)setSubProcess(&PolyBlepOsc::subProcessSaw);
                        else
                        setSubProcess(&PolyBlepOsc::subProcessSawSyncExt);
                        break;

                    case 3:
                        //setWaveform(TRIANGLE);
                        if(pinSyncMode !=2)setSubProcess(&PolyBlepOsc::subProcessTri);
                        else
                        setSubProcess(&PolyBlepOsc::subProcessTriSyncExt);
                        break;

                    case 4:
                        //setWaveform(SQUARE);
                        if(pinSyncMode !=2)setSubProcess(&PolyBlepOsc::subProcessPulse);
                        else
                        setSubProcess(&PolyBlepOsc::subProcessPulseSyncExt);
                        break;
                    case 5:
                        //setWaveform(White Noise);
                        setSubProcess(&PolyBlepOsc::subProcessWhiteNoise);
                        break;
                    case 6:
                        //setWaveform(Pink Noise);
                        setSubProcess(&PolyBlepOsc::subProcessPinkNoise);
                        break;
                    }
                pinAudioOut.setStreaming(true);
                setSleep(false);
       }

    }
////////////////////////////////////////////////////////////////

    // that works but actually in unstable context the other method seems little more robust...
    /*if( pinOverFactor.isUpdated())
    {

        //ResetF();
        mcut=-100.f;
        prev_tuning=-100.f;
        prev_tuning2=-100.f;
        t = 0;
        Mt= 0;


        if(sampleRate>=176400) // limit oversampling to 16 *96 k 1.536 ghz
        {
            //if(pinOverFactor<16)OverFactor=pinOverFactor;
            //else
            //{
            OverFactor=pinOverFactor/2;
            if(OverFactor<2)OverFactor=2;
            //}

        }
        else
        {
        OverFactor=pinOverFactor;
        }

        loopI=OverFactor/2;
        //OverSpl=sampleRate*OverFactor;//for testing sync pitch

    }

    if( pinVoltLimit.isUpdated() )
    {

        switch (pinVoltLimit)
        {
        case 0:
            VoltLimit=10.f;
            break;
        case 1:
            VoltLimit=10.33333333333f;
            break;
        case 2:
            VoltLimit=10.5f;
            break;
        }

    }



    if( pinVoiceActive.isUpdated() && pinVoiceActive>0.0f && pinResetMode >= 2)
    {
    mcut=-100.f;
    prev_tuning=-100.f;
    prev_tuning2=-100.f;
	t = 0;
    Mt= 0;
    }


    if(pinBypass)//pinBypass==true
    {
        mcut=-100.f;
        prev_tuning=-100.f;
        prev_tuning2=-100.f;
        t = 0;
        Mt= 0;
        //zeroSamplesCounter = 0; // OscillatorHD sleep counter
        SET_PROCESS2(&PolyBlepOsc::subProcessSleep);
        pinAudioOut.setStreaming(false);
        setSleep(true);
    }
    else
    {
        if( pinWaveform.isUpdated() || pinSyncMode.isUpdated() || pinBypass.isUpdated() || pinResetMode.isUpdated())
        {

               // ResetF();
                mcut=-100.f;
                prev_tuning=-100.f;
                prev_tuning2=-100.f;
                t = 0;
                Mt= 0;


                    switch (pinWaveform)
                    {
                    case 0:
                        //setWaveform(SINE);
                        if(pinSyncMode !=2)setSubProcess(&PolyBlepOsc::subProcessSine);
                        else
                        setSubProcess(&PolyBlepOsc::subProcessSineSyncExt);
                        break;

                    case 1:
                        //setWaveform(RAMP);
                        if(pinSyncMode !=2)setSubProcess(&PolyBlepOsc::subProcessRamp);
                        else
                        setSubProcess(&PolyBlepOsc::subProcessRampSyncExt);
                        break;

                    case 2:
                        //setWaveform(SAWTOOTH);
                        if(pinSyncMode !=2)setSubProcess(&PolyBlepOsc::subProcessSaw);
                        else
                        setSubProcess(&PolyBlepOsc::subProcessSawSyncExt);
                        break;

                    case 3:
                        //setWaveform(TRIANGLE);
                        if(pinSyncMode !=2)setSubProcess(&PolyBlepOsc::subProcessTri);
                        else
                        setSubProcess(&PolyBlepOsc::subProcessTriSyncExt);
                        break;

                    case 4:
                        //setWaveform(SQUARE);
                        if(pinSyncMode !=2)setSubProcess(&PolyBlepOsc::subProcessPulse);
                        else
                        setSubProcess(&PolyBlepOsc::subProcessPulseSyncExt);
                        break;
                    case 5:
                        //setWaveform(White Noise);
                        setSubProcess(&PolyBlepOsc::subProcessWhiteNoise);
                        break;
                    case 6:
                        //setWaveform(Pink Noise);
                        setSubProcess(&PolyBlepOsc::subProcessPinkNoise);
                        break;
                    }
                pinAudioOut.setStreaming(true);
                setSleep(false);
        }

    }*/

}

};
namespace
{
	auto r = Register<PolyBlepOsc>::withId(L"CK PolyBlep Oscillator");
}

// all waveforms, just copy and past in new proccess to test them...
/*

	double sin() const {
		return amplitude * std::sin(TWO_PI * t);
	}

	double cos() const {
		return amplitude * std::cos(TWO_PI * t);
	}

	double half() const {
		double t2 = t + 0.5;
		t2 -= bitwiseOrZero(t2);

		double y = (t < 0.5 ? 2 * std::sin(TWO_PI * t) - 2 / M_PI : -2 / M_PI);
		y += TWO_PI * FreqInc * (blamp(t, FreqInc) + blamp(t2, FreqInc));

		return amplitude * y;
	}

	double full() const {
		double _t = this->t + 0.25;
		_t -= bitwiseOrZero(_t);

		double y = 2 * std::sin(M_PI * _t) - 4 / M_PI;
		y += TWO_PI * FreqInc * blamp(_t, FreqInc);

		return amplitude * y;
	}

	double tri() const {
		double t1 = t + 0.25;
		t1 -= bitwiseOrZero(t1);

		double t2 = t + 0.75;
		t2 -= bitwiseOrZero(t2);

		double y = t * 4;

		if (y >= 3) {
			y -= 4;
		}
		else if (y > 1) {
			y = 2 - y;
		}

		y += 4 * FreqInc * (blamp(t1, FreqInc) - blamp(t2, FreqInc));

		return amplitude * y;
	}

	double tri2() const {
		double pulseWidth = std::fmax(0.0001, std::fmin(0.9999, this->pulseWidth));

		double t1 = t + 0.5 * pulseWidth;
		t1 -= bitwiseOrZero(t1);

		double t2 = t + 1 - 0.5 * pulseWidth;
		t2 -= bitwiseOrZero(t2);

		double y = t * 2;

		if (y >= 2 - pulseWidth) {
			y = (y - 2) / pulseWidth;
		}
		else if (y >= pulseWidth) {
			y = 1 - (y - pulseWidth) / (1 - pulseWidth);
		}
		else {
			y /= pulseWidth;
		}

		y += FreqInc / (pulseWidth - pulseWidth * pulseWidth) * (blamp(t1, FreqInc) - blamp(t2, FreqInc));

		return amplitude * y;
	}

	double trip() const {
		double t1 = t + 0.75 + 0.5 * pulseWidth;
		t1 -= bitwiseOrZero(t1);

		double y;
		if (t1 >= pulseWidth) {
			y = -pulseWidth;
		}
		else {
			y = 4 * t1;
			y = (y >= 2 * pulseWidth ? 4 - y / pulseWidth - pulseWidth : y / pulseWidth - pulseWidth);
		}

		if (pulseWidth > 0) {
			double t2 = t1 + 1 - 0.5 * pulseWidth;
			t2 -= bitwiseOrZero(t2);

			double t3 = t1 + 1 - pulseWidth;
			t3 -= bitwiseOrZero(t3);
			y += 2 * FreqInc / pulseWidth * (blamp(t1, FreqInc) - 2 * blamp(t2, FreqInc) + blamp(t3, FreqInc));
		}
		return amplitude * y;
	}

	double trap() const {
		double y = 4 * t;
		if (y >= 3) {
			y -= 4;
		}
		else if (y > 1) {
			y = 2 - y;
		}
		y = std::fmax(-1, std::fmin(1, 2 * y));

		double t1 = t + 0.125;
		t1 -= bitwiseOrZero(t1);

		double t2 = t1 + 0.5;
		t2 -= bitwiseOrZero(t2);

		// Triangle #1
		y += 4 * FreqInc * (blamp(t1, FreqInc) - blamp(t2, FreqInc));

		t1 = t + 0.375;
		t1 -= bitwiseOrZero(t1);

		t2 = t1 + 0.5;
		t2 -= bitwiseOrZero(t2);

		// Triangle #2
		y += 4 * FreqInc * (blamp(t1, FreqInc) - blamp(t2, FreqInc));

		return amplitude * y;
	}

	double trap2() const {
		double pulseWidth = std::fmin(0.9999, this->pulseWidth);
		double scale = 1 / (1 - pulseWidth);

		double y = 4 * t;
		if (y >= 3) {
			y -= 4;
		}
		else if (y > 1) {
			y = 2 - y;
		}
		y = std::fmax(-1, std::fmin(1, scale * y));

		double t1 = t + 0.25 - 0.25 * pulseWidth;
		t1 -= bitwiseOrZero(t1);

		double t2 = t1 + 0.5;
		t2 -= bitwiseOrZero(t2);

		// Triangle #1
		y += scale * 2 * FreqInc * (blamp(t1, FreqInc) - blamp(t2, FreqInc));

		t1 = t + 0.25 + 0.25 * pulseWidth;
		t1 -= bitwiseOrZero(t1);

		t2 = t1 + 0.5;
		t2 -= bitwiseOrZero(t2);

		// Triangle #2
		y += scale * 2 * FreqInc * (blamp(t1, FreqInc) - blamp(t2, FreqInc));

		return amplitude * y;
	}

	double sqr() const {
		double t2 = t + 0.5;
		t2 -= bitwiseOrZero(t2);

		double y = t < 0.5 ? 1 : -1;
		y += blep(t, FreqInc) - blep(t2, FreqInc);

		return amplitude * y;
	}

	double sqr2() const {
		double t1 = t + 0.875 + 0.25 * (pulseWidth - 0.5);
		t1 -= bitwiseOrZero(t1);

		double t2 = t + 0.375 + 0.25 * (pulseWidth - 0.5);
		t2 -= bitwiseOrZero(t2);

		// Square #1
		double y = t1 < 0.5 ? 1 : -1;

		y += blep(t1, FreqInc) - blep(t2, FreqInc);

		t1 += 0.5 * (1 - pulseWidth);
		t1 -= bitwiseOrZero(t1);

		t2 += 0.5 * (1 - pulseWidth);
		t2 -= bitwiseOrZero(t2);

		// Square #2
		y += t1 < 0.5 ? 1 : -1;

		y += blep(t1, FreqInc) - blep(t2, FreqInc);

		return amplitude * 0.5 * y;
	}

	double rect() const {
		double t2 = t + 1 - pulseWidth;
		t2 -= bitwiseOrZero(t2);

		double y = -2 * pulseWidth;
		if (t < pulseWidth) {
			y += 2;
		}

		y += blep(t, FreqInc) - blep(t2, FreqInc);

		return amplitude * y;
	}

	//SE RAMP ???
	double saw() const {
		double _t = t + 0.5;
		_t -= bitwiseOrZero(_t);

		double y = 2 * _t - 1;
		y -= blep(_t, FreqInc);

		return amplitude * y;
	}

	//SE SAW ???
	double ramp() const {
		double _t = t;
		_t -= bitwiseOrZero(_t);

		double y = 1 - 2 * _t;
		y += blep(_t, FreqInc);

		return amplitude * y;
	}

*/

//*******************************************************************************************/

// original Jeff's code, file "Oscillator" from SDK3 of SE 1.5552, 08/08/2024



/*
#include "../se_sdk3/mp_sdk_audio.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <cstdint>

/*
PolyBLEP Waveform generator ported from the Jesusonic code by Tale
http://www.taletn.com/reaper/mono_synth/

Permission has been granted to release this port under the WDL/IPlug license:

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
*/

/*
using namespace gmpi;

const double TWO_PI = 2 * M_PI;

template<typename T>
inline T square_number(const T &x) {
	return x * x;
}

// Adapted from "Phaseshaping Oscillator Algorithms for Musical Sound
// Synthesis" by Jari Kleimola, Victor Lazzarini, Joseph Timoney, and Vesa
// Valimaki.
// http://www.acoustics.hut.fi/publications/papers/smc2010-phaseshaping/
inline double blep(double t, double dt) {
	if (t < dt) {
		return -square_number(t / dt - 1);
	}
	else if (t > 1 - dt) {
		return square_number((t - 1) / dt + 1);
	}
	else {
		return 0;
	}
}

// Derived from blep().
inline double blamp(double t, double dt) {
	if (t < dt) {
		t = t / dt - 1;
		return -1 / 3.0 * square_number(t) * t;
	}
	else if (t > 1 - dt) {
		t = (t - 1) / dt + 1;
		return 1 / 3.0 * square_number(t) * t;
	}
	else {
		return 0;
	}
}

template<typename T>
inline int64_t bitwiseOrZero(const T &t) {
	return static_cast<int64_t>(t) | 0;
}

class Oscillator : public MpBase2
{
	AudioInPin pinPitch;
	AudioInPin pinPulseWidth;
	IntInPin pinWaveform;
	AudioInPin pinSync;
	AudioInPin pinPhaseMod;
	AudioOutPin pinAudioOut;
	AudioInPin pinPMDepthdmy;
	BoolInPin pinPolypodPW;
	IntInPin pinOne_Shot;
	BoolInPin pinBypass;
	BoolInPin pinEconomymode;
	IntInPin pinResetMode;
	FloatInPin pinVoiceActive;

	enum Waveform {
		SINE,
		COSINE,
		TRIANGLE,
		SQUARE,
		RECTANGLE,
		SAWTOOTH,
		RAMP,
		MODIFIED_TRIANGLE,
		MODIFIED_SQUARE,
		HALF_WAVE_RECTIFIED_SINE,
		FULL_WAVE_RECTIFIED_SINE,
		TRIANGULAR_PULSE,
		TRAPEZOID_FIXED,
		TRAPEZOID_VARIABLE
	};

	Waveform waveform = SQUARE;
	double sampleRate = 44100;
	double freqInSecondsPerSample;
	double amplitude = 0.5; // Frequency dependent gain [0.0..1.0]
	double pulseWidth = 0.5; // [0.0..1.0]
	double t = 0.0; // The current phase [0.0..1.0) of the oscillator.


public:
	Oscillator()
	{
		initializePin( pinPitch );
		initializePin( pinPulseWidth );
		initializePin( pinWaveform );
		initializePin( pinSync );
		initializePin( pinPhaseMod );
		initializePin( pinAudioOut );
		initializePin( pinPMDepthdmy );
		initializePin( pinPolypodPW );
		initializePin( pinOne_Shot );
		initializePin( pinBypass );
		initializePin( pinEconomymode );
		initializePin( pinResetMode );
		initializePin( pinVoiceActive );


    //PolyBLEP::PolyBLEP(double sampleRate, Waveform waveform, double initialFrequency)
    //: waveform(waveform), sampleRate(sampleRate), amplitude(1.0), t(0.0) {
    //setSampleRate(sampleRate);
    //setFrequency(initialFrequency);
    //setWaveform(waveform);
    //setPulseWidth(0.5);
    //}


		setFrequency(400.0);
	}

	int32_t MP_STDCALL open() override
	{
		setSampleRate(getSampleRate());
		return MpBase2::open();
	}

	const static int maxVolts = 10;
	inline float SampleToVoltage(float s) const
	{
		return s * (float)maxVolts;
	}
	inline float SampleToFrequency(float volts) const
	{
		return 440.f * powf(2.f, SampleToVoltage(volts) - (float)maxVolts * 0.5f);
	}

	void subProcess( int sampleFrames )
	{
		// get pointers to in/output buffers.
		auto pitch = getBuffer(pinPitch);
		auto pulseWidth = getBuffer(pinPulseWidth);
		auto sync = getBuffer(pinSync);
		auto phaseMod = getBuffer(pinPhaseMod);
		auto audioOut = getBuffer(pinAudioOut);
		auto pMDepthdmy = getBuffer(pinPMDepthdmy);

		setFrequency(SampleToFrequency(*pitch));

		for( int s = sampleFrames; s > 0; --s )
		{
			// TODO: Signal processing goes here.
			*audioOut = getAndInc();

			// Increment buffer pointers.
			++pitch;
			++pulseWidth;
			++sync;
			++phaseMod;
			++audioOut;
			++pMDepthdmy;
		}
	}

	void subProcessPitchMod(int sampleFrames)
	{
		// get pointers to in/output buffers.
		auto pitch = getBuffer(pinPitch);
		auto pulseWidth = getBuffer(pinPulseWidth);
		auto sync = getBuffer(pinSync);
		auto phaseMod = getBuffer(pinPhaseMod);
		auto audioOut = getBuffer(pinAudioOut);
		auto pMDepthdmy = getBuffer(pinPMDepthdmy);

		for (int s = sampleFrames; s > 0; --s)
		{
			setFrequency(SampleToFrequency(*pitch));

			// TODO: Signal processing goes here.
			*audioOut = getAndInc();

			// Increment buffer pointers.
			++pitch;
			++pulseWidth;
			++sync;
			++phaseMod;
			++audioOut;
			++pMDepthdmy;
		}
	}

	virtual void onSetPins() override
	{
		// Check which pins are updated.
		if( pinPulseWidth.isStreaming() )
		{
		}
		if( pinWaveform.isUpdated() )
		{
			switch (pinWaveform)
			{
			case 0:
				setWaveform(SINE);
				break;

			case 1:
				setWaveform(SAWTOOTH);
				break;

			case 2:
				setWaveform(RAMP);
				break;

			case 3:
				setWaveform(TRIANGLE);
				break;

			case 4:
				setWaveform(SQUARE);
				break;
			}
		}
		if( pinSync.isStreaming() )
		{
		}
		if( pinPhaseMod.isStreaming() )
		{
		}
		if( pinPMDepthdmy.isStreaming() )
		{
		}
		if( pinPolypodPW.isUpdated() )
		{
		}
		if( pinOne_Shot.isUpdated() )
		{
		}
		if( pinBypass.isUpdated() )
		{
		}
		if( pinEconomymode.isUpdated() )
		{
		}
		if( pinResetMode.isUpdated() )
		{
		}
		if( pinVoiceActive.isUpdated() )
		{
		}

		// Set state of output audio pins.
		pinAudioOut.setStreaming(true);

		if (pinPitch.isStreaming())
		{
			// Set processing method.
			setSubProcess(&Oscillator::subProcessPitchMod);
		}
		else
		{
			setSubProcess(&Oscillator::subProcess);
		}

		// Set sleep mode (optional).
		// setSleep(false);
	}

	void setdt(double time) {
		freqInSecondsPerSample = time;
	}

	void setFrequency(double freqInHz) {
		setdt(freqInHz / sampleRate);
	}

	void setSampleRate(double sampleRate) {
		const double freqInHz = getFreqInHz();
		this->sampleRate = sampleRate;
		setFrequency(freqInHz);
	}

	double getFreqInHz() const {
		return freqInSecondsPerSample * sampleRate;
	}

	void setPulseWidth(double pulseWidth) {
		this->pulseWidth = pulseWidth;
	}

	void sync(double phase) {
		t = phase;
		if (t >= 0) {
			t -= bitwiseOrZero(t);
		}
		else {
			t += 1 - bitwiseOrZero(t);
		}
	}

	void setWaveform(Waveform waveform) {
		this->waveform = waveform;
	}

	double get() const {
		if (getFreqInHz() >= sampleRate / 4) {
			return sin();
		}
		else switch (waveform) {
		case SINE:
			return sin();
		case COSINE:
			return cos();
		case TRIANGLE:
			return tri();
		case SQUARE:
			return sqr();
		case RECTANGLE:
			return rect();
		case SAWTOOTH:
			return saw();
		case RAMP:
			return ramp();
		case MODIFIED_TRIANGLE:
			return tri2();
		case MODIFIED_SQUARE:
			return sqr2();
		case HALF_WAVE_RECTIFIED_SINE:
			return half();
		case FULL_WAVE_RECTIFIED_SINE:
			return full();
		case TRIANGULAR_PULSE:
			return trip();
		case TRAPEZOID_FIXED:
			return trap();
		case TRAPEZOID_VARIABLE:
			return trap2();
		default:
			return 0.0;
		}
	}

	void inc() {
		t += freqInSecondsPerSample;
		t -= bitwiseOrZero(t);
	}

	double getAndInc() {
		const double sample = get();
		inc();
		return sample;
	}

	double sin() const {
		return amplitude * std::sin(TWO_PI * t);
	}

	double cos() const {
		return amplitude * std::cos(TWO_PI * t);
	}

	double half() const {
		double t2 = t + 0.5;
		t2 -= bitwiseOrZero(t2);

		double y = (t < 0.5 ? 2 * std::sin(TWO_PI * t) - 2 / M_PI : -2 / M_PI);
		y += TWO_PI * freqInSecondsPerSample * (blamp(t, freqInSecondsPerSample) + blamp(t2, freqInSecondsPerSample));

		return amplitude * y;
	}

	double full() const {
		double _t = this->t + 0.25;
		_t -= bitwiseOrZero(_t);

		double y = 2 * std::sin(M_PI * _t) - 4 / M_PI;
		y += TWO_PI * freqInSecondsPerSample * blamp(_t, freqInSecondsPerSample);

		return amplitude * y;
	}

	double tri() const {
		double t1 = t + 0.25;
		t1 -= bitwiseOrZero(t1);

		double t2 = t + 0.75;
		t2 -= bitwiseOrZero(t2);

		double y = t * 4;

		if (y >= 3) {
			y -= 4;
		}
		else if (y > 1) {
			y = 2 - y;
		}

		y += 4 * freqInSecondsPerSample * (blamp(t1, freqInSecondsPerSample) - blamp(t2, freqInSecondsPerSample));

		return amplitude * y;
	}

	double tri2() const {
		double pulseWidth = std::fmax(0.0001, std::fmin(0.9999, this->pulseWidth));

		double t1 = t + 0.5 * pulseWidth;
		t1 -= bitwiseOrZero(t1);

		double t2 = t + 1 - 0.5 * pulseWidth;
		t2 -= bitwiseOrZero(t2);

		double y = t * 2;

		if (y >= 2 - pulseWidth) {
			y = (y - 2) / pulseWidth;
		}
		else if (y >= pulseWidth) {
			y = 1 - (y - pulseWidth) / (1 - pulseWidth);
		}
		else {
			y /= pulseWidth;
		}

		y += freqInSecondsPerSample / (pulseWidth - pulseWidth * pulseWidth) * (blamp(t1, freqInSecondsPerSample) - blamp(t2, freqInSecondsPerSample));

		return amplitude * y;
	}

	double trip() const {
		double t1 = t + 0.75 + 0.5 * pulseWidth;
		t1 -= bitwiseOrZero(t1);

		double y;
		if (t1 >= pulseWidth) {
			y = -pulseWidth;
		}
		else {
			y = 4 * t1;
			y = (y >= 2 * pulseWidth ? 4 - y / pulseWidth - pulseWidth : y / pulseWidth - pulseWidth);
		}

		if (pulseWidth > 0) {
			double t2 = t1 + 1 - 0.5 * pulseWidth;
			t2 -= bitwiseOrZero(t2);

			double t3 = t1 + 1 - pulseWidth;
			t3 -= bitwiseOrZero(t3);
			y += 2 * freqInSecondsPerSample / pulseWidth * (blamp(t1, freqInSecondsPerSample) - 2 * blamp(t2, freqInSecondsPerSample) + blamp(t3, freqInSecondsPerSample));
		}
		return amplitude * y;
	}

	double trap() const {
		double y = 4 * t;
		if (y >= 3) {
			y -= 4;
		}
		else if (y > 1) {
			y = 2 - y;
		}
		y = std::fmax(-1, std::fmin(1, 2 * y));

		double t1 = t + 0.125;
		t1 -= bitwiseOrZero(t1);

		double t2 = t1 + 0.5;
		t2 -= bitwiseOrZero(t2);

		// Triangle #1
		y += 4 * freqInSecondsPerSample * (blamp(t1, freqInSecondsPerSample) - blamp(t2, freqInSecondsPerSample));

		t1 = t + 0.375;
		t1 -= bitwiseOrZero(t1);

		t2 = t1 + 0.5;
		t2 -= bitwiseOrZero(t2);

		// Triangle #2
		y += 4 * freqInSecondsPerSample * (blamp(t1, freqInSecondsPerSample) - blamp(t2, freqInSecondsPerSample));

		return amplitude * y;
	}

	double trap2() const {
		double pulseWidth = std::fmin(0.9999, this->pulseWidth);
		double scale = 1 / (1 - pulseWidth);

		double y = 4 * t;
		if (y >= 3) {
			y -= 4;
		}
		else if (y > 1) {
			y = 2 - y;
		}
		y = std::fmax(-1, std::fmin(1, scale * y));

		double t1 = t + 0.25 - 0.25 * pulseWidth;
		t1 -= bitwiseOrZero(t1);

		double t2 = t1 + 0.5;
		t2 -= bitwiseOrZero(t2);

		// Triangle #1
		y += scale * 2 * freqInSecondsPerSample * (blamp(t1, freqInSecondsPerSample) - blamp(t2, freqInSecondsPerSample));

		t1 = t + 0.25 + 0.25 * pulseWidth;
		t1 -= bitwiseOrZero(t1);

		t2 = t1 + 0.5;
		t2 -= bitwiseOrZero(t2);

		// Triangle #2
		y += scale * 2 * freqInSecondsPerSample * (blamp(t1, freqInSecondsPerSample) - blamp(t2, freqInSecondsPerSample));

		return amplitude * y;
	}

	double sqr() const {
		double t2 = t + 0.5;
		t2 -= bitwiseOrZero(t2);

		double y = t < 0.5 ? 1 : -1;
		y += blep(t, freqInSecondsPerSample) - blep(t2, freqInSecondsPerSample);

		return amplitude * y;
	}

	double sqr2() const {
		double t1 = t + 0.875 + 0.25 * (pulseWidth - 0.5);
		t1 -= bitwiseOrZero(t1);

		double t2 = t + 0.375 + 0.25 * (pulseWidth - 0.5);
		t2 -= bitwiseOrZero(t2);

		// Square #1
		double y = t1 < 0.5 ? 1 : -1;

		y += blep(t1, freqInSecondsPerSample) - blep(t2, freqInSecondsPerSample);

		t1 += 0.5 * (1 - pulseWidth);
		t1 -= bitwiseOrZero(t1);

		t2 += 0.5 * (1 - pulseWidth);
		t2 -= bitwiseOrZero(t2);

		// Square #2
		y += t1 < 0.5 ? 1 : -1;

		y += blep(t1, freqInSecondsPerSample) - blep(t2, freqInSecondsPerSample);

		return amplitude * 0.5 * y;
	}

	double rect() const {
		double t2 = t + 1 - pulseWidth;
		t2 -= bitwiseOrZero(t2);

		double y = -2 * pulseWidth;
		if (t < pulseWidth) {
			y += 2;
		}

		y += blep(t, freqInSecondsPerSample) - blep(t2, freqInSecondsPerSample);

		return amplitude * y;
	}

	double saw() const {
		double _t = t + 0.5;
		_t -= bitwiseOrZero(_t);

		double y = 2 * _t - 1;
		y -= blep(_t, freqInSecondsPerSample);

		return amplitude * y;
	}

	double ramp() const {
		double _t = t;
		_t -= bitwiseOrZero(_t);

		double y = 1 - 2 * _t;
		y += blep(_t, freqInSecondsPerSample);

		return amplitude * y;
	}
};

namespace
{
	auto r = Register<Oscillator>::withId(L"SE Oscillator");
}
*/
