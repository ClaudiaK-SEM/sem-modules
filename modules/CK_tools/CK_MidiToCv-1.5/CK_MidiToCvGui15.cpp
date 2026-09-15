#include "CK_MidiToCvGui15.h"
#include "../shared/voice_allocation_modes.h"

using namespace gmpi;
using namespace gmpi_gui;

using namespace voice_allocation;
using namespace voice_allocation::bits;

//SE_DECLARE_INIT_STATIC_FILE(CK_MidiToCvGui15)
GMPI_REGISTER_GUI(MP_SUB_TYPE_GUI2, CK_MidiToCvGui15, L"CK_MidiToCv-1.5");

CK_MidiToCvGui15::CK_MidiToCvGui15()
{
	initializePin(BlobToGui, static_cast<MpGuiBaseMemberPtr2>( &CK_MidiToCvGui15::onSetBlobToGui ));
	initializePin(hostPolyphony);
	initializePin(hostReserveVoices);
	initializePin(hostVoiceAllocationMode);
	initializePin(host_PortamentoTime);
	initializePin(hostBendRange);

}

//int32_t CK_MidiToCvGui15::initialize()
//{

	/*const auto allocationMode = hostVoiceAllocationMode.getValue();

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

	assert(monoNotePriority == extractBits(allocationMode, 8, 2));
	assert(GlideType == extractBits(allocationMode, 16, 1));
	assert(GlideTiming == extractBits(allocationMode, 18, 1));
	assert(voiceStealMode == extractBits(allocationMode, 0, 2));
	assert(VoiceRefresh == extractBits(allocationMode, 19, 1));

    BendRange = hostBendRange;
    PortamentoTime = host_PortamentoTime;
    polyphony = hostPolyphony;
    reserveVoiceshost = ReserveVoices;*/


	//return MpGuiInvisibleBase::initialize();
//}

void CK_MidiToCvGui15::onSetBlobToGui()
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
            if(id==12)onSetVoiceAllocation();//all Allocation
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

void CK_MidiToCvGui15::onSetBendRange()
{
if(BendRange != hostBendRange)hostBendRange = BendRange;
}

void CK_MidiToCvGui15::onSetPortamento()
{
if(PortamentoTime != host_PortamentoTime)host_PortamentoTime = PortamentoTime;
}

void CK_MidiToCvGui15::onSetPolyphony()
{
if(polyphony != hostPolyphony) hostPolyphony = polyphony;
}

void CK_MidiToCvGui15::onSetPolyphonyReserve()
{
if(reserveVoices != hostReserveVoices) hostReserveVoices = reserveVoices;
}
void CK_MidiToCvGui15::onSetPolyAndReserve()
{
if(polyphony != hostPolyphony) hostPolyphony = polyphony;
if(reserveVoices != hostReserveVoices) hostReserveVoices = reserveVoices;
}
/*
void CK_MidiToCvGui15::onSetMonoMode()
{
	const int flags
		= MM_IN_USE
		| (monoMode ? MM_ON : 0)
		| (monoRetrigger ? MM_RETRIGGER : 0);

	const auto v = hostVoiceAllocationMode.getValue();
	hostVoiceAllocationMode = insertBits(v, MonoModes_startbit, MonoModes_sizebits, flags);
}

void CK_MidiToCvGui15::onSetVoiceStealMode()
{

	auto v = hostVoiceAllocationMode.getValue();

	const int combinedVoiceAllocationMode = voiceStealMode & 0x03;

	hostVoiceAllocationMode = ( hostVoiceAllocationMode & 0xffffff00 ) | combinedVoiceAllocationMode;

	assert(hostVoiceAllocationMode == insertBits(v, 0, 8, combinedVoiceAllocationMode));
}

void CK_MidiToCvGui15::onSetNotePriority()
{
	const auto v = hostVoiceAllocationMode.getValue();
	hostVoiceAllocationMode = insertBits(v, NotePriority_startbit, NotePriority_sizebits, monoNotePriority);
}

void CK_MidiToCvGui15::onSetVoiceRefresh()
{
	auto v = hostVoiceAllocationMode.getValue();

	const int bitPosistion = 19;
	const int mask = ~(1 << bitPosistion);
	hostVoiceAllocationMode = (hostVoiceAllocationMode & mask) | ((VoiceRefresh & 0x01) << bitPosistion);

	assert(hostVoiceAllocationMode == insertBits(v, 19, 1, VoiceRefresh));
}

void CK_MidiToCvGui15::onSetGlide()
{
	auto v = hostVoiceAllocationMode.getValue();

// should be 0xfffeffff	hostVoiceAllocationMode = ( hostVoiceAllocationMode & 0xfffcffff ) | ( GlideType << 16 );

//	assert(hostVoiceAllocationMode == insertBits(v, 16, 1, GlideType));

	hostVoiceAllocationMode = insertBits(v, 16, 1, GlideType);
}

void CK_MidiToCvGui15::onSetGlideTiming()
{
    auto v = hostVoiceAllocationMode.getValue();

	hostVoiceAllocationMode = ( hostVoiceAllocationMode & 0xfffbffff ) | ( GlideTiming << 18 );

	assert(hostVoiceAllocationMode == insertBits(v, 18, 1, GlideTiming));
}
*/

void CK_MidiToCvGui15::onSetVoiceAllocation()
{

    auto v = hostVoiceAllocationMode.getValue();
    auto mv = hostVoiceAllocationMode.getValue();

	//monoMode
	const int flags
		= MM_IN_USE
		| (monoMode ? MM_ON : 0)
		| (monoRetrigger ? MM_RETRIGGER : 0);

	v = insertBits(v, MonoModes_startbit, MonoModes_sizebits, flags);
    //VoiceSteal
	const int combinedVoiceAllocationMode = voiceStealMode & 0x03;
	v = ( v & 0xffffff00 ) | combinedVoiceAllocationMode;
	assert(v == insertBits(v, 0, 8, combinedVoiceAllocationMode));
    //NotePriority
	v = insertBits(v, NotePriority_startbit, NotePriority_sizebits, monoNotePriority);
    //GlideTiming
 	v = insertBits(v, 16, 1, GlideType);
    //GlideTiming
	v = ( v & 0xfffbffff ) | ( GlideTiming << 18 );
	assert(v == insertBits(v, 18, 1, GlideTiming));
    //VoiceRefresh
	const int bitPosistion = 19;
	const int mask = ~(1 << bitPosistion);
	v = (v & mask) | ((VoiceRefresh & 0x01) << bitPosistion);
	assert(v == insertBits(v, 19, 1, VoiceRefresh));

    if(mv != v)hostVoiceAllocationMode=v;
}


