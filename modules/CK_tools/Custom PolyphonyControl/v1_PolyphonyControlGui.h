#ifndef V1_POLYPHONYCONTROLGUI_H_INCLUDED
#define V1_POLYPHONYCONTROLGUI_H_INCLUDED

#include "../se_sdk3/mp_sdk_gui2.h"

class v1_PolyphonyControlGui_base : public gmpi_gui::MpGuiInvisibleBase
{
public:
//	PolyphonyControlGui_base();
	virtual int32_t MP_STDCALL initialize() override;

	//void onSetPolyphony();
	//void onSetHostPolyphony();
	//void onSetNotePriority();
	//void onSetPolyphonyReserve();
	//void onSetHostPolyphonyReserve();

	IntGuiPin hostPolyphony;
	IntGuiPin hostReserveVoices;
	//IntGuiPin hostVoiceAllocationMode;

	//IntGuiPin polyphony;
	//IntGuiPin reserveVoices;
	//IntGuiPin monoNotePriority;
	//StringGuiPin itemList2;
	//StringGuiPin itemList3;
	//StringGuiPin itemList4;
};

class v1_PolyphonyControlGui1 : public v1_PolyphonyControlGui_base
{
	//IntGuiPin voiceAllocationMode;
	//StringGuiPin itemList_voiceAllocationMode;
	//IntGuiPin VoiceRefresh;
	//StringGuiPin itemList_VoiceRefresh;
public:
	v1_PolyphonyControlGui1();
	//virtual int32_t MP_STDCALL initialize() override;

	//void onSetVoiceRefresh();

	//void onSetHostVoiceAllocationMode();
	//void onSetVoiceAllocationMode();
};

#endif
