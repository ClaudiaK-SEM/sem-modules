#pragma once
#include "segui_base.h"


class GuiModule :
	public SEGUI_base
{
public:
	GuiModule(seGuiCallback seaudioMaster, void *p_resvd1);
	~GuiModule(void);
	virtual void paint(HDC hDC, SEWndInfo *wi);
//	virtual void OnModuleMsg(int p_user_msg_id,int p_length, void * p_data);
    virtual void OnGuiPinValueChange(SeGuiPin *p_pin);
/*
	virtual bool OnIdle(void);
	void Initialise(bool loaded_from_file);
*/

private:
//	void SendStringToAudio( int p_msg_id, int p_length, void *p_data );
//	int Handle(void);
	int InvalidateControl(void);
	float f_in;
    char disp[20];
    //char face[20];
	SeFontInfo m_font_info;

};
