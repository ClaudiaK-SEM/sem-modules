#ifndef CK_MidiToCvGUI_H_INCLUDED
#define CK_MidiToCvGUI_H_INCLUDED

#include "../se_sdk3/mp_sdk_gui2.h"

class CK_MidiToCvGui2 : public gmpi_gui::MpGuiInvisibleBase
{
public:
	CK_MidiToCvGui2();
//	virtual int32_t MP_STDCALL initialize() override;

	BlobGuiPin BlobToGui;
	IntGuiPin hostPolyphony;
	IntGuiPin hostReserveVoices;
	IntGuiPin hostVoiceAllocationMode;
	FloatGuiPin host_PortamentoTime;
	FloatGuiPin hostBendRange;

private:

	void onSetBlobToGui();

	void onSetPolyphony();
	int polyphony;

	void onSetNotePriority();
	int monoNotePriority;

	void onSetPolyphonyReserve();
    int reserveVoices;

	void onSetPolyAndReserve();

	void onSetMonoMode();
	int monoMode;
	int monoRetrigger;

	void onSetVoiceStealMode();
	int voiceStealMode;

	void onSetGlide();
    int GlideType;

	void onSetGlideTiming();
    int GlideTiming;


	void onSetPortamento();
	float PortamentoTime;

	void onSetBendRange();
	float BendRange;

	void onSetVoiceRefresh();
	int VoiceRefresh;

	void onSetVoiceAllocation();

};


#endif
