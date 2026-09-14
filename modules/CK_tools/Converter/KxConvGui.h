#pragma once

#include "mp_sdk_gui.h"
#include "KxConvGui.h"
//#include "TimerManager.h"

class KxConvGui : public MpGuiBase
{
public:
	KxConvGui(IMpUnknown* host);


private:

	void onAChanged();
	void onBChanged();

    FloatGuiPin pinA;
    FloatGuiPin pinB;

};
