#include ".\PrgChGui.h"
#include <Time.h>

REGISTER_GUI_PLUGIN( PrgChGui, L"KxPrgChange" );

PrgChGui::PrgChGui(IMpUnknown* host) : MpGuiBase(host)
{
	// initialise pins.
 	RndInToDSP.initialize( this, 0 );
 	programIn.initialize( this, 1, static_cast<MpGuiBaseMemberPtr>(&PrgChGui::onSetRndIn) );
 	RndIn.initialize( this, 2, static_cast<MpGuiBaseMemberPtr>(&PrgChGui::onSetRndIn) );
 	Mode.initialize( this,3);
  	programNameIn.initialize( this, 4, static_cast<MpGuiBaseMemberPtr>(&PrgChGui::onSetRndIn) );
  	WriteName.initialize( this, 5 , static_cast<MpGuiBaseMemberPtr>(&PrgChGui::onSetWriteName) );
  	programNameOut.initialize( this, 6, static_cast<MpGuiBaseMemberPtr>(&PrgChGui::onSetProgramOut) );

    init=0; // alway set c to 0 because the gui pins are alway initialized when the gui editor is going to on.
    write_name=true;
    mem_prg=0;
    mem_rnd=0;
}

int32_t PrgChGui::initialize()
{



        init=1;// now the gui editor is on
	    return MpGuiBase::initialize();

}
void PrgChGui::onSetProgramIn()
{
    programNameOut=programNameIn;
}

void PrgChGui::onSetWriteName()
{
write_name=WriteName;
}

void PrgChGui::onSetProgramOut()
{
programNameIn=programNameOut;
}

void PrgChGui::onSetRndIn()
{



                if(write_name==false)
                {
                    programNameOut=programNameIn;


                    if(init==1 && Mode==0)
                    {
                    if(mem_prg!=programIn)RndInToDSP = programIn;
                    if(mem_rnd!=RndIn)RndInToDSP = RndIn;
                    mem_prg=programIn;
                    mem_rnd=RndIn;
                    }

                    if(init==1 && Mode==1)
                    {
                    if(mem_prg!=programIn)RndInToDSP = programIn;
                    mem_prg=programIn;
                    }
                    if(init==1 && Mode==2)
                    {
                    if(mem_rnd!=RndIn)RndInToDSP = RndIn;
                    mem_rnd=RndIn;
                    }
                    if(init==1 && Mode==3)
                    {
                    srand(time(0));
                    float RANGE_MIN = 0.0f;
                    float  RANGE_MAX = 2147483647.0f;
                    RndInToDSP = int(((float) rand() / (float) RAND_MAX) * RANGE_MAX + RANGE_MIN);
                    }

                }

	//if(init==1 && (Mode==0 || Mode==1))RndInToDSP = programIn;
	//if(init==1 && (Mode==0 || Mode==2))RndInToDSP = RndIn;
}





