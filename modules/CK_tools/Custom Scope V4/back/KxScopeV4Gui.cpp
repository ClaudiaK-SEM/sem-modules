// prevent MS CPP - 'swprintf' was declared deprecated warning
#if defined(_MSC_VER)
  #define _CRT_SECURE_NO_DEPRECATE
  #pragma warning(disable : 4996)
#endif

#include "KxScopeV4Gui.h"
#include <algorithm>
#ifndef _WIN32
#include <sys/time.h>                // for gettimeofday()
#endif

using namespace gmpi;
using namespace gmpi_gui;
using namespace GmpiDrawing;
using namespace GmpiDrawing_API;
static const float m = 0.0078128f;//1/256


#undef DrawText

GMPI_REGISTER_GUI(MP_SUB_TYPE_GUI2, KxScopeV4Gui, L"KX SCOPE V4");

#ifndef _WIN32
int32_t timeGetTime(void) // MacOS version
{
    timeval t1;

    // start timer
    gettimeofday(&t1, NULL);
    return (int32_t)(t1.tv_sec * 1000.0);
}
#endif
KxScopeV4Gui::KxScopeV4Gui()
{
	initializePin( 0, pinSamplesA, static_cast<MpGuiBaseMemberPtr2>( &KxScopeV4Gui::onValueChanged ) );
    initializePin( 1, pinPatchStoreIn );

    size0 = SCOPE_BUFFER_SIZE;
    lumhi=230;
    lumlo=int(230/1.5f);






}
void KxScopeV4Gui::onValueChanged()
{
	invalidateRect();
}
/*void KxScopeV4Gui::DrawTrace(GmpiDrawing_API::IMpDeviceContext* drawingContext, signed char* capturedata, IMpSolidColorBrush* pen, float mid_y, float scale, float width)
{
	const float penWidth = 1;

	gmpi_sdk::mp_shared_ptr<GmpiDrawing_API::IMpFactory> factory;
	drawingContext->GetFactory(&factory.get());

	gmpi_sdk::mp_shared_ptr<IMpPathGeometry> geometry;
	factory->CreatePathGeometry(&geometry.get());
//	drawingContext->CreatePathGeometry(&geometry.get());

	gmpi_sdk::mp_shared_ptr<IMpGeometrySink> sink;
	geometry->Open(&sink.get());

	sink->BeginFigure(GmpiDrawing::Point(0.f, mid_y - (*capturedata * scale*m)), MP1_FIGURE_BEGIN_HOLLOW);

	for (int i = 1; i < SCOPE_BUFFER_SIZE; i++)
	{
		//signed char x = (i * width) / SCOPE_BUFFER_SIZE;

		float x = (i * width) / size0;
		sink->AddLine(GmpiDrawing::Point(x, mid_y - (*capturedata * scale*m)));
		//if(x<=width)sink->AddLine(GmpiDrawing::Point(x, mid_y - (*capturedata * scale*m)));
		++capturedata;
	}

	sink->EndFigure(MP1_FIGURE_END_OPEN);
	sink->Close();

	drawingContext->DrawGeometry(geometry, pen, penWidth);
}*/

int32_t KxScopeV4Gui::OnRender(GmpiDrawing_API::IMpDeviceContext* drawingContext)
{

#ifdef _DEBUG
	assert(debugInitializeCheck_);
//	assert(debug_IsMeasured);
	assert(debug_IsArranged);
#endif

	GmpiDrawing::Graphics dc2(drawingContext);
	auto dc = dc2.Get();

	GmpiDrawing::Rect r = getRect();//

	//std::chrono::steady_clock::time_point showUpdatesAfter = std::chrono::steady_clock::now() - std::chrono::milliseconds(500); // timeGetTime() - 500; // show any trace updated in last 0.5 seconds.
	float width = r.right - r.left;
	float height = r.bottom - r.top;

//TODO	getGuiHost()->getFontInfo( L"tty", fontInfo_, font_handle );
//	fontInfo_.color = 0x00ff00;
//	fontInfo_.colorBackground = 0x000044;

	unsigned int color = 0x7f7f7f;
	unsigned int colorBackground = 0x000000;

	float scale = height / 2;
	float mid_x = width / 2;
	float mid_y = height / 2;

	/*if( true) // fontInfo_.colorBackground >= 0 ) // -1 indicates transparent background
	{
		// Fill in solid background black
		auto background_brush = dc2.CreateSolidColorBrush(Color::Black);
		dc2.FillRectangle(r, background_brush);
	}*/




    if(pinPatchStoreIn.rawSize() == sizeof(int) * 3 )
    {
       int* intValue = (int*) pinPatchStoreIn.rawData();
       if(intValue[0] >=11  &&  intValue[0] <=SCOPE_BUFFER_SIZE)
       {
       size0=intValue[0];
       //float msec=float(intValue[1])/1000.0f;
       //float time=size0/msec;
       //float hz=1000.0f/time;
       //swprintf(txt_error,L"  Time %3.3f ms - Freq %3.3f hz - Buffer %i spl",time,hz,size0);
       //swprintf(txt_error,L" %i x  %i y",width,height);
       }
       if(intValue[2] >=0  &&  intValue[2] <=255)
       {
       lumhi=intValue[2];
       lumlo=int(lumhi/1.5f);
       if(lumlo<=0)lumlo=0;

       //float msec=float(intValue[1])/1000.0f;
       //float time=size0/msec;
       //float hz=1000.0f/time;
       //swprintf(txt_error,L"  Time %3.3f ms - Freq %3.3f hz - Buffer %i spl",time,hz,size0);
       //swprintf(txt_error,L" %i x  %i y",width,height);
       }
    }

	///////////////////////////
	// create a green brush2
//	uint32_t darked_col = (fontInfo_.color >> 1) &0x7f7f7f;
	uint32_t darked_col = (color >> 1) & 0x7f7f7f;
/*
	gmpi_sdk::mp_shared_ptr<IMpSolidColorBrush> brush2;
	dc->CreateSolidColorBrush(&Color(darked_col), &brush2.get());
    //brush2->SetColor(&Color::FromBytes(76, 90, 100)); // green.
    brush2->SetColor(&Color::FromBytes(lumlo, lumlo, lumlo)); // green.
*/
	gmpi_sdk::mp_shared_ptr<IMpSolidColorBrush> brush3;
	dc->CreateSolidColorBrush(&Color(darked_col), &brush3.get());
    //brush3->SetColor(&Color::FromBytes(186, 200, 210)); // green.
    brush3->SetColor(&Color::FromBytes(lumhi, lumhi, lumhi)); // green.
//#if defined(INTEL_HD)

   // brush2->SetColor(&Color::FromBytes(170, 170, 170)); // green.
   // brush3->SetColor(&Color::FromBytes(255, 255, 255)); // green.

//#else

    //brush2->SetColor(&Color::FromBytes(76, 90, 100)); // green.
    //brush3->SetColor(&Color::FromBytes(186, 200, 210)); // green.

//#endif




	float penWidth = 1.0f;
	//dc->DrawLine(GmpiDrawing::Point(0, mid_y), GmpiDrawing::Point(width, mid_y), brush3, penWidth);



		if( pinSamplesA.rawSize() == sizeof(signed char) * SCOPE_BUFFER_SIZE )
		{


			signed char* capturedata = (signed char*)pinSamplesA.rawData();

				//float x2=0.0f;
				for (int i = 1; i < size0-1; i++)
                {
                    float x = (i * width) / size0;
                    float y = mid_y - (*capturedata * scale*m);
                    dc->DrawLine(GmpiDrawing::Point(x, mid_y), GmpiDrawing::Point(x,  y), brush3, penWidth);
                    //dc->DrawLine(GmpiDrawing::Point(x2, y), GmpiDrawing::Point(x,  y), brush3, penWidth);
                    //x2=x;
                    ++capturedata;
                }


		}

	return gmpi::MP_OK;

}
int32_t KxScopeV4Gui::measure(GmpiDrawing_API::MP1_SIZE availableSize, GmpiDrawing_API::MP1_SIZE* returnDesiredSize)
{
	const float minSize = 15;

	returnDesiredSize->width = (std::max)(minSize, availableSize.width);
	returnDesiredSize->height = (std::max)(minSize, availableSize.height);

	return gmpi::MP_OK;
}
