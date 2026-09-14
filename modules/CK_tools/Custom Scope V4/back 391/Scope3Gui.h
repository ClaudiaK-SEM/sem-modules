#pragma once

//#include <chrono>
#include "../se_sdk3/mp_sdk_gui2.h"
#include "Scope3.h"
//#include "../se_sdk3/TimerManager.h"

class Scope3Gui :
	public gmpi_gui::MpGuiGfxBase//, public TimerClient
{

public:
Scope3Gui();

	virtual int32_t MP_STDCALL measure(GmpiDrawing_API::MP1_SIZE availableSize, GmpiDrawing_API::MP1_SIZE* returnDesiredSize);
	virtual int32_t MP_STDCALL OnRender(GmpiDrawing_API::IMpDeviceContext* drawingContext);
	void onValueChanged();
	//void DrawTrace(GmpiDrawing_API::IMpDeviceContext* drawingContext, signed char* capturedata, GmpiDrawing_API::IMpSolidColorBrush* pen, float mid_y, float scale, float width);

   //BlobGuiPin pinSamplesA;
	//BlobGuiPin pinPatchStoreIn;


private:

    BlobGuiPin pinSamplesA;
	BlobGuiPin pinPatchStoreIn;
//	MpFontInfo fontInfo_;
    int size0;

};
