#pragma once

#include "mp_sdk_audio.h"

using namespace gmpi;

class CK_Tool1 : public MpBase
{
public:
    CK_Tool1();
    virtual ~CK_Tool1(){};

    virtual int32_t MP_STDCALL onSetPins();

private:
    // Audio pins
    AudioInPin pinAudioIn;
    AudioOutPin pinAudioOut;

    // Parameter pins
    FloatInPin pinParam1;
};
