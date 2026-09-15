#ifndef PrgChGUI_H_INCLUDED
#define PrgChGUI_H_INCLUDED

//#include "MP_SDK_GUI.h"

class PrgChGui : public MpGuiBase
{
public:
	PrgChGui( IMpUnknown* host );
    virtual int32_t MP_STDCALL initialize();

private:

	void onSetProgramIn();
	void onSetWriteName();
	void onSetProgramOut();
	void onSetRndIn();


	bool write_name;
	int mem_prg;
	int mem_rnd;
 	int init;
    //wchar_t * mem_name;
 	IntGuiPin RndInToDSP;
 	IntGuiPin programIn;
 	IntGuiPin RndIn;
 	IntGuiPin Mode;
  	StringGuiPin programNameIn;
  	BoolGuiPin WriteName;
   	StringGuiPin programNameOut;

};

#endif


