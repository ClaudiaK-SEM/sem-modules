#ifndef SampleLoader3_H_INCLUDED
#define SampleLoader3_H_INCLUDED

//#include <vector>
#include "mp_sdk_audio.h"

/*struct releasedSample
{
	int handle;
	int releaseCount;
	int64_t releaseTime;
};*/

class SampleLoader3 : public MpBase
{
public:
	SampleLoader3( IMpUnknown* host );
	~SampleLoader3();
	virtual void onSetPins(void);
	//void MP_STDCALL process(int32_t count, const gmpi::MpEvent* events) override;

private:

	StringInPin pinFilename;
	IntInPin pinBank;
	IntInPin pinPatch;
	IntOutPin pinSampleId;
	StringOutPin pinFilenameG; // CK's change : Gui communication parameter (to the module gui)
	IntOutPin pinBankG;// CK's change : Gui communication parameter (to the module gui)
	IntOutPin pinPatchG; // CK's change : Gui communication parameter (to the module gui)
	IntInPin pinGuiSampleId; // CK's change : Gui communication parameter (from the module gui)-> gui samplehandle
	IntOutPin pinIdToGui; // CK's change : Gui communication parameter (to the module gui)

    int loading; // CK's change : not sure that is necessary but...
    void load(); //CK's change : to call the sample manager for loading the sf2
	int sampleHandle; // sf2 id

    //wchar_t *memFilename;
    //int memBank;
    //int memPatch;

  	//int64_t time = 0;
	//std::vector<releasedSample> toRelease;

};

#endif
