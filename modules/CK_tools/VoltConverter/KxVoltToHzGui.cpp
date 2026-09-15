// prevent MS CPP - 'swprintf' was declared deprecated warning
#if defined(_MSC_VER)
  #define _CRT_SECURE_NO_DEPRECATE
  #pragma warning(disable : 4996)
#endif

#include "KxVoltToHzGui.h"
#include "math.h"


REGISTER_GUI_PLUGIN( KxVoltToHzGui, L"KxVoltConverter" );

KxVoltToHzGui::KxVoltToHzGui( IMpUnknown* host ) : MpGuiBase(host)
{
	initializePin( 0, pinFromDSP, static_cast<MpGuiBaseMemberPtr>( &KxVoltToHzGui::onValueChanged ) );
	//initializePin( 1, pinToDSP);
	initializePin( 1, pinIntDO);
	initializePin( 2, pinTextDO);
	initializePin( 3, pinFloatDO);
	//initializePin( 5, pinIntDI);
	//initializePin( 6, pinTextDI);
	//initializePin( 7, pinFloatDI);
	initializePin( 4, pinMode, static_cast<MpGuiBaseMemberPtr>( &KxVoltToHzGui::onValueChanged ) );
	initializePin( 5, pinThreshold, static_cast<MpGuiBaseMemberPtr>( &KxVoltToHzGui::onValueChanged ) );

    //ct=0;

}
void KxVoltToHzGui::onValueChanged()
{

	const float vsn = 1E-6f;
    float f_10=pinFromDSP;
    float f=pinFromDSP*0.1f;

    float conv=0.0f;



    wchar_t txt[15];
    const size_t txt_size = sizeof(txt) / sizeof(wchar_t);


      if(pinMode==0)//float -ok
      {
        conv=f_10;
        swprintf(txt, txt_size, L"%5.3f hz",conv);
      }


      if(pinMode==1)//volt to dB -ok
      {
      if(f>=0.00000006f)conv=20.0f * log10f(f);
      swprintf(txt, txt_size, L"%5.3f dB",conv);
      }


      if(pinMode==2)//dB to volt -ok
      {
        float tmp=f_10*0.05f;
        conv=10.0f * powf(10.0f,tmp );
        swprintf(txt, txt_size, L"%5.3f",conv);
      }


      if(pinMode==3)//volt to Hz -ok
      {
        //float volt=f_10;
        float volt=pinFromDSP;
        if(volt>10.506355f)volt=10.506355f;
        if(volt<-10.0f)volt=-10.0f;
        conv=13.75f*powf(2.0f,volt);
        swprintf(txt, txt_size, L"%5.3f hz",conv);
      }
      else
      {
        //conv=f_10;
        conv=pinFromDSP;
        swprintf(txt, txt_size, L"%5.3f hz",conv);
      }


      if(pinMode==4)//hz to volt -ok
      {
        if(f_10>vsn)conv=logf( f_10 / 440.0f ) / logf(2.0f) + 5.0f;
        swprintf(txt, txt_size, L"%5.3f",conv);
      }


      if(pinMode==5)//Hz to Bpm -ok
      {
      conv=f_10*60.0f;//60.0f/f_10
      swprintf(txt, txt_size, L"%5.3f bpm",conv);
      }

      if(pinMode==6)//Bpm to Hz
      {
      if(f_10 >=0.002f)conv=60.0f/f_10;
      swprintf(txt, txt_size, L"%5.3f hz",conv);
      }

      if(pinMode==7)//volt to hz to Bpm -ok
      {
        float volt=f_10;
        if(volt>10.506355f)volt=10.506355f;
        if(volt<-10.0f)volt=-10.0f;
        float tmp;
        tmp=13.75f*powf(2.0f,volt);
        conv=tmp*60.0f;
        swprintf(txt, txt_size, L"%5.3f bpm",conv);
      }

      if(pinMode==8)//Bpm to hz to volt
      {
        if(f_10 >=0.002f)
        {
        float tmp;
        tmp=60.0f/f_10;
        conv=logf( tmp / 440.0f ) / logf(2.0f) + 5.0f;
        }

        swprintf(txt, txt_size, L"%5.3f",conv);
      }

      if(pinMode==9)//+on off 10
      {
        if(f_10>pinThreshold)
        {
        conv=10.0f;
        //txt=L"On";
        swprintf(txt, txt_size, L"On");
        }
        else
        {
        conv=0.0f;
        //txt=L"Off";
        swprintf(txt, txt_size, L"Off");
        }

      }

       if(pinMode==10)//+on off 1
      {
        if(f_10>pinThreshold)
        {
        conv=1.0f;
        //txt=L"On";
        swprintf(txt, txt_size, L"On");
        }
        else
        {
        conv=0.0f;
        //txt=L"Off";
        swprintf(txt, txt_size, L"Off");
        }
      }

     /*if(pinMode==11)//-+on off 10
      {
        //if(f_10>pinThreshold || f_10<-pinThreshold)
        //{
        //ct=1;
        //}
        //else
        //{

        //}

        //if(ct>=1)
        //{
        //conv=10.0f;
        //swprintf(txt,L"On");
            //if(ct<5)ct++;
            //else
            //ct=0;
        //}
        //if(ct==0)
        //{
        //conv=0.0f;
        //swprintf(txt,L"Off");
        //}

        if(f_10>pinThreshold || f_10<-pinThreshold)
        {
        conv=1.0f;
        //txt=L"On";
        swprintf(txt,L"On");
        }
        else
        {
        conv=0.0f;
        //txt=L"Off";
        swprintf(txt,L"Off");
        }
      }

       if(pinMode==12)//-+on off 1
      {
        if(f_10>pinThreshold || f_10<-pinThreshold)
        {
        conv=1.0f;
        //txt=L"On";
        swprintf(txt,L"On");
        }
        else
        {
        conv=0.0f;
        //txt=L"Off";
        swprintf(txt,L"Off");
        }
      }*/

        pinIntDO=int(conv);
        pinTextDO=txt;
        pinFloatDO=conv;
        //pinToDSP=pinFloatDO=pinFloatDI=conv;
       //pinToDSP.sendPinUpdate()

        //pinIntDO=pinIntDI=int(conv);
       //pinTextDO=pinTextDI=txt;
        //pinToDSP=pinFloatDO=pinFloatDI=conv;
       //pinToDSP.sendPinUpdate()



        //pinTextDO=txt;



	//invalidateRect();
}
/*
int32_t KxVoltToHzGui::paint(HDC hDC)
{

	MpRect r = getRect();
	int width = r.right - r.left;
	int height = r.bottom - r.top;

    HFONT font_handle;
	getGuiHost()->getFontInfo( L"control_edit", fontInfo_, font_handle );


	if( fontInfo_.colorBackground >= 0 ) // -1 indicates transparent background
	{
		// Fill in solid background black
		HBRUSH background_brush = CreateSolidBrush(fontInfo_.colorBackground);
		RECT r;
		r.top = r.left = 0;
		r.right = width + 1;
		r.bottom = height + 1;
		FillRect(hDC, &r, background_brush);
		// cleanup objects
		DeleteObject(background_brush); //ok
	}

    //int fontHeight = fontInfo_.fontHeight;
    //wchar_t txt_hz[60];
    swprintf(txt_hz,L"%5.3f hz",f_10);
    HGDIOBJ old_font = SelectObject( hDC, font_handle );
    SetTextColor( hDC, fontInfo_.color );
    SetBkMode( hDC, TRANSPARENT );
    SetTextAlign( hDC, TA_RIGHT );
    TextOut( hDC, width-1, height-fontInfo_.fontHeight, txt_hz, (int) wcslen(txt_hz) );
    DeleteObject(font_handle);


    //StopTimer();
	return gmpi::MP_OK;
}
int32_t KxVoltToHzGui::measure(MpSize availableSize, MpSize &returnDesiredSize)
{
	const int prefferedSize = 100;
	const int minSize = 15;

	returnDesiredSize.x = availableSize.x;
	returnDesiredSize.y = availableSize.y;
	if(returnDesiredSize.x > prefferedSize)
	{
		returnDesiredSize.x = prefferedSize;
	}
	else
	{
		if(returnDesiredSize.x < minSize)
		{
			returnDesiredSize.x = minSize;
		}
	}
	if(returnDesiredSize.y > prefferedSize)
	{
		returnDesiredSize.y = prefferedSize;
	}
	else
	{
		if(returnDesiredSize.y < minSize)
		{
			returnDesiredSize.y = minSize;
		}
	}
	return gmpi::MP_OK;
};
*/
