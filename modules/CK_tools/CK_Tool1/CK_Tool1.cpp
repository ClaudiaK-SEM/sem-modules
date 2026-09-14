#include "./CK_Tool1.h"

SE_DECLARE_INIT_STATIC_FILE(CK_tools_xml)

namespace
{
SE_REGISTER_CLASS
(
CK_Tool1
, L"CK Tool1"
, SE_GROUP_NONE
, L"Tool"
, prodID_CK_Tool1
, 1, 0 // version
, pinAudioIn
, pinAudioOut
, pinParam1
);
}

CK_Tool1::CK_Tool1()
{
    // Initialize pins
    initializePin( pinAudioIn );
    initializePin( pinAudioOut );
    initializePin( pinParam1, 1.0f );
}

int32_t CK_Tool1::onSetPins()
{
    // Set audio output from input (placeholder)
    pinAudioOut.setValueAddress( pinAudioIn.getBuffer() );
    return gmpi::MP_OK;
}
