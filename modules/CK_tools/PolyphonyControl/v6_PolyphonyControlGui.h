#ifndef V6_POLYPHONYCONTROLGUI_H_INCLUDED
#define V6_POLYPHONYCONTROLGUI_H_INCLUDED

#include "../se_sdk3/mp_sdk_gui2.h"

class v6_PolyphonyControlGui_base : public gmpi_gui::MpGuiInvisibleBase
{
public:
//	VrPolyphonyControlGui_base();
	virtual int32_t MP_STDCALL initialize() override;

	//void onSetPolyphony();
	//void onSetHostPolyphony();
	//void onSetNotePriority();
	//void onSetPolyphonyReserve();
	//void onSetHostPolyphonyReserve();

	//IntGuiPin hostPolyphony;
	IntGuiPin hostReserveVoices;
	//IntGuiPin hostVoiceAllocationMode;

	//IntGuiPin polyphony;
	//IntGuiPin reserveVoices;
	//IntGuiPin monoNotePriority;
	//StringGuiPin itemList2;
	//StringGuiPin itemList3;
	//StringGuiPin itemList4;
};

class v6_PolyphonyControlGui1 : public v6_PolyphonyControlGui_base
{
	//IntGuiPin voiceAllocationMode;
	//StringGuiPin itemList_voiceAllocationMode;
	//IntGuiPin VoiceRefresh;
	//StringGuiPin itemList_VoiceRefresh;
public:
	v6_PolyphonyControlGui1();
	//virtual int32_t MP_STDCALL initialize() override;

	//void onSetVoiceRefresh();

	//void onSetHostVoiceAllocationMode();
	//void onSetVoiceAllocationMode();
};


#endif
