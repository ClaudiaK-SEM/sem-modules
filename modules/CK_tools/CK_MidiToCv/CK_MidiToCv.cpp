
#include <math.h>
#include "CK_MidiToCv.h"
#include "../shared/voice_allocation_modes.h"

SE_DECLARE_INIT_STATIC_FILE(CK_MidiToCv)
REGISTER_PLUGIN2 ( CK_MidiToCv, L"CK_MidiToCv" );

// New. 0v = 0.001s, 10V = 10s

#define VoltageToTime(v) ( powf( 10.0f,((v) * 0.4f ) - 3.0f ) )

CK_MidiToCv::CK_MidiToCv( ):
previousGate_(0.0f)
, currentGate_(0.0f)
{
	// Register pins.



	initializePin(pinMIDIIn);
	initializePin(pinChannel);
	initializePin(pinTrigger);
	initializePin(pinGate);
	initializePin(pinPitch);
	initializePin(pinVelocity);
	initializePin(pinAftertouchOut);

	initializePin(BlobToGui);
	initializePin(pinPolyphony);
	initializePin(pinPolyRes);
	initializePin(pinPolyMode);
	initializePin(pinMono);
	initializePin(pinRetrigger);
	initializePin(pinMonoPiority);
	initializePin(pinGlide);
	initializePin(pinGlideRate);
	initializePin(pinAutoGlide);
	initializePin(pinBRange);
	initializePin(pinVoiceRefresh);
	initializePin(pinMidiToCv);
	initializePin(pinPolyGlide);

	initializePin(pinVoiceActive);
	initializePin(pinVoiceGate);
	initializePin(pinVoiceTrigger);
	initializePin(pinVoiceVelocityKeyOn);
	initializePin(pinVoicePitch);
	initializePin(pinVoiceVirtualVoiceId);
	initializePin(pinBender);
	initializePin(pinHoldPedal);
	//initializePin(pinSostenutoPedal);
	initializePin(pinGlideStartPitch);
	initializePin(pinVoiceAllocationMode);
	initializePin(pinPortamento);
	initializePin(pinBenderRange);
	initializePin(pinVoiceAftertouch);
	initializePin(pinChannelPressure);
	//initializePin(pinVoiceBender);
}

int32_t CK_MidiToCv::open()
{
	MpBase2::open();	// always call the base class

	pinVelocity.setCurveType(SmartAudioPin::Curved);

	SET_PROCESS2(&CK_MidiToCv::subProcess);

	pinTrigger.setTransitionTime(getSampleRate() * 0.0005f); // 0.5 ms trigger pulse.
	benderInterpolator_.Init(getSampleRate());
	pinAftertouchOut.setCurveType(SmartAudioPin::LinearAdaptive);

    init=1;
    BlobOn=1;
	return gmpi::MP_OK;
}

void CK_MidiToCv::subProcess( int sampleFrames )
{
	auto bufferOffset = getBlockPosition();
	if( pinPitch.isStreaming() && pitchInterpolator_.isDone() && benderInterpolator_.isDone() )
	{
		pinPitch.setStreaming(false, bufferOffset);
	}

	float* gate = getBuffer(pinGate);
	float* pitch = getBuffer(pinPitch);

	for( int s = sampleFrames; s > 0; s-- )
	{
		*gate++ = currentGate_;
		*pitch++ = pitchInterpolator_.getNext() + benderInterpolator_.getNext();
	}

	bool canSleepUnused = true;
	pinVelocity.subProcess(bufferOffset, sampleFrames, canSleepUnused);
	pinTrigger.subProcess(bufferOffset, sampleFrames, canSleepUnused);
	pinAftertouchOut.subProcess(bufferOffset, sampleFrames, canSleepUnused);
}

void CK_MidiToCv::CleanVelocityAndAftertouch()
{
	// Voice needs clean start, no Velocity Smoothing.
	// assume velocity changes on exact same sample as reset.
	pinVelocity.setValueInstant(0.1f * pinVoiceVelocityKeyOn); // 0.1 to convert to Volts.

	if(usePolyAftertouch())
	{
		pinAftertouchOut.setValueInstant(0.1f * pinVoiceAftertouch);
	}
	else
	{
		pinAftertouchOut.setValueInstant(pinChannelPressure);
	}
}

void CK_MidiToCv::onSetPins()
{


    float GlideM=1.0f;
    float autoG = float(pinAutoGlide);
    float timeG = float(pinGlideRate);
    if(pinMidiToCv==1)
    {
    GlideM=0.85f;
    if(autoG==0.0f)autoG=1.0f;
    else
    autoG=0.0f;
    if(timeG==0.0f)timeG=1.0f;
    else
    timeG=0.0f;
    }

    if(pinPolyGlide==0 && pinMono==0)GlideM=0.0f;

    if(pinMidiToCv.isUpdated() || pinMono.isUpdated() || pinRetrigger.isUpdated() || pinMonoPiority.isUpdated() || pinPolyMode.isUpdated() || pinGlideRate.isUpdated() || pinAutoGlide.isUpdated() || pinVoiceRefresh.isUpdated())
    {
    p[0]=12;
    BlobOn=1;
    }
	if(pinPolyphony.isUpdated() || pinPolyRes.isUpdated())
    {
    p[0]=13;
    BlobOn=1;
    }
	if(pinGlide.isUpdated())
    {
    p[0]=7;
    BlobOn=1;
    }
	if(pinBRange.isUpdated())
    {
    p[0]=10;
    BlobOn=1;
    }
	if(init==1)
    {
    p[0]=14;
    init=0;
    BlobOn=1;
    }



	if(BlobOn==1)
	{
    BlobOn=0;
    p[1]=float(pinMono);
    p[2]=float(pinRetrigger);
    p[3]=float(pinMonoPiority);
    p[4]=float(pinPolyphony);
    p[5]=float(pinPolyRes);
    p[6]=float(pinPolyMode);
    p[7]=pinGlide*GlideM;
    //p[8]=float(pinGlideRate);
    p[8]=timeG;
    //p[9]=float(pinAutoGlide);
	p[9]=autoG;
    p[10]=float(pinBRange);
    p[11]=float(pinVoiceRefresh);

    BlobToGui.setValueRaw( sizeof(p), &p );
    BlobToGui.sendPinUpdate();
    }




	// Actual value of trigger signal is garbage. All that matters is that it updated.
	const bool triggered = pinVoiceTrigger.isUpdated() && pinVoiceActive == 1.0f; // pinVoiceActive is checked to avoid a spurious pulse on first sample.

	if( triggered )
	{
		pinTrigger.pulse(1.0f);
	}

	// Voice reset happens at hard note ons, unless mono mode is on and a voice is already sounding. (not suspended)
	bool hardReset = false;
	if( pinVoiceActive.isUpdated() )
	{
		if( pinVoiceActive > 0.0f )
		{
			if (pinVoiceActive < 1.0f) // voiceActive of 0.5 indicates overlap voice, don't sustain.
			{
				if (currentGate_ != 0.0f)
				{
					currentGate_ = 0.0f;
					pinGate.setUpdated();
				}
			}
			else
			{
				hardReset = true;
				CleanVelocityAndAftertouch();

				// Gate does notr nesc change on stolen voice when poly=1. Force non-glide.
				previousGate_ = 0;
			}
		}
	}

	// CC 64 (HoldPedal, mono) or CC 66 (Sostenuto, per-voice): a transition on either may finally
	// allow a deferred gate-off to take effect. Held = either pedal still pressed for this voice.
	if( pinHoldPedal.isUpdated())
	//if( pinHoldPedal.isUpdated() || pinSostenutoPedal.isUpdated() )
	{
		//const bool held = pinHoldPedal >= 5.0f || pinSostenutoPedal >= 5.0f;
		const bool held = pinHoldPedal >= 5.0f;
		if (currentGate_ != 0.0f && pinVoiceGate == 0.0f && (!held || pinVoiceActive < 1.0f))
		{
			currentGate_ = 0.0f;
			pinGate.setUpdated();
		}
	}

	// PITCH.
	bool pitchUpdated = false;
	//if (pinVoiceBender.isUpdated() || pinBender.isUpdated() || pinBenderRange.isUpdated())
	if (pinBender.isUpdated() || pinBenderRange.isUpdated())
	{

		constexpr float benderRangeScale = 1.0f / 120.0f;
		// voice bender is hard-coded to 48 semitones (for MPE)
		//const float totalBend = pinVoiceBender * 0.05f + pinBender * pinBenderRange * benderRangeScale;
		const float totalBend = pinBender * pinBenderRange * benderRangeScale;
		benderInterpolator_.setTarget(totalBend);
		pitchUpdated = true;
	}

	if( pinMidiToCv.isUpdated() || pinGlide.isUpdated() || pinVoiceAllocationMode.isUpdated() )
	//if( pinPortamento.isUpdated() || pinVoiceAllocationMode.isUpdated() )
	{
		const bool constantTimeGlide = 0 != voice_allocation::extractBits(pinVoiceAllocationMode, voice_allocation::bits::GlideRate_startbit, 1);

		if( constantTimeGlide )
		{

			//pitchInterpolator_.setTransitionTime(getSampleRate() * VoltageToTime(pinPortamento));
			pitchInterpolator_.setTransitionTime(getSampleRate() * VoltageToTime(pinGlide*GlideM));
		}
	}

	// glide start is only needed on fresh voice activations.
	// else it's a soft-steal and current pitch should already be correct.
	if (hardReset)
	{
		pitchInterpolator_.setValueInstant(0.1f * pinGlideStartPitch);
		// fix issue of MPE pitch blip on note-on due to benderInterpolator not done from previous note.
		benderInterpolator_.jumpToTarget();
	}

	// If pinVoicePitch updated, glide to that value.
	if (pinVoicePitch.isUpdated() || hardReset)
	{
		// for constant RATE glide, first recalc glide time.
		const bool constantRateGlide = GT_CONST_RATE == voice_allocation::extractBits(pinVoiceAllocationMode, voice_allocation::bits::GlideRate_startbit, 1);
		if (constantRateGlide)
		{
			const float deltaPitch = fabsf(pinGlideStartPitch - pinVoicePitch);
			//pitchInterpolator_.setTransitionTime(getSampleRate() * VoltageToTime(pinPortamento) * deltaPitch);
			pitchInterpolator_.setTransitionTime(getSampleRate() * VoltageToTime(pinGlide*GlideM) * deltaPitch);
		}

		pitchInterpolator_.setTarget(0.1f * pinVoicePitch);
		pitchUpdated = true;
	}

	// isFirstSample prevents pitch gliding gradually from zero (pinGlideStartPitch) at startup.
	if (isFirstSample)
	{
        pitchInterpolator_.jumpToTarget();
		pitchUpdated = true;
	}

	if (pitchUpdated)
	{
		pinPitch.setStreaming(!pitchInterpolator_.isDone() || !benderInterpolator_.isDone());
	}

	bool legato = true;

	if( pinVoiceGate.isUpdated() )
	{
		if( pinVoiceGate != 0.0f )
		{
			currentGate_ = 1.0f;
		}
		else
		{
			// Don't drop gate while either pedal holds this voice. voiceActive 0.5 = overlap voice — ignore pedals.
			//const bool held = pinHoldPedal >= 5.0f || pinSostenutoPedal >= 5.0f;
			const bool held = pinHoldPedal >= 5.0f;
			if (!held || pinVoiceActive < 1.0f)
			{
				currentGate_ = 0.0f;
			}
		}

		pinGate.setStreaming(false);

		// gate transition to HI indicates note-on (not legato.
		if( previousGate_ == 0.0f && pinVoiceGate != 0.0f )
		{
			legato = false;
		}

		previousGate_ = pinVoiceGate;
	}

	if( pinVoiceVelocityKeyOn.isUpdated() )
	{
		// no point setting it again if we just did.
		if( !pinVoiceActive.isUpdated() || pinVoiceActive <= 0.0f )
		{
			float newVelocity = 0.1f * pinVoiceVelocityKeyOn; // convert to Volts.

			if( legato ) // glide smoothly to new velocity.
			{
				pinVelocity.setTransitionTime(getSampleRate() * 0.008f); // 8ms
			}
			else // re-play decaying note, move to new velocity as quickly as pos to keep attack snappy.
			{
				pinVelocity.setTransitionTime(getSampleRate() * 0.0015f); // 1.5ms
			}

			pinVelocity = newVelocity;
		}
	}

	if( pinVoiceAftertouch.isUpdated())
	{
		polyAftertouchDetected |= (pinVoiceAftertouch > 0.0f);

		if( usePolyAftertouch() )
		{
			pinAftertouchOut.setValue(0.1f * pinVoiceAftertouch); // voice aftertouch range is 0 - 10
		}
	}

	if( pinChannelPressure.isUpdated() )
	{
		monoAftertouchDetected |= (pinChannelPressure > 0.0f);
		if(!usePolyAftertouch() )
		{
			pinAftertouchOut.setValue(pinChannelPressure); // channel aftertouch range is 0 - 1
		}
	}

	isFirstSample = false;
}

