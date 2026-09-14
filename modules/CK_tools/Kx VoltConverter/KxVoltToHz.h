// Copyright 2006 Jeff McClintock

#ifndef KxVoltToHz_dsp_H_INCLUDED
#define KxVoltToHz_dsp_H_INCLUDED

#include "mp_sdk_audio.h"

class KxVoltToHz: public MpBase
{
public:
	KxVoltToHz(IMpUnknown* host);
	//virtual int32_t MP_STDCALL open();
	//void subProcess(int bufferOffset, int sampleFrames);
	void onSetPins(void);

private:

    FloatOutPin pinToGUI;
    //FloatInPin pinFromGUI;
   	AudioInPin pinVolt;
    FloatInPin pinFloat;
    IntInPin pinInt;
    FloatOutPin pinFloatOut;
    IntInPin pinMode;
    float m_volt;
    float m_float;
    int m_int;
};

#endif
