//add "On"
//line 46,65 to 66,97 to 106

//add "Mod"
//line 51, 178, 184, 358, 449, 513

//add "Tracking ID"
//line 53, 178, 360

//DAM's custom code to add a crossfade (32 spl) at the loop end instead of Jeff's code,
//to avoid clicks when the interpolation is used, if the loop end point is to close of the end sample.
//I adapted his code for the compatibility with stereo samples which use different loop end points.
//line 165 to 334


//line 314 end note buffer offset to -1, better stability with short asio buffers (2ms)
//and originally I modified this line to avoid clicks with no looped samples with the first version of the sampleoscillator2

#include <math.h>
#include "SampleOscillator3.h"
#include "CSoundFont.h"
#include "SampleManager.h"

#define INTERPOLATION_POINTS 8
#define INTERPOLATION_DIV 32

const int tableSize_ = 2 * (INTERPOLATION_POINTS / 2 + 1) * INTERPOLATION_DIV;
static float interpolationTable[tableSize_];
static bool interpolationTableInitialized = false;
float* SincInterpolator::interpolation_table2 = 0;

REGISTER_PLUGIN ( SoundfontOscillator3, L"SampleOscillator3" );

/* notes
Soundfonts don't unload until all voices have been woken once, can take sometime during which both soundfonts
consume memory, same when changing patch (each patch currently treated as new soundfont).
*/

SoundfontOscillator3::SoundfontOscillator3(IMpUnknown* host) : MpBase(host)
, trigger_state(false)
, gate_state(false)
{
	// Register pins.
	initializePin(0, pinSampleId);
	initializePin(1, pinPitch);
	initializePin(2, pinTrigger);
	initializePin(3, pinGate);
	initializePin(4, pinVelocity);
	initializePin(5, pinQuality);
	initializePin(6, pinLeft);
	initializePin(7, pinRight);
    initializePin( 8, pinOn );//CK's change
    initializePin( 9, pinMod );//CK's change
    initializePin( 10, pinTracking );//CK's change
    //initializePin( 11, pinFadeLoop );//CK's change

	SET_PROCESS(&SoundfontOscillator3::sub_process_silence);
}


void SoundfontOscillator3::onGraphStart()
{
	MpBase::onGraphStart();

	pinLeft.setStreaming(false);
	pinRight.setStreaming(false);
}

void SoundfontOscillator3::onSetPins()
{
        if(pinOn==1)//CK's change : on/off
        {
        setSleep(false);

                    if (pinGate.isUpdated() || pinTrigger.isUpdated())
                    {
                        if (pinGate.isStreaming() || pinTrigger.isStreaming())
                        {
                            SET_PROCESS(&SoundfontOscillator3::process_with_gate);
                        }
                        else
                        {
                            bool prevTrigger = trigger_state;
                            bool prevGate = gate_state;

                            gate_state = (pinGate > 0.f);
                            trigger_state = (pinTrigger > 0.f);

                            // Gate must be high, and either gate or trigger transition high.
                            if (gate_state && (!prevGate || (trigger_state && !prevTrigger)))
                            {
                                NoteOn(-1);
                            }
                        }
                    }

                    if (pinPitch.isUpdated())
                    {
                        ChooseSubProcess(-1);
                    }
       }
         //else //CK's change : on/off
         if(pinOn==0 )
         {
         gate_state = (false );
         trigger_state = ( false );
         NoteDone( -1 );
         SET_PROCESS( &SoundfontOscillator3::sub_process_silence );
         pinLeft.setStreaming( false );
         pinRight.setStreaming( false );
         setSleep(true);
         }
}

void SoundfontOscillator3::sub_process_silence(int bufferOffset, int sampleframes)
{
	float* output_l = bufferOffset + pinLeft.getBuffer();
	float* output_r = bufferOffset + pinRight.getBuffer();

	for (int s = sampleframes; s > 0; s--)
	{
		*output_l++ = *output_r++ = 0.f;
	}
}

void SoundfontOscillator3::process_with_gate(int bufferOffset, int sampleframes)
{
	float* gate = bufferOffset + pinGate.getBuffer();
	float* trigger = bufferOffset + pinTrigger.getBuffer();

	int last = bufferOffset + sampleframes;
	int to_pos = bufferOffset;
	int cur_pos = bufferOffset;

	while (cur_pos < last)
	{
		// how long till next gate/trigger change?
		while ((*gate > 0.f) == gate_state && (*trigger > 0.f) == trigger_state && to_pos < last)
		{
			to_pos++;
			gate++;
			trigger++;
		}

		//		if( to_pos > cur_pos ) // no harm (sub_process mono seems to handle sampleframes = 0 ok)
		(this->*(current_osc_func))(cur_pos, to_pos - cur_pos);

		if (to_pos == last)
		{
			return;
		}

		cur_pos = to_pos;

		bool prevTrigger = trigger_state;
		bool prevGate = gate_state;

		gate_state = (*gate > 0.f);
		trigger_state = (*trigger > 0.f);

		// Gate must be high, and either gate or trigger transition high.
		if (gate_state && (!prevGate || (trigger_state && !prevTrigger)))
		{
			NoteOn(cur_pos);
		}
	}
}

template <class InterpolationPolicy, class PitchModulationPolicy, PanningSupport_t PanningSupport>
void SoundfontOscillator3::sub_process_template(int bufferOffset, int sampleframes)
{
	bool addToOutput = false;

	for (auto it = partials.begin(); it != partials.end(); )
	{
        //partial& p = *it;
		partial& partial = *it;

		float* pitch = pinPitch.getBuffer() + bufferOffset;
		float* mod = pinMod.getBuffer() + bufferOffset;//CK's change
		float* output_l = pinLeft.getBuffer() + bufferOffset;
		float* output_r = pinRight.getBuffer() + bufferOffset;

		for (int s = sampleframes; s > 0; s--)
		{
			PitchModulationPolicy::CalculateIncrement(partial, pitch, mod);//CK's change


			//p.IncrementPointer(gate_state);
			partial.left.IncrementPointer(gate_state);
			partial.right.IncrementPointer(gate_state);
            //auto sR = partial.right.IncrementPointer(gate_state);

			// Loop crossfade blending block begins here
			//int samplesLeftInLoop = (int)(p.s_loop_end - p.s_ptr_r);
			//bool inCrossfadeRegion = (samplesLeftInLoop >= 0 && samplesLeftInLoop < partial::loopCrossfadeSamples);
			int samplesLeftInLoop = (int)(partial.left.s_loop_end - partial.left.s_ptr);
			int samplesRightInLoop = (int)(partial.right.s_loop_end - partial.right.s_ptr);
			bool inCrossfadeRegionLeft = (samplesLeftInLoop >= 0 && samplesLeftInLoop < partial::loopCrossfadeSamples);
			bool inCrossfadeRegionRight = (samplesRightInLoop >= 0 && samplesRightInLoop < partial::loopCrossfadeSamples);

			float fadeOutFactorL = 1.0f;
			float fadeOutFactorR = 1.0f;
			float fadeInFactorL = 0.f;
			float fadeInFactorR = 0.f;

			if (inCrossfadeRegionLeft)
			{
				fadeOutFactorL = (float)samplesLeftInLoop / partial::loopCrossfadeSamples;
				fadeInFactorL = 1.f - fadeOutFactorL;
			}

			if (inCrossfadeRegionRight)
			{
				fadeOutFactorR = (float)samplesRightInLoop / partial::loopCrossfadeSamples;
				fadeInFactorR = 1.f - fadeOutFactorR;
			}

			float left, right;

			if (partial.IsStereo())
			{

				//assert(p.s_ptr_l == p.s_ptr_r + p.s_ptr_l_offset);
				//float sampleMainRight = InterpolationPolicy::Interpolate(p.s_ptr_r, p.s_ptr_fine);
				//float sampleMainLeft = InterpolationPolicy::Interpolate(p.s_ptr_l, p.s_ptr_fine);

                //assert(partial.left.s_ptr == partial.right.s_ptr+ partial.left.s_ptr_offset);
				float sampleMainRight = InterpolationPolicy::Interpolate(partial.right.s_ptr, partial.right.s_ptr_fine);
				float sampleMainLeft = InterpolationPolicy::Interpolate(partial.left.s_ptr, partial.left.s_ptr_fine);

				if (inCrossfadeRegionLeft)
				{
					//short* loopStartLeft = p.s_loop_st + (p.s_ptr_l - p.s_loop_end);
					short* loopStartLeft = partial.left.s_loop_st + (partial.left.s_ptr - partial.left.s_loop_end);

					//float crossfadeSampleLeft = InterpolationPolicy::Interpolate(loopStartLeft, p.s_ptr_fine);
					float crossfadeSampleLeft = InterpolationPolicy::Interpolate(loopStartLeft, partial.left.s_ptr_fine);

					left = sampleMainLeft * fadeOutFactorL + crossfadeSampleLeft * fadeInFactorL;
				}
				else
				{
					left = sampleMainLeft;
				}

				if (inCrossfadeRegionRight)
				{
					//short* loopStartRight = p.s_loop_st + (p.s_ptr_r - p.s_loop_end);
					short* loopStartRight = partial.right.s_loop_st + (partial.right.s_ptr - partial.right.s_loop_end);


					//float crossfadeSampleRight = InterpolationPolicy::Interpolate(loopStartRight, p.s_ptr_fine);
					float crossfadeSampleRight = InterpolationPolicy::Interpolate(loopStartRight, partial.right.s_ptr_fine);


					right = sampleMainRight * fadeOutFactorR + crossfadeSampleRight * fadeInFactorR;
				}
				else
				{
					right = sampleMainRight;
				}

			}
			else
			{
				//float sampleMain = InterpolationPolicy::Interpolate(p.s_ptr_r, p.s_ptr_fine);
  				float sampleMain = InterpolationPolicy::Interpolate(partial.right.s_ptr, partial.right.s_ptr_fine);

				if (inCrossfadeRegionRight)
				{
					//short* loopStartRight = p.s_loop_st + (p.s_ptr_r - p.s_loop_end);
					short* loopStartRight = partial.right.s_loop_st + (partial.right.s_ptr - partial.right.s_loop_end);

					//float crossfadeSample = InterpolationPolicy::Interpolate(loopStartRight, p.s_ptr_fine);
					float crossfadeSample = InterpolationPolicy::Interpolate(loopStartRight, partial.right.s_ptr_fine);

					float outSample = sampleMain * fadeOutFactorR + crossfadeSample * fadeInFactorR;

					if (PanningSupport == Panning)
					{
						left = outSample * partial.pan_left_level;
						right = outSample * partial.pan_right_level;
					}
					else
					{
						left = right = outSample;
					}
				}
				else
				{
					if (PanningSupport == Panning)
					{
						left = sampleMain * partial.pan_left_level;
						right = sampleMain * partial.pan_right_level;
					}
					else
					{
						left = right = sampleMain;
					}
				}
			}
			// Loop crossfade blending block ends here

			if (addToOutput)
			{
				*output_l += left;
				*output_r += right;
			}
			else
			{
				*output_l = left;
				*output_r = right;
			}

			++output_l;
			++output_r;
		}

		if (partial.IsDone())
		{
			it = partials.erase(it);

			if (partials.empty())
			{
				//if(pinFadeLoop==1)NoteDone(bufferOffset + sampleframes - 1);//to test original code...
				//else
				NoteDone(-1);//CK's change
			}
		}
		else
		{
			++it;
		}

		addToOutput = true;
	}
}

void SoundfontOscillator3::NoteOn(int blockPosistion)
{
	sample_playing = false;

	if (sampleHandle != pinSampleId)
	{
		partials.clear();

		SampleManager::Instance()->Release(sampleHandle);
		sampleHandle = pinSampleId;

		if (sampleHandle != -1) // none loaded
		{
			SampleManager::Instance()->AddRef(sampleHandle);
		}
	}

	if (sampleHandle != -1) // loaded OK.
	{
		float p_pitch = pinPitch.getValue(blockPosistion);
        float p_mod = pinMod.getValue( blockPosistion );//CK's change
        if(pinTracking==0)p_pitch=p_pitch+p_mod;//CK's change
		float p_velocity = pinVelocity.getValue(blockPosistion);

		const int MIDDLE_A = 69;
		WORD bank = 0;
		int chan = 0;
		float NoteNum = floorf(0.5f + MIDDLE_A + (p_pitch - 0.5f) * 120.f);
		int NoteVel = (int) (p_velocity * 127.f);

		if( NoteNum < 0.f )
			 NoteNum = 0.f;

		if( NoteNum > 127.f )
			 NoteNum = 127.f;

		if( NoteVel < 0 )
			 NoteVel = 0;

		if( NoteVel > 127 )
			 NoteVel = 127;

		GetZone(chan, (int)NoteNum, NoteVel);

		for( auto& partial : partials )
		{
			Jzone &z = *partial.zone;

			assert(0 == (partial.right.cur_sample->sfSampleType & 0x8000)); // no ROM samples allowed.

			sample_playing = true;

			float root_key = partial.right.cur_sample->byOriginalKey;
			float pitch_correction = partial.right.cur_sample->chCorrection;

			float overiding_root_key = z.Get(58).shAmount;
			if( overiding_root_key >= 0.0 )
				root_key = overiding_root_key;

			float course_tune = z.Get(51).shAmount;
			float fine_tune = z.Get(52).shAmount;
			partial.scale_tune = z.Get(56).shAmount * 0.1f; // degree to which MIDI key number influences pitch. zero -> no effect on pitch. 100 - normal semitone scale
			float tune = course_tune + ( fine_tune + pitch_correction ) / 100.f;
			float transposition = ( (float) NoteNum - root_key + tune ) / 120.f ;

			partial.root_pitch = 0.5f + (NoteNum - MIDDLE_A) / 120.f;
			partial.root_pitch -= transposition;

			partial.relative_sample_rate = (float) partial.right.cur_sample->dwSampleRate / getSampleRate();
			partial.CalculateIncrement( p_pitch );
			partial.left.s_ptr_fine = -partial.left.s_increment;
			partial.right.s_ptr_fine = -partial.right.s_increment;
		}
	}

	pinLeft.setStreaming( sample_playing, blockPosistion );
	pinRight.setStreaming( sample_playing, blockPosistion );

	ChooseSubProcess( blockPosistion );
}

void SoundfontOscillator3::NoteDone(int blockPosistion)
{
	pinRight.setStreaming(false, blockPosistion);
	pinLeft.setStreaming(false, blockPosistion);

	sample_playing = false;

	SampleManager::Instance()->Release(sampleHandle);
	sampleHandle = -1;

	ChooseSubProcess(blockPosistion);
}

int32_t SoundfontOscillator3::open()
{
	interpolation_table2 = GetInterpolationtable();
	SincInterpolator::interpolation_table2 = GetInterpolationtable();

	return MpBase::open();
}

SoundfontOscillator3::~SoundfontOscillator3()
{
	SampleManager::Instance()->Release(sampleHandle);
}

void SoundfontOscillator3::ChooseSubProcess(int blockPosistion)
{
	if (sample_playing)
	{
		if (pinMod.isStreaming() || pinPitch.isStreaming())//CK's change
		{
			bool usesPanning = false;
			for (activePartialListType::iterator it = partials.begin(); it != partials.end(); ++it)
			{
				partial& partial = *it;
				usesPanning = (std::max)(usesPanning, partial.UsesPanning());
			}

			switch (pinQuality)
			{
			case 8:
				if (usesPanning)
				{
					current_osc_func = (SubProcess_ptr)&SoundfontOscillator3::sub_process_template< SincInterpolator, PitchChanging, Panning >;
				}
				else
				{
					current_osc_func = (SubProcess_ptr)&SoundfontOscillator3::sub_process_template< SincInterpolator, PitchChanging, noPanning >;
				}
				break;

			case 4:
				if (usesPanning)
				{
					current_osc_func = (SubProcess_ptr)&SoundfontOscillator3::sub_process_template< CubicInterpolator, PitchChanging, Panning >;
				}
				else
				{
					current_osc_func = (SubProcess_ptr)&SoundfontOscillator3::sub_process_template< CubicInterpolator, PitchChanging, noPanning >;
				}
				break;

			case 0:
				if (usesPanning)
				{
					current_osc_func = (SubProcess_ptr)&SoundfontOscillator3::sub_process_template< NoInterpolator, PitchChanging, Panning >;
				}
				else
				{
					current_osc_func = (SubProcess_ptr)&SoundfontOscillator3::sub_process_template< NoInterpolator, PitchChanging, noPanning >;
				}
				break;

			default: // 2
				if (usesPanning)
				{
					current_osc_func = (SubProcess_ptr)&SoundfontOscillator3::sub_process_template< LinearInterpolator, PitchChanging, Panning >;
				}
				else
				{
					current_osc_func = (SubProcess_ptr)&SoundfontOscillator3::sub_process_template< LinearInterpolator, PitchChanging, noPanning >;
				}
				break;
			}
		}
		else
		{
			bool usesPanning = false;

			for (activePartialListType::iterator it = partials.begin(); it != partials.end(); ++it)
			{
				partial& partial = *it;
				//partial.CalculateIncrement(pinPitch.getValue(blockPosistion));
				float p=pinPitch.getValue( blockPosistion )+pinMod.getValue( blockPosistion );//CK's change
				partial.CalculateIncrement( p );//CK's change

				usesPanning = (std::max)(usesPanning, partial.UsesPanning());
			}

			switch (pinQuality)
			{
			case 8:
				if (usesPanning)
				{
					current_osc_func = (SubProcess_ptr)&SoundfontOscillator3::sub_process_template< SincInterpolator, PitchFixed, Panning >;
				}
				else
				{
					current_osc_func = (SubProcess_ptr)&SoundfontOscillator3::sub_process_template< SincInterpolator, PitchFixed, noPanning >;
				}
				break;

			case 4:
				if (usesPanning)
				{
					current_osc_func = (SubProcess_ptr)&SoundfontOscillator3::sub_process_template< CubicInterpolator, PitchFixed, Panning >;
				}
				else
				{
					current_osc_func = (SubProcess_ptr)&SoundfontOscillator3::sub_process_template< CubicInterpolator, PitchFixed, noPanning >;
				}
				break;

			case 0:
				if (usesPanning)
				{
					current_osc_func = (SubProcess_ptr)&SoundfontOscillator3::sub_process_template< NoInterpolator, PitchFixed, Panning >;
				}
				else
				{
					current_osc_func = (SubProcess_ptr)&SoundfontOscillator3::sub_process_template< NoInterpolator, PitchFixed, noPanning >;
				}
				break;

			default: // 2
				if (usesPanning)
				{
					current_osc_func = (SubProcess_ptr)&SoundfontOscillator3::sub_process_template< LinearInterpolator, PitchFixed, Panning >;
				}
				else
				{
					current_osc_func = (SubProcess_ptr)&SoundfontOscillator3::sub_process_template< LinearInterpolator, PitchFixed, noPanning >;
				}
				break;
			}
		}
	}
	else
	{
		current_osc_func = (SubProcess_ptr)&SoundfontOscillator3::sub_process_silence;
	}

	if (pinGate.isStreaming())
	{
		SET_PROCESS(&SoundfontOscillator3::process_with_gate);
	}
	else
	{
		SET_PROCESS(current_osc_func);
	}
}

void SoundfontOscillator3::ResetWave()
{
	if (sample_playing)
	{
		NoteDone(-1);
		NoteOn(-1);
	}
}

//inline bool is_denormal( float f )
//{
//	uint32_t l = *((uint32_t*)&f);
//
//	return( f != 0.f && (l & 0x7FF00000) == 0 && (l & 0x000FFFFF) != 0 ); // anything less than approx 1E-38 excluding +ve and -ve zero (two distinct values)
//}

// *** shared Interpolation filter setup ***
float* SoundfontOscillator3::GetInterpolationtable()
{
	if (interpolationTableInitialized)
		return interpolationTable;

	{
		// create a pointer to array with 'negative' indices
		interpolation_table2 = interpolationTable;
		// initialise interpolation table
		// as per page 523 MAMP
		// create vitual array with -ve indices

		// check not initialised already, entry 11 should be 0.801....
		assert(fabs(interpolation_table2[11] - .801) > .1);

		int i;
		const int table_width = INTERPOLATION_POINTS / 2;
		const int table_entries = (table_width + 1) * INTERPOLATION_DIV;
		const float PI = 3.14159265358979323846f;

		for (int sub_table = 0; sub_table < INTERPOLATION_DIV; sub_table++)
		{
			int table_index = sub_table * INTERPOLATION_POINTS + INTERPOLATION_POINTS / 2 - 1;
			for (int x = -table_width; x < table_width; x++)
			{
				i = sub_table + x * INTERPOLATION_DIV;
				// position on x axis
				double o = (double)i / INTERPOLATION_DIV;
				// filter impulse response
				double sinc = sin(PI * o) / (PI * o);

				// apply tailing function
				double hanning = cos(0.5 * PI * i / (INTERPOLATION_DIV * table_width));
				float windowed_sinc = (float)(sinc * hanning * hanning);

				assert((table_index - x) >= 0 && (table_index - x) < tableSize_);
				interpolation_table2[table_index - x] = windowed_sinc;
			}
		}
		assert((table_width - 1) >= 0 && (table_width - 1) < tableSize_);
		interpolation_table2[table_width - 1] = 1.f; // fix div by 0 bug

		// first table copied to last, shifted 1 place
		int idx = INTERPOLATION_DIV * INTERPOLATION_POINTS;
		for (int table_entry = 1; table_entry <= INTERPOLATION_POINTS; table_entry++)
		{
			assert((idx + table_entry) >= 0 && (idx + table_entry) < tableSize_);
			interpolation_table2[idx + table_entry] = interpolation_table2[table_entry - 1];
		}
		//		interpolation_table2[idx+INTERPOLATION_POINTS-1] = 0.f;
		assert((idx) >= 0 && (idx) < tableSize_);
		interpolation_table2[idx] = 0.f;

		/* print out interpolation table
				for( int sub_table = 0 ; sub_table <= INTERPOLATION_DIV ; sub_table++ )
				{
					for( int x = 0 ; x < INTERPOLATION_POINTS; x++ )
					{
						int table_index = sub_table * INTERPOLATION_POINTS + x;
						int linear_index = sub_table + INTERPOLATION_DIV * x;
						_RPT2(_CRT_WARN,"%d %+.5f\n",linear_index, interpolation_table2[table_index] );
					}
				}
		*/
		// additional fine tuning.  Normalise all 32 posible filters so total gain is always 1.0
		// This fixes 'overtones' problems, due to different sub-filters having slightly different overall gains
		for (int sub_table = 0; sub_table <= INTERPOLATION_DIV; sub_table++)
		{
			int table_index = sub_table * INTERPOLATION_POINTS;
			assert(table_index >= 0 && table_index < table_entries * 2);
			// calc sub table sum
			double fir_sum = 0.f;
			for (int table_entry = 0; table_entry < INTERPOLATION_POINTS; table_entry++)
			{
				fir_sum += interpolation_table2[table_index + table_entry];
			}

			// use it to normalise sub table
			for (int table_entry = 0; table_entry < INTERPOLATION_POINTS; table_entry++)
			{
				float adjusted = interpolation_table2[table_index + table_entry] / (float)fir_sum;
				if (fpclassify(adjusted) == FP_SUBNORMAL)
					adjusted = 0.f;
				assert((table_index + table_entry) >= 0 && (table_index + table_entry) < tableSize_);
				interpolation_table2[table_index + table_entry] = adjusted;
			}
		}
	}

	interpolationTableInitialized = true;
	return interpolationTable;
}
