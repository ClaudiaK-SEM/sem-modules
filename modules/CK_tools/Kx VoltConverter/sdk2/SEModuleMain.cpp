/*-----------------------------------------------------------------------------

© 2002, Jeff McClintock, SynthEdit 

-----------------------------------------------------------------------------*/
#include "Module.h"
#include "GuiModule.h"
#include "semod_struct.h"

extern "C" {
__declspec (dllexport)
int getModuleProperties(int p_index, SEModuleProperties* p_properties)
{
	p_properties->sdk_version = SDK_VERSION;

	switch( p_index )								// !!TODO!! list your in / out plugs
	{
	case 0:
		Module::getModuleProperties(p_properties);
		break;
	default:
		return false; // host will ask for module 0,1,2,3 etc. return false to signal when done
	};

	return true;
}
}

extern "C" {
__declspec (dllexport)
void * makeModule(int p_index, int p_type, seaudioMasterCallback2 seaudioMaster, void *p_resvd1)
{
	switch( p_index )								// !!TODO!! list your in / out plugs
	{
	case 0:
		{
			if( p_type == 1 ) // Audio Processing Object
			{
				SEModule_base *effect = new Module(seaudioMaster, p_resvd1);
				if (!effect)
					return 0;
				return effect->getAeffect();
			}

			if( p_type == 2 ) // GUI Object
			{
				GuiModule *effect = new GuiModule((seGuiCallback) seaudioMaster, p_resvd1); //nasty!
				if (!effect)
					return 0;
				return effect->getAeffect();
			}
		}
		break;
	};

	return 0;
}
}

#if WIN32
#include <windows.h>
void* hInstance;
BOOL WINAPI DllMain (HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
{
	hInstance = hInst;
	return 1;
}
#endif
