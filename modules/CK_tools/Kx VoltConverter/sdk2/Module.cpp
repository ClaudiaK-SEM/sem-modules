/*-----------------------------------------------------------------------------

© 2002, Jeff M Soft und Hardware GmbH, All Rights Reserved

-----------------------------------------------------------------------------*/

#include "Module.h"
#include "SEMod_struct.h"
#include "SEPin.h"
#include <assert.h>
#include <math.h>

// define some constants to make referencing in/outs clearer
#define PN_IN  0
#define PN_TO_GUI 1
#define PN_GUIPIN 2
void Module::sub_process(long start_pos, long sampleframes)
{
	CallHost(seaudioMasterSleepMode);

}


Module::Module(seaudioMasterCallback2 seaudioMaster, void *p_resvd1) : SEModule_base(seaudioMaster, p_resvd1)
{
}

Module::~Module()
{
	// This is where you free any memory/resources your module has created
}

void Module::open()
{
	SEModule_base::open();	// always call the base class

	SET_PROCESS_FUNC(Module::sub_process);

   // volt=m_volt=getPin(PN_IN)->getValue();
    //Freq();

}

// describe your module
void Module::getModuleProperties(SEModuleProperties* properties)
{
#if defined(GNU)
	properties->name = "GCC KX-Hz";
	properties->id = "KX-Hz";
	properties->about = "GCC KX-Hz by Xavier Kalensky, kx77free.free.fr, License agreement: This module is provided as is with no warranty of any kind.The use of this plugin is entirely at your own risk. You may not distribute this module in any way.";
#endif
#if defined(MSV)
	properties->name = "KX-Hz";
	properties->id = "KX-Hz";
	properties->about = "KX-Hz by Xavier Kalensky, kx77free.free.fr, License agreement: This module is provided as is with no warranty of any kind.The use of this plugin is entirely at your own risk. You may not distribute this module in any way.";
#endif
	//return true;
	properties->flags = UGF_POLYPHONIC_AGREGATOR|UGF_VOICE_MON_IGNORE;
	properties->gui_flags = CF_CONTROL_VIEW|CF_STRUCTURE_VIEW;
	properties->sdk_version = SDK_VERSION;
}

// describe the pins (plugs and parameters)
bool Module::getPinProperties (long index, SEPinProperties* properties)
{
	switch( index )								// !!TODO!! list your in / out plugs
	{
		// typical input plug (inputs are listed first)


	case 0:
		properties->name				= "Volt";
		properties->variable_address	= &in;
		properties->direction			= DR_IN;
		properties->datatype			= DT_FSAMPLE;
		properties->default_value		= "0";
		break;

    case 1: // hidden pin that sends data from DSP to the GUI module
        properties->name = "gui_com_pin";
        //properties->name = "Value Out";
        properties->variable_address = &out;
        properties->direction = DR_OUT;
        properties->datatype = DT_FLOAT;
        properties->flags = IO_PATCH_STORE|IO_HIDE_PIN;
        //properties->flags = IO_HIDE_PIN;
        break;

			// GUI PIN. Appears only on GUI object
    case 2: // this GUI pin receives data from the DSP Module
        properties->name = "Val from DSP";
        properties->direction = DR_IN;
        properties->datatype = DT_FLOAT;
        //properties->flags = IO_UI_COMMUNICATION|IO_HIDE_PIN;
        properties->flags = IO_PATCH_STORE|IO_UI_COMMUNICATION|IO_HIDE_PIN;
        break;

	default:
		return false; // host will ask for plugs 0,1,2,3 etc. return false to signal when done
	};

	return true;
}
void Module::OnPlugStateChange(SEPin *pinptr)
{


   float volt=getPin(PN_IN)->getValue();
   // if(volt != m_volt)
    //{
    //m_volt=volt;
    if(volt>1.0333f)volt=1.0333f;
    if(volt<-1.0f)volt=-1.0f;
    out=440.0f*powf(2.0f,(10.0f*volt)-5.0f);
    getPin(1)->TransmitStatusChange( SampleClock(), ST_STATIC );
    //Freq();
    //}


}
/*void Module::Freq()
{
//float volt;
out=440.0f*powf(2.0f,(10.0f*volt)-5.0f);
getPin(1)->TransmitStatusChange( SampleClock(), ST_STATIC );
}*/
