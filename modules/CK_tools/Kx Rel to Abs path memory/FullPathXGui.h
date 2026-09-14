#ifndef FullPathXGui_H_INCLUDED
#define FullPathXGui_H_INCLUDED

#include "mp_sdk_gui.h"

class FullPathXGui :
	public MpGuiBase
{
public:
	FullPathXGui(IMpUnknown* host);
	//virtual int32_t MP_STDCALL initialize();


 	StringGuiPin    pinFilenameIn;//dsp
 	StringGuiPin    pinName;//out
 	StringGuiPin    pinFilename;//out
 	StringGuiPin    pinPath;//out
 	StringGuiPin    pinFullPath;//out
 	//StringGuiPin    pinAbsPath;//out

private:

	void update();
	void onFilenameInChanged();
	void onNameChanged();
    void onFilenameChanged();
    void onPathChanged();
    void onFullPathChanged();
    //void onAbsPathChanged();
};

#endif
