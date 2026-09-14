// Copyright 2006 Jeff McClintock

#ifndef FullPathX_H_INCLUDED
#define FullPathX_H_INCLUDED

#include "mp_sdk_audio.h"

class FullPathX: public MpBase
{
	public:
	FullPathX(IMpUnknown* host);
	void onSetPins(void);

private:
	StringInPin Filename;
	//StringInPin AbsPath;
    StringInPin pinAbsPath;
	StringOutPin pinPathOut;
};

#endif
