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
    IntGuiPin       pinDebugVST3;//CK's change
    IntGuiPin       pinLoadToGui;//CK's change
private:

    void load(int loading);////CK's change  : to call the sample manager for loading the sf2
    void test();////CK's change : to test the file before to load and to update the patchs and the banks, names and numbers.
    int sampleHandle; ////CK's change : sf2 id
    int init;  ////CK's change : 1 when the module gui is initialized
    //int loading;////CK's change
    int buzy;////CK's change
};

#endif
