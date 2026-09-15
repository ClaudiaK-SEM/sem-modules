// prevent MS CPP - 'swprintf' was declared deprecated warning
#if defined(_MSC_VER)
  #define _CRT_SECURE_NO_DEPRECATE
  #pragma warning(disable : 4996)
#endif

#include "Scope3Gui.h"
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

GMPI_REGISTER_GUI(MP_SUB_TYPE_GUI2, Scope3Gui, L"KX SCOPE V4");

#ifndef _WIN32
int32_t timeGetTime(void) // MacOS version
{
    timeval t1;

    // start timer
    gettimeofday(&t1, NULL);
    return (int32_t)(t1.tv_sec * 1000.0);
}
#endif
Scope3Gui::Scope3Gui()
{
	initializePin( 0, pinSamplesA, static_cast<MpGuiBaseMemberPtr2>( &Scope3Gui::onValueChanged ) );
    initializePin( 1, pinPatchStoreIn );
}
void Scope3Gui::onValueChanged()
{
	invalidateRect();
}
/*void Scope3Gui::DrawTrace(GmpiDrawing_API::IMpDeviceContext* drawingContext, signed char* capturedata, IMpSolidColorBrush* pen, float mid_y, float scale, float width)
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

int32_t Scope3Gui::OnRender(GmpiDrawing_API::IMpDeviceContext* drawingContext)
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

	float scale = height / 2.15f;
	float mid_x = width / 2;
	float mid_y = height / 2;

	/*if( true) // fontInfo_.colorBackground >= 0 ) // -1 indicates transparent background
	{
		// Fill in solid background black
		auto background_brush = dc2.CreateSolidColorBrush(Color::Black);
		dc2.FillRectangle(r, background_brush);
	}*/

    size0 = SCOPE_BUFFER_SIZE;

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
       lumlo=intValue[2]/1.5f;
       if(lumlo<=23)lumlo=23;

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

	/*gmpi_sdk::mp_shared_ptr<IMpSolidColorBrush> brush;
	dc->CreateSolidColorBrush(&Color(darked_col), &brush.get());
    brush->SetColor(&Color::FromBytes(23, 23, 23)); // green.*/


	/*gmpi_sdk::mp_shared_ptr<IMpSolidColorBrush> brush2;
	dc->CreateSolidColorBrush(&Color(darked_col), &brush2.get());
    //brush2->SetColor(&Color::FromBytes(23, 23, 23)); // green.
    brush2->SetColor(&Color::FromBytes(lumlo, lumlo, lumlo)); // green.

	gmpi_sdk::mp_shared_ptr<IMpSolidColorBrush> brush3;
	dc->CreateSolidColorBrush(&Color(darked_col), &brush3.get());
    //brush3->SetColor(&Color::FromBytes(186, 200, 210)); // green.
    brush3->SetColor(&Color::FromBytes(lumhi, lumhi, lumhi)); // green.*/
//#if defined(INTEL_HD)

   // brush2->SetColor(&Color::FromBytes(170, 170, 170)); // green.
   // brush3->SetColor(&Color::FromBytes(255, 255, 255)); // green.

//#else

    //brush2->SetColor(&Color::FromBytes(76, 90, 100)); // green.
    //brush3->SetColor(&Color::FromBytes(186, 200, 210)); // green.

//#endif

	gmpi_sdk::mp_shared_ptr<IMpSolidColorBrush> brush2;
	Color color2(darked_col);
	dc->CreateSolidColorBrush(&color2, &brush2.get());
    Color color2_set = Color::FromBytes(lumlo, lumlo, lumlo);
    brush2->SetColor(&color2_set); // green.

	gmpi_sdk::mp_shared_ptr<IMpSolidColorBrush> brush3;
	Color color3(darked_col);
	dc->CreateSolidColorBrush(&color3, &brush3.get());
    Color color3_set = Color::FromBytes(lumhi, lumhi, lumhi);
    brush3->SetColor(&color3_set); // green.


	// Create font.
	/*float fontSize = 8;
	gmpi_sdk::mp_shared_ptr<IMpTextFormat> dtextFormat;

	//		getGuiHost()->CreateTextFormat(
	GetGraphicsFactory().Get()->CreateTextFormat(
		"Verdana",
		NULL,
		GmpiDrawing_API::MP1_FONT_WEIGHT_REGULAR,
		GmpiDrawing_API::MP1_FONT_STYLE_NORMAL,
		GmpiDrawing_API::MP1_FONT_STRETCH_NORMAL,
		fontSize,
		0, //L"", //locale
		&dtextFormat.get()
		);

	*/
	// BACKGROUND LINES
	// horizontal line


	float penWidth = 1.0f;
	dc->DrawLine(GmpiDrawing::Point(0, mid_y), GmpiDrawing::Point(width, mid_y), brush2, penWidth);
	// vertical line
/*  dc->DrawLine(GmpiDrawing::Point(mid_x, 0), GmpiDrawing::Point(mid_x, height), brush2, penWidth);

	// voltage ticks
	int tick_width = 2;
	int step = 1;
	if(height < 50)
		step = 4;

	for( int v = -10 ; v < 11 ; v += step )
	{
		float y = v * scale / 10.f;

		if(v % 5 == 0)
			tick_width = 4;
		else
			tick_width = 2;

		dc->DrawLine(GmpiDrawing::Point(mid_x - tick_width, mid_y + (int)y), GmpiDrawing::Point(mid_x + tick_width, mid_y + (int)y), brush2, penWidth);
	}

	// labels
	if( height > 30 )
	{
		for( int v = -10 ; v < 11 ; v += 5 )
		{
			char txt[10];
			float y = v * scale / 10.f;
			sprintf(txt, "%2.0f", (float) v);

			float tx = mid_x + tick_width;
			float ty = mid_y - (int)y - fontSize / 2;
			dc->DrawTextU(txt, (int32_t) strlen(txt), dtextFormat, &GmpiDrawing::Rect(tx, ty, tx + 100, ty + fontSize), brush2);
		}
	}
*/
        //Color grey = Color::FromBytes(186, 200,210);
        //brush2->SetColor(&grey);


		if( pinSamplesA.rawSize() == sizeof(signed char) * SCOPE_BUFFER_SIZE )
		{

			//brush2->SetColor(&Color::FromBytes(0, 255, 0)); // green.
			signed char* capturedata = (signed char*)pinSamplesA.rawData();
			//++capturedata;
				//for (int i = 1; i < SCOPE_BUFFER_SIZE; i++)
				float x2=0.0f;
				for (int i = 1; i < size0-1; i++)
                {
                    float x = (i * width) / size0;
                    float y = mid_y - (*capturedata * scale*m);
                    dc->DrawLine(GmpiDrawing::Point(x, mid_y), GmpiDrawing::Point(x,  y), brush2, penWidth);
                    dc->DrawLine(GmpiDrawing::Point(x2, y), GmpiDrawing::Point(x,  y), brush3, penWidth);
                    x2=x;
                    ++capturedata;
                }

           // DrawTrace(dc, capturedata, brush2, mid_y, scale, width );
		}

	return gmpi::MP_OK;

}
int32_t Scope3Gui::measure(GmpiDrawing_API::MP1_SIZE availableSize, GmpiDrawing_API::MP1_SIZE* returnDesiredSize)
{
	const float minSize = 15;

	returnDesiredSize->width = (std::max)(minSize, availableSize.width);
	returnDesiredSize->height = (std::max)(minSize, availableSize.height);

	return gmpi::MP_OK;
}
