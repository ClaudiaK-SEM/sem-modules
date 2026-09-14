//lines 106, 115 mod feature added, CK's change
//lines 145 to 148 CK's change
//lines 150,151, 171, 172 DAM's change


#pragma once

#include "mp_sdk_audio.h"
#include "soundfont_user.h"

// 1-point.
class NoInterpolator
{
public:
	// needs 'Inline Function Expansion (Any Suitable)' enabled.
	inline static float Interpolate( short* const sample_ptr, int fine ) //const
	{
		constexpr float scale = 1.f / (float) 0x10000; // shorts.
		return scale * sample_ptr[0];
	};
};

// 2-point.
class LinearInterpolator
{
public:
	// needs 'Inline Function Expansion (Any Suitable)' enabled.
	inline static float Interpolate( short* const sample_ptr, int fine ) //const
	{
		constexpr float scale = ( 1.f / 0x10000 ) / (float) 0x1000000; // shorts.
		return ( scale *  ((float) sample_ptr[0] * ( 0x1000000 - (int)fine) + (float) sample_ptr[1] * (int)fine) );
	};
};

// 4-point
class CubicInterpolator
{
public:
	// needs 'Inline Function Expansion (Any Suitable)' enabled.
	inline static float Interpolate( short* const sample_ptr, int fine ) //const
	{
		constexpr float scale = 1.f / (float) 0x10000; // shorts.
/*
FAULTY!!!!!!!!!!!
		float y0, y1, y2, y3, mu;
		y0 = (float) sample_ptr[-1];
		y1 = (float) sample_ptr[0];
		y2 = (float) sample_ptr[1];
		y3 = (float) sample_ptr[2];
		mu = (float)fine / (float) 0x1000000;

		float a0,a1,a2,a3,mu2;

		mu2 = mu*mu;
		a0 = y3 - y2 - y0 + y1;
		a1 = y0 - y1 - a0;
		a2 = y2 - y0;
		a3 = y1;

		return scale * ( a0 * mu * mu2 + a1 * mu2 + a2 * mu + a3 );
*/
		float x = (float)fine / (float) 0x1000000;
		return scale * ( sample_ptr[0] + 0.5f * x*(sample_ptr[1] - sample_ptr[-1] + x*(2.0f*sample_ptr[-1] - 5.0f*sample_ptr[0] + 4.0f*sample_ptr[1] - sample_ptr[2] + x*(3.0f*(sample_ptr[0] - sample_ptr[1]) + sample_ptr[2] - sample_ptr[-1]))));

	};
};

// 7-point.
class SincInterpolator
{
public:
	// needs 'Inline Function Expansion (Any Suitable)' enabled.
	inline static float Interpolate( short* const sample_ptr, int fine ) //const
	{
		short* ptr = sample_ptr - 3;

		// reduce fraction part to range 0-31 then multiply by 8 (zeros lower 3 bits)
		float * interpolation_table_ptr = interpolation_table2 + (fine >> 19) * 8;

		// now look up interpolation filter table
		// perform 2 filters at once, one 1 sub sample ahead
		float a = 0.f;
		for( int x = 4 ; x > 0 ; x-- )
		{
			a += *ptr++ * *interpolation_table_ptr++;
			a += *ptr++ * *interpolation_table_ptr++; //unrolled
		}

		float a2 = 0.f;
		ptr = sample_ptr - 3;
		for( int x = 4 ; x > 0 ; x-- )
		{
			a2 += *ptr++ * *interpolation_table_ptr++;
			a2 += *ptr++ * *interpolation_table_ptr++;
		}

		// now linear interpolation between 2 subsamples
		constexpr float scale = (2.f / 0x1000)/(float) 0x1000000; //shorts

		// save lowest 19 bits for interpolation
		int fract_part_fine = fine & 0x7ffff;
		return scale * (a * ( 0x80000 - fract_part_fine) + a2 * fract_part_fine);
	};

	static float* interpolation_table2;
};

class PitchFixed
{
public:
	inline static void CalculateIncrement( partial & p, float*& pitch, float*& mod )//CK's change
	{
		// do nothing. Hopefully optimizes away to nothing.
	};
};

class PitchChanging
{
public:
	inline static void CalculateIncrement( partial& p, float*& pitch, float*& mod )
	{
		p.CalculateIncrement( *pitch++ + *mod++ );//CK's change
	};
};

enum PanningSupport_t { Panning, noPanning };

class SoundfontOscillator3 : public MpBase, public soundfontUser
{
public:
	SoundfontOscillator3(IMpUnknown* host);
	~SoundfontOscillator3();
	virtual int32_t MP_STDCALL open();
	void onSetPins() override;
	virtual void onGraphStart();

	IntInPin pinSampleId;
	EnumInPin pinQuality;
	AudioInPin pinPitch;
	AudioInPin pinTrigger;
	AudioInPin pinGate;
	AudioInPin pinVelocity;
	AudioOutPin pinLeft;
	AudioOutPin pinRight;
    IntInPin pinOn;//CK's change
	AudioInPin pinMod;//CK's change
	EnumInPin pinTracking;//CK's change
	//IntInPin pinFadeLoop;//CK's change

	void ChooseSubProcess(int blockPosistion);
	void process_with_gate(int bufferOffset, int sampleframes);

	template<class InterpolationPolicy, class PitchModulationPolicy = PitchFixed, PanningSupport_t PanningSupport = noPanning>//DAM's change
	void sub_process_template(int bufferOffset, int sampleframes);//DAM's change

	void sub_process_silence(int bufferOffset, int sampleframes);
	void NoteDone(int blockPosistion);
	void NoteOn(int blockPosistion);
	void ResetWave(void);

private:
	float* GetInterpolationtable();
	bool trigger_state;
	bool gate_state;

	std::wstring FileName;

	SubProcess_ptr current_osc_func = {};
	bool sample_playing = false;

	// Loop crossfade window size in samples
	static constexpr int loopCrossfadeSamples = 32;//DAM's change
};
