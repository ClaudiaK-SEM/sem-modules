#include "v6_PolyphonyControlGui.h"

using namespace gmpi;
using namespace gmpi_gui;


GMPI_REGISTER_GUI(MP_SUB_TYPE_GUI2, v6_PolyphonyControlGui1, L"6v Polyphony Control");


v6_PolyphonyControlGui1::v6_PolyphonyControlGui1()
{

	//initializePin(hostPolyphony);
	initializePin(hostReserveVoices);
	//initializePin(hostPolyphony, static_cast<MpGuiBaseMemberPtr2>( &VrPolyphonyControlGui_base::onSetHostPolyphony ));
	//initializePin(hostReserveVoices, static_cast<MpGuiBaseMemberPtr2>( &VrPolyphonyControlGui_base::onSetHostPolyphonyReserve ));
	//initializePin(hostVoiceAllocationMode, static_cast<MpGuiBaseMemberPtr2>( &VrPolyphonyControlGui1::onSetHostVoiceAllocationMode ));

	//initializePin(polyphony, static_cast<MpGuiBaseMemberPtr2>( &VrPolyphonyControlGui_base::onSetPolyphony ));
	//initializePin(itemList3);
	//initializePin(reserveVoices);
    //initializePin(reserveVoices, static_cast<MpGuiBaseMemberPtr2>( &VrPolyphonyControlGui_base::onSetPolyphonyReserve ));
	//initializePin(itemList4);
	//initializePin(voiceAllocationMode, static_cast<MpGuiBaseMemberPtr2>( &VrPolyphonyControlGui1::onSetVoiceAllocationMode ));
	//initializePin(itemList_voiceAllocationMode);

	//initializePin(monoNotePriority, static_cast<MpGuiBaseMemberPtr2>( &VrPolyphonyControlGui_base::onSetNotePriority ));
	//initializePin(itemList2);

    //initializePin(VoiceRefresh, static_cast<MpGuiBaseMemberPtr2>(&VrPolyphonyControlGui1::onSetVoiceRefresh));
	//initializePin(itemList_VoiceRefresh);

}

int32_t v6_PolyphonyControlGui_base::initialize()
{
	//itemList2 = L"Off,Low,High,Last";
	//itemList3 = L"range 1,128";
	//itemList4 = L"range 0,128";
    //hostPolyphony = 6;
	hostReserveVoices=0;
	return MpGuiInvisibleBase::initialize();
}

