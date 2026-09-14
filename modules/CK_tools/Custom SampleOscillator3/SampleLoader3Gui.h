#ifndef SampleLoader3Gui_H_INCLUDED
#define SampleLoader3Gui_H_INCLUDED


#include "mp_sdk_gui.h"

class SampleLoader3Gui : public MpGuiBase
{
public:
	SampleLoader3Gui(IMpUnknown* host);
	virtual int32_t MP_STDCALL initialize();
	virtual ~SampleLoader3Gui();



	StringGuiPin	pinFilename;
	IntGuiPin		pinBank;
	IntGuiPin		pinPatch;
	StringGuiPin	pinBankNames;
	StringGuiPin	pinPatchNames;
	IntGuiPin		pinPatchReset;//CK's change
	IntGuiPin		pinBankReset;//CK's change
    IntGuiPin		pinIdToDsp;//CK's change
    IntGuiPin		pinIdToGui;//CK's change
    IntGuiPin       pinRndIn;//CK's change
    //FloatGuiPin	pinLoadingGui;//CK's change
    //bool test();
    //bool t;
private:

    void load(int loading);////CK's change  : to call the sample manager for loading the sf2
    //void loadingGUI();
    void test();
    int sampleHandle; ////CK's change : sf2 id
    int init;  ////CK's change : 1 when the module gui is initialized
    int c;
    wchar_t *memFilename;
    int memBank;
    int memPatch;
    int memRnd;
};

#endif
