#ifndef CK_MidiToCv_H_INCLUDED
#define CK_MidiToCv_H_INCLUDED

#include "mp_sdk_audio.h"
#include "smart_audio_pin.h"

using namespace gmpi;


class CK_MidiToCv : public MpBase2
{
public:
	CK_MidiToCv( );
	int32_t open() override;
    void subProcess(int sampleFrames);
	void onSetPins() override;

private:
	void CleanVelocityAndAftertouch();

    int init;
    int BlobOn;

	MidiInPin pinMIDIIn;
	IntInPin pinChannel;
	SmartAudioPin pinTrigger;
	SmartAudioPin pinGate;
	AudioOutPin pinPitch;
	SmartAudioPin pinVelocity;
	SmartAudioPin pinAftertouchOut;

    BlobOutPin BlobToGui;
	IntInPin pinPolyphony;
	IntInPin pinPolyRes;
	IntInPin pinPolyMode;
	IntInPin pinMono;
	IntInPin pinRetrigger;
	IntInPin pinMonoPiority;
	FloatInPin pinGlide;
	IntInPin pinGlideRate;
	IntInPin pinAutoGlide;
	IntInPin pinBRange;
	IntInPin pinVoiceRefresh;
	IntInPin pinMidiToCv;
	IntInPin pinPolyGlide;

	float p[12];

	FloatInPin pinVoiceActive;
	FloatInPin pinVoiceGate;
	FloatInPin pinVoiceTrigger;
	FloatInPin pinVoiceVelocityKeyOn;
	FloatInPin pinVoicePitch;
	//FloatInPin pinVoiceBender;

	FloatInPin pinVoiceAftertouch;
	FloatInPin pinChannelPressure;
	IntInPin pinVoiceVirtualVoiceId;
	FloatInPin pinBender;
	FloatInPin pinBenderRange;
	FloatInPin pinHoldPedal;
	//FloatInPin pinSostenutoPedal;
	FloatInPin pinGlideStartPitch;
	IntInPin pinVoiceAllocationMode;
	FloatInPin pinPortamento;

	RampGenerator pitchInterpolator_;
	RampGeneratorAdaptive benderInterpolator_;

	float previousGate_;
	float currentGate_;

	// by default use poly-aftertouch, use mono-aftertouch (channel-pressure) as a last-resort.
	bool monoAftertouchDetected = {};
	bool polyAftertouchDetected = {};
	bool usePolyAftertouch() const
	{
		return polyAftertouchDetected || !monoAftertouchDetected;
	}
	bool isFirstSample = true;

};

#endif

