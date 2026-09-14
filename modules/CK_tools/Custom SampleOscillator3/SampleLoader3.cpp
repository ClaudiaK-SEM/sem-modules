//use gmpi::IProtectedFile* instead of gmpi::IProtectedFile2*

#include "SampleLoader3.h"
#include "SampleManager.h"

//#include <string>
//#include <fstream>
//using namespace std;


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

    sampleHandle=-1;
    loading=0;

    //memFilename=L"";
    //memBank=0;
    //memPatch=0;
}
SampleLoader3::~SampleLoader3()
{
	SampleManager::Instance()->Release( sampleHandle );
}

void SampleLoader3::onSetPins(void)
{
    //bool tmp=&SampleLoader3Gui::t;

    int edit=SampleManager::Instance()->TestOn();

    if( pinFilename.isUpdated() || pinBank.isUpdated() || pinPatch.isUpdated() )
    {
            //update gui com pins!!
            if( pinFilename.isUpdated())
            {
            pinFilenameG=pinFilename;
            //pinFilenameG.sendPinUpdate();
            }
            if( pinBank.isUpdated() )
            {
            pinBankG=pinBank;
            //pinBankG.sendPinUpdate();
            }

            if(pinPatch.isUpdated() )
            {
            pinPatchG=pinPatch;
            //pinPatchG.sendPinUpdate();
            }



        if(loading==0 && edit==0)load();//to load with the dsp if the gui editor is off

    }
    // to update the samplehandle and all out pins when the gui module loaded the sf2
    // by this way the sf2 bank will be unloaded if the user will change the preset when the gui editor will be off
    if(edit==1 && pinGuiSampleId.isUpdated())pinSampleId = sampleHandle = pinGuiSampleId;
}
void SampleLoader3::load()//to call the loading with the sample manager
{

                        loading=1;


                        if(!pinFilename.getValue().empty())// if not empty
                        {
                        //wchar_t fullFilename[MAX_PATH];
                        //getHost()->resolveFilename( pinFilename.getValue().c_str(), MAX_PATH, fullFilename ); //max path =260

                        const int maxpath = 500;
                        wchar_t fullFilename[maxpath];
                        getHost()->resolveFilename( pinFilename.getValue().c_str(), maxpath, fullFilename ); //max path =260

                        std::wstring pathR = fullFilename;// unicode

                            if(pathR.length()>7)// path min like "c:\x.sf2"
                            {

                                    //test .sf2 and .SF2
                                    if(pathR[pathR.length()-1=='2'] && (pathR[pathR.length()-2]=='f' || pathR[pathR.length()-2]=='F') && (pathR[pathR.length()-3]=='s' || pathR[pathR.length()-3]=='S')&& pathR[pathR.length()-4]=='.')
                                    {


                                        //std::ifstream file(pathR);

                                        // open imbedded file.
                                        gmpi::IProtectedFile* file = 0;
                                        int r = getHost()->openProtectedFile( fullFilename, &file );


                                        if ( r == gmpi::MP_OK ) // to test if the file exist
                                        {
                                            //if(memFilename!=fullFilename || memBank!=pinBank || memPatch!=pinPatch)
                                            //{
                                            int oldSamplehandle = sampleHandle;
                                            //if(sampleHandle!=-1)SampleManager::Instance()->Release( sampleHandle );

                                            //sampleHandle = SampleManager::Instance()->Load( fullFilename, pinBank, pinPatch  );

			                                sampleHandle = SampleManager::Instance()->Load( file, fullFilename, pinBank, pinPatch  );

                                            pinIdToGui = pinSampleId = sampleHandle; // important to update all pins
                                            if(oldSamplehandle!=-1)SampleManager::Instance()->Release( oldSamplehandle );

                                            //memFilename=fullFilename;
                                            //memBank=pinBank;
                                            //memPatch=pinPatch;
                                            //}
                                        //file.close();// !!
                                        file->close();
                                        }
                                        else
                                        {
                                        //if(sampleHandle!=-1)SampleManager::Instance()->Release( sampleHandle );
                                        int oldSamplehandle = sampleHandle;
                                        pinIdToGui = pinSampleId = sampleHandle = -1;// important to update all pins
                                        if(oldSamplehandle!=-1)SampleManager::Instance()->Release( oldSamplehandle );
                                        }

                                    }
                                    else
                                    {
                                    //if(sampleHandle!=-1)SampleManager::Instance()->Release( sampleHandle );
                                    int oldSamplehandle = sampleHandle;
                                    pinIdToGui = pinSampleId = sampleHandle = -1;// important to update all pins
                                    if(oldSamplehandle!=-1)SampleManager::Instance()->Release( oldSamplehandle );
                                    }

                               }
                               else
                               {
                                //if(sampleHandle!=-1)SampleManager::Instance()->Release( sampleHandle );
                                int oldSamplehandle = sampleHandle;
                                pinIdToGui = pinSampleId = sampleHandle =-1;// important to update all pins
                                if(oldSamplehandle!=-1)SampleManager::Instance()->Release( oldSamplehandle );
                               }
                            }
                            else
                            {
                            //if(sampleHandle!=-1)SampleManager::Instance()->Release( sampleHandle );
                            int oldSamplehandle = sampleHandle;
                            pinIdToGui = pinSampleId = sampleHandle = -1;// important to update all pins
                            if(oldSamplehandle!=-1)SampleManager::Instance()->Release( oldSamplehandle );
                            }

                            loading=0;


}




