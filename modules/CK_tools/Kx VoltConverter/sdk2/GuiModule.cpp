#include "guimodule.h"
#include "SEGUI_Pin.h"
#include "windows.h"
#include <stdio.h>
#include <assert.h>

#define PN_GUIIN 2

GuiModule::GuiModule(seGuiCallback seaudioMaster, void *p_resvd1) : SEGUI_base(seaudioMaster, p_resvd1)
{
	CallHost(seGuiHostSetWindowSize, 100, 30 );
	CallHost(seGuiHostSetWindowType, 0 ); // 0 = Draw on SE's window (default), 1 = HWND based
	CallHost(seGuiHostSetWindowFlags, HWF_RESIZEABLE );
    f_in = getPin(PN_GUIIN)->getValueFloat();

}

GuiModule::~GuiModule(void)
{
}

void GuiModule::paint( HDC hDC, SEWndInfo *wi )
{

	    //HFONT font_handle = (HFONT) CallHost(seGuiHostGetFontInfo, wi->context_handle, (long) "tty", &m_font_info );
	    HFONT font_handle = (HFONT) CallHost(seGuiHostGetFontInfo, wi->context_handle, (long) "control_edit", &m_font_info );
	    //HFONT font_handle = (HFONT) CallHost(seGuiHostGetFontInfo, wi->context_handle, (long) "plug", &m_font_info );

		// Fill in solid background black

	if( m_font_info.color_background >= 0 ) // -1 indicates transparent background
	{

		//HBRUSH background_brush = CreateSolidBrush(RGB(0,0,0));
		HBRUSH background_brush = CreateSolidBrush(m_font_info.color_background);


		RECT r;
		r.top = r.left = 0;
		r.right = wi->width + 1;
		r.bottom = wi->height + 1;
		FillRect( hDC, &r, background_brush );

		// cleanup objects
		DeleteObject(background_brush);

    }
        int font_height = m_font_info.font_height;

	// txt
	//if( wi->height > font_height )
	//{
		//int font_height = m_font_info.font_height;
		HGDIOBJ old_font = SelectObject( hDC, font_handle );
        SetTextColor( hDC, m_font_info.color );
        //SetTextColor( hDC, RGB(200,200,200));
		SetBkMode( hDC, TRANSPARENT );
        SetTextAlign( hDC, TA_RIGHT );

        sprintf_s(disp,"%5.3f hz",f_in);
        TextOut( hDC, wi->width-1, wi->height-font_height, disp, strlen(disp) );

		//SelectObject( hDC, old_font ); //txt
        //DeleteObject(font_handle);

   //}


/*
		//int font_height = 10; //p_child->Skin()->getFontDescription(_T("tty"))->AverageCharSize().cy;
        int font_height = m_font_info.font_height-2;
		LOGFONT lf;
		memset(&lf, 0, sizeof(LOGFONT));   // Clear out structure.

		//strcpy_s(lf.lfFaceName, "Terminal" );    // face name
		strcpy_s(lf.lfFaceName, "Arial" );
		//strcpy_s(lf.lfFaceName, "Verdana" );
		lf.lfHeight = -font_height;

		HFONT font = CreateFontIndirect(&lf);
		HGDIOBJ old_font = SelectObject( hDC, font );

        //HGDIOBJ old_font = SelectObject( hDC, font_handle );

		SetTextColor( hDC, m_font_info.color );
		//SetTextColor( hDC, RGB(200,200,200) );
		SetBkMode( hDC, TRANSPARENT );
        SetTextAlign( hDC, TA_RIGHT );

        sprintf_s(disp,"%5.3f hz",f_in);
        TextOut( hDC, wi->width-1, wi->height-font_height-2, disp, strlen(disp) );

		SelectObject( hDC, old_font );
		//DeleteObject(font_handle);
		DeleteObject(font);
*/

}
int GuiModule::InvalidateControl(void)
{
	return CallHost(seGuiHostRequestRepaint);
}

void GuiModule::OnGuiPinValueChange(SeGuiPin *p_pin)
{
     f_in = getPin(PN_GUIIN)->getValueFloat();
	 InvalidateControl();
}
