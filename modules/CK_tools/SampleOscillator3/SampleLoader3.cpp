//use gmpi::IProtectedFile* instead of gmpi::IProtectedFile2*

#include "SampleLoader3.h"
#include "SampleManager.h"

//#include <time.h>

REGISTER_PLUGIN ( SampleLoader3, L"SampleLoader4Gui" );

SampleLoader3::SampleLoader3( IMpUnknown* host ) : MpBase( host )
{
	// Register pins.
	initializePin( 0, pinFilename );
	initializePin( 1, pinBank );
	initializePin( 2, pinPatch );
	initializePin( 3, pinSampleId );
	initializePin( 4, pinFilenameG );
	initializePin( 5, pinBankG );
	initializePin( 6, pinPatchG );
	initializePin( 7, pinGuiSampleId );
	initializePin( 8, pinIdToGui );
	initializePin( 9, pinLoadToGui );

    sampleHandle=-1;
    loading=0;
}
SampleLoader3::~SampleLoader3()
{
	SampleManager::Instance()->Release( sampleHandle );
}

void SampleLoader3::onSetPins(void)
{

    if( pinFilename.isUpdated() || pinBank.isUpdated() || pinPatch.isUpdated() )
    {
            pinFilenameG=pinFilename;;
            //pinFilenameG.sendPinUpdate();
            pinBankG=pinBank;
            //pinBankG.sendPinUpdate();
            pinPatchG=pinPatch;
            //pinPatchG.sendPinUpdate();

            //srand(0);
            //float RANGE_MIN = 0.0f;
            //float  RANGE_MAX = 2147483647.0f;
            //int  r= int(((float) rand() / (float) RAND_MAX) * RANGE_MAX + RANGE_MIN);
            pinLoadToGui=1;
            pinLoadToGui.sendPinUpdate();

            int edit=SampleManager::Instance()->TestOn();//TestOn returns if the GUI is on
            if(loading==0 && edit==0)load();//to load with the dsp if the gui editor is off
    }

    if(pinGuiSampleId.isUpdated())
    {
    // to update the samplehandle and all out pins when the gui module loaded the sf2
    // by this way the sf2 bank will be unloaded if the user will change the preset when the gui editor will be off
    int edit=SampleManager::Instance()->TestOn();
    if(edit==1)pinSampleId = sampleHandle = pinGuiSampleId;
    }
}
void SampleLoader3::load()//to call the loading with the sample manager
{

            loading=1;


            if(!pinFilename.getValue().empty())// if not empty
            {

            //const int maxpath = 500;
            //wchar_t fullFilename[maxpath];
            //getHost()->resolveFilename( pinFilename.getValue().c_str(), maxpath, fullFilename ); //max path =260

            std::wstring filename = pinFilename.getValue();
            wchar_t fullFilename[500];
            getHost()->resolveFilename(filename.c_str(), sizeof(fullFilename) / sizeof(fullFilename[0]), fullFilename);

            std::wstring pathR = fullFilename;// unicode

                if(pathR.length()>7)// path min like "c:\x.sf2"
                {

                        //test .sf2 and .SF2
                        if(pathR[pathR.length()-1]==L'2' && (pathR[pathR.length()-2]==L'f' || pathR[pathR.length()-2]==L'F') && (pathR[pathR.length()-3]==L's' || pathR[pathR.length()-3]==L'S')&& pathR[pathR.length()-4]==L'.')
                        {

                            // open imbedded file.
                            gmpi::IProtectedFile* file = 0;
                            int r = getHost()->openProtectedFile( fullFilename, &file );


                            if ( r == gmpi::MP_OK ) // to test if the file exist
                            {
                                int oldSamplehandle = sampleHandle;

                                sampleHandle = SampleManager::Instance()->Load( file, fullFilename, pinBank, pinPatch  );

                                pinIdToGui = pinSampleId = sampleHandle; // important to update all pins
                                if(oldSamplehandle!=-1)SampleManager::Instance()->Release( oldSamplehandle );

                                file->close();
                                //wchar_t msg[200];
                                //swprintf(msg, 200, L"DSP->SF2 loaded %d", sampleHandle);
                                //MessageBox( 0, msg, fullFilename,MB_OK );
                            }
                            else
                            {
                            int oldSamplehandle = sampleHandle;
                            pinIdToGui = pinSampleId = sampleHandle = -1;// important to update all pins
                            if(oldSamplehandle!=-1)SampleManager::Instance()->Release( oldSamplehandle );
                            }

                        }
                        else
                        {
                        int oldSamplehandle = sampleHandle;
                        pinIdToGui = pinSampleId = sampleHandle = -1;// important to update all pins
                        if(oldSamplehandle!=-1)SampleManager::Instance()->Release( oldSamplehandle );
                        }

                   }
                   else
                   {
                    int oldSamplehandle = sampleHandle;
                    pinIdToGui = pinSampleId = sampleHandle =-1;// important to update all pins
                    if(oldSamplehandle!=-1)SampleManager::Instance()->Release( oldSamplehandle );
                   }
                }
                else
                {
                int oldSamplehandle = sampleHandle;
                pinIdToGui = pinSampleId = sampleHandle = -1;// important to update all pins
                if(oldSamplehandle!=-1)SampleManager::Instance()->Release( oldSamplehandle );
                }

                loading=0;


}




