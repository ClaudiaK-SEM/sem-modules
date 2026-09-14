#pragma once

#include "mp_sdk_gui.h"
#include "KxVoltToHz.h"
//#include "TimerManager.h"

class KxVoltToHzGui : public MpGuiBase
{
public:
	KxVoltToHzGui(IMpUnknown* host);


private:

	void onValueChanged();

	FloatGuiPin pinFromDSP;
	//FloatGuiPin pinToDSP;

    IntGuiPin pinIntDO;
	StringGuiPin pinTextDO;//////////////////
	FloatGuiPin pinFloatDO;
    //IntGuiPin pinIntDI;
	//StringGuiPin pinTextDI;
	//FloatGuiPin pinFloatDI;

    IntGuiPin pinMode;//////////////////////
    FloatGuiPin pinThreshold;

	//MpFontInfo fontInfo_;
    //int ct;
};
