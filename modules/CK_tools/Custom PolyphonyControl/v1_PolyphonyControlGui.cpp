#include "v1_PolyphonyControlGui.h"

using namespace gmpi;
using namespace gmpi_gui;

GMPI_REGISTER_GUI(MP_SUB_TYPE_GUI2, v1_PolyphonyControlGui1, L"1v0r Polyphony Control");
;

v1_PolyphonyControlGui1::v1_PolyphonyControlGui1()
//PolyphonyControlGui_base::PolyphonyControlGui_base()
{
	//initializePin(hostPolyphony, static_cast<MpGuiBaseMemberPtr2>( &PolyphonyControlGui_base::onSetHostPolyphony ));
	initializePin(hostPolyphony);
	//initializePin(hostReserveVoices, static_cast<MpGuiBaseMemberPtr2>( &PolyphonyControlGui_base::onSetHostPolyphonyReserve ));
	initializePin(hostReserveVoices);

	//initializePin(hostVoiceAllocationMode, static_cast<MpGuiBaseMemberPtr2>( &PolyphonyControlGui1::onSetHostVoiceAllocationMode ));

	//initializePin(polyphony, static_cast<MpGuiBaseMemberPtr2>( &PolyphonyControlGui_base::onSetPolyphony ));
    //initializePin(polyphony);
	//initializePin(itemList3);

	//initializePin(reserveVoices, static_cast<MpGuiBaseMemberPtr2>( &PolyphonyControlGui_base::onSetPolyphonyReserve ));
	//initializePin(reserveVoices);
	//initializePin(itemList4);

	//initializePin(voiceAllocationMode, static_cast<MpGuiBaseMemberPtr2>( &PolyphonyControlGui1::onSetVoiceAllocationMode ));
	//initializePin(itemList_voiceAllocationMode);

	//initializePin(monoNotePriority, static_cast<MpGuiBaseMemberPtr2>( &PolyphonyControlGui_base::onSetNotePriority ));
	//initializePin(itemList2);

    //initializePin(VoiceRefresh, static_cast<MpGuiBaseMemberPtr2>(&PolyphonyControlGui1::onSetVoiceRefresh));
	//initializePin(itemList_VoiceRefresh);

}

int32_t v1_PolyphonyControlGui_base::initialize()
{
	//itemList2 = L"Off,Low,High,Last";
	//itemList3 = L"range 1,128";
	//itemList4 = L"range 0,128";
    hostPolyphony = 1;
	hostReserveVoices=0;
	return MpGuiInvisibleBase::initialize();
}
