#include "CK_VoiceControlGui.h"
#include "../shared/voice_allocation_modes.h"

using namespace gmpi;
using namespace gmpi_gui;

using namespace voice_allocation;
using namespace voice_allocation::bits;

//SE_DECLARE_INIT_STATIC_FILE(CK_VoiceControlGui2)
GMPI_REGISTER_GUI(MP_SUB_TYPE_GUI2, CK_VoiceControlGui2, L"CK_VoiceControl");

CK_VoiceControlGui2::CK_VoiceControlGui2()
{
	initializePin(BlobToGui, static_cast<MpGuiBaseMemberPtr2>( &CK_VoiceControlGui2::onSetBlobToGui ));
	initializePin(hostPolyphony);
	initializePin(hostReserveVoices);
	initializePin(hostVoiceAllocationMode);
	initializePin(host_PortamentoTime);
	initializePin(hostBendRange);

}

/*int32_t CK_VoiceControlGui2::initialize()
{

	const auto allocationMode = hostVoiceAllocationMode.getValue();

	monoNotePriority = 0x03 & ( ( (int)hostVoiceAllocationMode ) >> 8 );
	GlideType = 0x01 & ( ( (int)hostVoiceAllocationMode ) >> 16 );
	GlideTiming = 0x01 & ( ( (int)hostVoiceAllocationMode ) >> 18 );

	voiceStealMode = 0x03 & ( (int)hostVoiceAllocationMode );

	monoMode = isMonoMode(allocationMode);
	monoRetrigger = isMonoRetrigger(allocationMode);

	{
		const int bitPosistion = 19;
		VoiceRefresh = 0x01 & (((int)hostVoiceAllocationMode) >> bitPosistion);
	}

	//assert(monoNotePriority == extractBits(allocationMode, 8, 2));
	//assert(GlideType == extractBits(allocationMode, 16, 1));
	//assert(GlideTiming == extractBits(allocationMode, 18, 1));
	//assert(voiceStealMode == extractBits(allocationMode, 0, 2));
	//assert(VoiceRefresh == extractBits(allocationMode, 19, 1));

    BendRange = hostBendRange;
    //PortamentoTime = host_PortamentoTime;
    polyphony = hostPolyphony;
    reserveVoices = hostReserveVoices;

    onSetVoiceAllocation();
    //onSetPortamento();
    onSetBendRange();
    onSetPolyAndReserve();

	return MpGuiInvisibleBase::initialize();
}*/

void CK_VoiceControlGui2::onSetBlobToGui()
{

 		if( BlobToGui.rawSize() == sizeof(float) * 12 )
		{
			float* ptr = (float*) BlobToGui.rawData();

			int id=ptr[0];
            monoMode = ptr[1] ;
            monoRetrigger = ptr[2];
            monoNotePriority = ptr[3];
            polyphony = ptr[4];
            reserveVoices = ptr[5];
            voiceStealMode = ptr[6];
            PortamentoTime = ptr[7];
            GlideTiming = ptr[8];
            GlideType = ptr[9];
            BendRange = ptr[10];
            VoiceRefresh = ptr[11];

            //if(id==1)onSetMonoMode();//all Allocation
            //if(id==2)onSetMonoMode();//all Allocation
            //if(id==3)onSetNotePriority();//all Allocation
            //if(id==4)onSetPolyphony();
            //if(id==5)onSetPolyphonyReserve();
            //if(id==6)onSetVoiceStealMode();//all Allocation
            if(id==7)onSetPortamento();
            //if(id==8)onSetGlideTiming();//all Allocation
            //if(id==9)onSetGlide();//all Allocation
            if(id==10)onSetBendRange();
            //if(id==11)onSetVoiceRefresh();//all Allocation
            if(id==12)
            {
            onSetPortamento();
            onSetVoiceAllocation();//all Allocation
            }
            if(id==13)onSetPolyAndReserve();
            if(id==14)
            {
            onSetVoiceAllocation();
            onSetPortamento();
            onSetBendRange();
            onSetPolyAndReserve();
            }

		}
}

void CK_VoiceControlGui2::onSetBendRange()
{
hostBendRange = BendRange;
}

void CK_VoiceControlGui2::onSetPortamento()
{
host_PortamentoTime = PortamentoTime;
}

/*void CK_VoiceControlGui2::onSetPolyphony()
{
hostPolyphony = polyphony;
}

void CK_VoiceControlGui2::onSetPolyphonyReserve()
{
hostReserveVoices = reserveVoices;
}*/

void CK_VoiceControlGui2::onSetPolyAndReserve()
{
hostPolyphony = polyphony;
hostReserveVoices = reserveVoices;
}
/*
void CK_VoiceControlGui2::onSetMonoMode()
{
	const int flags
		= MM_IN_USE
		| (monoMode ? MM_ON : 0)
		| (monoRetrigger ? MM_RETRIGGER : 0);

	const auto v = hostVoiceAllocationMode.getValue();
	hostVoiceAllocationMode = insertBits(v, MonoModes_startbit, MonoModes_sizebits, flags);
}

void CK_VoiceControlGui2::onSetVoiceStealMode()
{

	auto v = hostVoiceAllocationMode.getValue();

	const int combinedVoiceAllocationMode = voiceStealMode & 0x03;

	hostVoiceAllocationMode = ( hostVoiceAllocationMode & 0xffffff00 ) | combinedVoiceAllocationMode;

	assert(hostVoiceAllocationMode == insertBits(v, 0, 8, combinedVoiceAllocationMode));
}

void CK_VoiceControlGui2::onSetNotePriority()
{
	const auto v = hostVoiceAllocationMode.getValue();
	hostVoiceAllocationMode = insertBits(v, NotePriority_startbit, NotePriority_sizebits, monoNotePriority);
}

void CK_VoiceControlGui2::onSetVoiceRefresh()
{
	auto v = hostVoiceAllocationMode.getValue();

	const int bitPosistion = 19;
	const int mask = ~(1 << bitPosistion);
	hostVoiceAllocationMode = (hostVoiceAllocationMode & mask) | ((VoiceRefresh & 0x01) << bitPosistion);

	assert(hostVoiceAllocationMode == insertBits(v, 19, 1, VoiceRefresh));
}

void CK_VoiceControlGui2::onSetGlide()
{
	auto v = hostVoiceAllocationMode.getValue();

// should be 0xfffeffff	hostVoiceAllocationMode = ( hostVoiceAllocationMode & 0xfffcffff ) | ( GlideType << 16 );

//	assert(hostVoiceAllocationMode == insertBits(v, 16, 1, GlideType));

	hostVoiceAllocationMode = insertBits(v, 16, 1, GlideType);
}

void CK_VoiceControlGui2::onSetGlideTiming()
{
    auto v = hostVoiceAllocationMode.getValue();

	hostVoiceAllocationMode = ( hostVoiceAllocationMode & 0xfffbffff ) | ( GlideTiming << 18 );

	assert(hostVoiceAllocationMode == insertBits(v, 18, 1, GlideTiming));
}
*/
void CK_VoiceControlGui2::onSetVoiceAllocation()
{
    auto v = 0;

	//monoMode
	const int flags
		= MM_IN_USE
		| (monoMode ? MM_ON : 0)
		| (monoRetrigger ? MM_RETRIGGER : 0);
    const auto tp_monoMode=hostVoiceAllocationMode.getValue();
	v = insertBits(tp_monoMode, MonoModes_startbit, MonoModes_sizebits, flags);

    //VoiceSteal
    //auto tp_voiceStealMode=v;
	const int combinedVoiceAllocationMode = voiceStealMode & 0x03;
	v = ( v & 0xffffff00 ) | combinedVoiceAllocationMode;
	//assert(v == insertBits(tp_voiceStealMode, 0, 8, combinedVoiceAllocationMode));

    //NotePriority
	const auto tp_NotePriority = v;
	v = insertBits(tp_NotePriority, NotePriority_startbit, NotePriority_sizebits, monoNotePriority);

    //GlideTiming
 	v = insertBits(v, 16, 1, GlideType);

    //GlideTiming
	//auto tp_GlideTiming=v;
	v = ( v & 0xfffbffff ) | ( GlideTiming << 18 );
	//assert(v == insertBits(GlideTiming, 18, 1, GlideTiming));

    //VoiceRefresh
	//auto tp_VoiceRefresh=v;
	const int bitPosistion = 19;
	const int mask = ~(1 << bitPosistion);
	v = (v & mask) | ((VoiceRefresh & 0x01) << bitPosistion);
	//assert(v == insertBits(tp_VoiceRefresh, 19, 1, VoiceRefresh));

hostVoiceAllocationMode=v;
}

