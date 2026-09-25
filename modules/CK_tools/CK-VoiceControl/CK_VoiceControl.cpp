
#include "CK_VoiceControl.h"


SE_DECLARE_INIT_STATIC_FILE(CK_VoiceControl)
REGISTER_PLUGIN2 ( CK_VoiceControl, L"CK_VoiceControl" );

CK_VoiceControl::CK_VoiceControl( )
{
	initializePin(BlobToGui);
	initializePin(pinPolyphony);
	initializePin(pinPolyRes);
	initializePin(pinPolyMode);
	initializePin(pinMono);
	initializePin(pinRetrigger);
	initializePin(pinMonoPiority);
	initializePin(pinGlide);
	initializePin(pinGlideRate);
	initializePin(pinAutoGlide);
	initializePin(pinBRange);
	initializePin(pinVoiceRefresh);
	initializePin(pinMidiToCv);
	initializePin(pinPolyGlide);
}

int32_t CK_VoiceControl::open()
{
	MpBase2::open();	// always call the base class

    init=1;
    m_Glide=-1.0f;
    BlobOn=0;

	return gmpi::MP_OK;
}

void CK_VoiceControl::onSetPins()
{

    GlideM=1.0f;
    autoG = float(pinAutoGlide);
    timeG = float(pinGlideRate);
    if(pinMidiToCv==1)//GlideRate and AutoGlide values inverted or not, Portamento value multiplied by 0.85 to have same settings of old MidiToCv
    {
    GlideM=0.85f;
    if(autoG==0.0f)autoG=1.0f;
    else
    autoG=0.0f;
    if(timeG==0.0f)timeG=1.0f;
    else
    timeG=0.0f;
    }
    if(pinPolyGlide==0 && pinMono==0)GlideM=0.000f;//to preserve presets based on old MidiToCv with Glide fx if they are in Mono mode even if Poly Glide is off.

    Glide=GlideM*pinGlide;

    if(pinGlide.isUpdated() || m_Glide != Glide )//even if the module does not use HostConnect pin, better it's to save the value of this pin in case of future use
    {
    m_Glide = Glide;
    p[0]=7.f;//onSetPortamento();
    BlobOn=1;
    }

    if(pinBRange.isUpdated())
    {
    p[0]=10.f;//onSetBendRange();
    BlobOn=1;
    }

    //to update VoiceAllocation HostConnect pin in only one time
    if(pinMidiToCv.isUpdated() || pinMono.isUpdated() || pinRetrigger.isUpdated() || pinMonoPiority.isUpdated() || pinPolyMode.isUpdated() || pinGlideRate.isUpdated() || pinAutoGlide.isUpdated() || pinVoiceRefresh.isUpdated())
    {
    p[0]=12.f;//onSetVoiceAllocation() and onSetPortamento()
    BlobOn=1;
    }

	if(pinPolyphony.isUpdated() || pinPolyRes.isUpdated())//to update these two hostConnect pins
    {
    p[0]=13.f;//onSetPolyAndReserve()
    BlobOn=1;
    }

	if(init==1)//ensure the BlobToGui will be updated on initialization, p[0]=14 -> all hostConnect pins of GUI part of module will be updated
    {
    p[0]=14.f;//onSetBendRange(), onSetVoiceAllocation(), onSetPortamento(), onSetPolyAndReserve()
    init=0;// works only on first onSetPins() update
    BlobOn=1;//update the BlobToGui parameter to communicate with GUI hostConnect pins
    }

	if(BlobOn==1)//simple float Blob, float to int works fine so... P[0] is the ID to select which HostConnect will be updated
	{
    BlobOn=0;
    p[1]=float(pinMono);//mono mode
    p[2]=float(pinRetrigger);// retrig on/off
    p[3]=float(pinMonoPiority);//off, low, high, last
    p[4]=float(pinPolyphony);//Number of voices
    p[5]=float(pinPolyRes);// Reserve of voices
    p[6]=float(pinPolyMode);// Soft,Hard, Over
    p[7]=Glide; //Glide value * 1 or * 0.85 (compatibility of presets done with old MidiToCv)
    p[8]=timeG;//GlideRate value inverted or not (compatibility of presets done with old MidiToCv)
    p[9]=autoG;//AutoGlide value inverted or not (compatibility of presets done with old MidiToCv)
    p[11]=float(pinVoiceRefresh); //1=off

    BlobToGui.setValueRaw( sizeof(p), &p );
    BlobToGui.sendPinUpdate();
    }
}

