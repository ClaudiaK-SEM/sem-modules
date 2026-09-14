#include "./CK_Tool2.h"

SE_DECLARE_INIT_STATIC_FILE(CK_tools_xml)

namespace
{
SE_REGISTER_CLASS
(
CK_Tool2
, L"CK Tool2"
, SE_GROUP_NONE
, L"Tool"
, prodID_CK_Tool2
, 1, 0 // version
, pinAudioIn
, pinAudioOut
, pinParam1
);
}

CK_Tool2::CK_Tool2()
{
    // Initialize pins
    initializePin( pinAudioIn );
    initializePin( pinAudioOut );
    initializePin( pinParam1, 1.0f );
}

int32_t CK_Tool2::onSetPins()
{
    // Set audio output from input (placeholder)
    pinAudioOut.setValueAddress( pinAudioIn.getBuffer() );
    return gmpi::MP_OK;
}
