//use gmpi::IProtectedFile* instead of gmpi::IProtectedFile2*
// I completly rewrote this file to use the graphic thread to load large SF2 files without broken the DSP stream !!

// prevent MS CPP - 'swprintf' was declared deprecated warning
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning(disable : 4996)

#include "SampleLoader3Gui.h"
#include "RiffFile2.h"
#include "CSoundFont.h"
#include "SampleManager.h"


//#include <string>
//#include <fstream>
#include <sstream>
//using namespace std;

REGISTER_GUI_PLUGIN( SampleLoader3Gui, L"SampleLoader4Gui" );

SampleLoader3Gui::SampleLoader3Gui(IMpUnknown* host) : MpGuiBase(host)
,init(0)
{
	initializePin( 0, pinFilename );
	initializePin( 1, pinBank );
 	initializePin( 2, pinPatch );
	initializePin( 3, pinBankNames );
	initializePin( 4, pinPatchNames );
 	initializePin( 5, pinPatchReset );
 	initializePin( 6, pinBankReset );
    initializePin( 7, pinIdToDsp );
    initializePin( 8, pinIdToGui );
    initializePin( 9, pinDebugVST3);
    initializePin( 10, pinLoadToGui, static_cast<MpGuiBaseMemberPtr>( &SampleLoader3Gui::test ) ); //to call the loading when this pin is updated);
}
int32_t SampleLoader3Gui::initialize()//init
{
        SampleManager::Instance()->GeditOn(1);
        if(init==0)test();//Gui editor is updated without possibility to load...
        //After the initialisation, the LoadToGui pin can when it's updated call the gui editor to load.
        init=1;// now the Gui editor can call the sample manager to load the sf2!
	    buzy=0;
	    return MpGuiBase::initialize();
}
SampleLoader3Gui::~SampleLoader3Gui()
{
    init=0;
    SampleManager::Instance()->GeditOn(0);
}
void SampleLoader3Gui::test()
{

        int LoadTest=0;

        //const int maxpath = 500;
        //wchar_t fullFilename[maxpath];
        //getHost()->resolveFilename( pinFilename.getValue().c_str(), maxpath, fullFilename ); //max path =260

        std::wstring filename = pinFilename.getValue();
        wchar_t fullFilename[1024];
        getHost()->resolveFilename(filename.c_str(), sizeof(fullFilename) / sizeof(fullFilename[0]), fullFilename);

        std::wstring pathR = fullFilename;

        //03 empty test
        if(!pinFilename.getValue().empty())
        {
            //02 length test
            if(pathR.length()>7)
            {
                //01 .SF2 test
                if(pathR[pathR.length()-1]==L'2' && (pathR[pathR.length()-2]==L'f' || pathR[pathR.length()-2]==L'F') && (pathR[pathR.length()-3]==L's' || pathR[pathR.length()-3]==L'S')&& pathR[pathR.length()-4]==L'.')
                {
                        //00 file test
                        //open imbedded (or not) file.
                        //Code based of stable sdk3, use of gmpi::IProtectedFile instead of gmpi::IProtectedFile2

                        gmpi::IProtectedFile* file = 0;//CK changes...
                        int r = getHost()->openProtectedFile( fullFilename, &file );//CK changes...

                        if( r == gmpi::MP_OK)//CK changes...
                        {

                            RiffFile2 riff;
                            uint32_t riff_type;

                            riff.Open( file, riff_type );//CK changes...

                            if( riff_type != MAKEFOURCC('s', 'f', 'b', 'k') )	// sf2 file
                            {
                            file->close();//CK changes...
                            pinPatchNames = L"0";//CK changes..
                            pinBankNames = L"0";//CK changes..
                            pinPatchReset = 0;//CK changes..
                            pinBankReset = 0;//CK changes..
                            LoadTest=0;
                            return;
                            }

                            unsigned int count_phdr;
                            sfPresetHeader* chunk_phdr = 0;

                            riff.REG_CHUNK( "Preset Headers", "pdta", "phdr", &chunk_phdr, &count_phdr, sfPresetHeader );

                            riff.ReadFile();

                            file->close();//CK changes...

                            std::string presetNames;

                            std::map<int,std::string> presets;
                            std::map<int,int> banks;


                            if( chunk_phdr == 0 ) // file not loaded or corrupt
                            {
                            pinPatchNames = L"0";//CK changes..
                            pinBankNames = L"0";//CK changes..
                            pinPatchReset = 0;//CK changes..
                            pinBankReset = 0;//CK changes..
                            LoadTest=0;
                            return;
                            }

                             sfPresetHeader* last_phdr = chunk_phdr + count_phdr - 1;

                                // collect bank numbers, duplicates will fail to insert.
                                for( sfPresetHeader* phdr = chunk_phdr ; phdr < last_phdr ; ++phdr )
                                {
                                    banks.insert( std::pair<int,int>( phdr->wBank, 0) );
                                }

                                // collect patch numbers for current bank.
                                int currentBank = pinBank;
                                {
                                    auto it = banks.find(pinBank);
                                    if (it == banks.end())
                                    {
                                        if (banks.empty())
                                        {
                                            currentBank = 0;
                                        }
                                        else
                                        {
                                            currentBank = banks.begin()->first; // choose first availalbe bank.
                                        }
                                    }
                                }

                                for( sfPresetHeader* phdr = chunk_phdr ; phdr < last_phdr ; ++phdr )
                                {
                                    if( currentBank == phdr->wBank )
                                    {
                                        presets.insert( std::pair<int,std::string>( phdr->wPreset, phdr->achPresetName) );
                                    }
                                }



                            delete [] (char*) chunk_phdr;

                            // Convert patch names to UNICODE wide-string.
                            std::wstring presetNamesWideChar;

                            if( presets.size() != 0 )
                            {
                                presetNames.clear();
                                std::ostringstream os;

                                int index = 0;
                                for( std::map<int,std::string>::iterator it = presets.begin() ; it != presets.end() ; ++it )
                                {
                                    if( index > 0 )
                                    {
                                        os << ',';
                                    }

                                    // sprintf( presetName, "%3d %s", (*it).first, (*it).second.c_str() );
                                    os << (*it).first << ' ' << (*it).second.c_str();

                                    // Add index if needed e.g. ",mypatch=23".
                                    if( (*it).first != index )
                                    {
                                        index = (*it).first;
                                        //sprintf( presetName, "=%d", index );
                                        //presetNames.append( presetName );
                                        os << '=' << index;
                                    }

                                    ++index;
                                }

                                presetNames = os.str();

                                presetNamesWideChar.clear();
                                presetNamesWideChar.assign( presetNames.length(), L' ' );
                                std::copy( presetNames.begin(), presetNames.end(), presetNamesWideChar.begin() );
                                pinPatchNames = presetNamesWideChar;
                            }
                            else
                            {
                                pinPatchNames = L"0";
                            }

                            // convert bank numbers to string.
                            if( banks.size() != 0 )
                            {
                                presetNames.clear();
                                std::ostringstream os;

                                int index = 0;
                                for( std::map<int,int>::iterator it = banks.begin() ; it != banks.end() ; ++it )
                                {
                                    if( index > 0 )
                                    {
                                        os << ',';
                                    }
                                    // sprintf( presetName, "%d", (*it).first );
                                    os << (*it).first;

                                    // Add index if needed e.g. ",mypatch=23".
                                    if( (*it).first != index )
                                    {
                                        index = (*it).first;
                                        //sprintf( presetName, "=%d", index );
                                        //presetNames.append( presetName );
                                        os << '=' << index;
                                    }

                                    ++index;
                                }

                                presetNames = os.str();
                                presetNamesWideChar.clear();
                                presetNamesWideChar.assign( presetNames.length(), L' ' );
                                std::copy( presetNames.begin(), presetNames.end(), presetNamesWideChar.begin() );
                                pinBankNames = presetNamesWideChar;
                            }
                            else
                            {
                                pinBankNames = L"0";
                            }

                            // Ensure bank number is valid for this soundfont.
                            pinBank = currentBank;

                            // Ensure patch number is valid for this soundfont.
                            {
                                auto it2 = presets.find(pinPatch);
                                if (it2 == presets.end())
                                {
                                    if (!presets.empty())
                                    {
                                     pinPatch = presets.begin()->first; // choose first availalbe preset.
                                    }
                                    else
                                    {
                                     pinPatch = 0;
                                    }
                                }
                            }

                            if(pinDebugVST3==1)pinBankReset = pinBank;//following the vst3 tests possible to check with..
                            pinPatchReset = pinPatch;

                            LoadTest=1;//to load the file after the Gui editor update

                            }//00 file test
                            else
                            {
                            pinPatchNames = L"0";//CK changes..
                            pinBankNames = L"0";//CK changes..
                            pinPatchReset = 0;//CK changes..
                            pinBankReset = 0;//CK changes..
                            LoadTest=0;
                            }
                    }//01 .SF2 test
                    else
                    {
                    pinPatchNames = L"0";//CK changes..
                    pinBankNames = L"0";//CK changes..
                    pinPatchReset = 0;//CK changes..
                    pinBankReset = 0;//CK changes..
                    LoadTest=0;
                    }
                }//02 length test
                else
                {
                pinPatchNames = L"0";//CK changes..
                pinBankNames = L"0";//CK changes..
                pinPatchReset = 0;//CK changes..
                pinBankReset = 0;//CK changes..
                LoadTest=0;
                }
            }// 03 empty test
            else
            {
            pinPatchNames = L"0";//CK changes..
            pinBankNames = L"0";//CK changes..
            pinPatchReset = 0;//CK changes..
            pinBankReset = 0;//CK changes..
            LoadTest=0;
            }


            if(init==1 && buzy==0)load(LoadTest);



}
void SampleLoader3Gui::load(int loading)
{

        buzy=1;//to avoid loading when loading ^^, not sure it's necessary

        //const int maxpath = 500;
        //wchar_t fullFilename[maxpath];
        //getHost()->resolveFilename( pinFilename.getValue().c_str(), maxpath, fullFilename ); //max path =260
        std::wstring filename = pinFilename.getValue();
        wchar_t fullFilename[1024];
        getHost()->resolveFilename(filename.c_str(), sizeof(fullFilename) / sizeof(fullFilename[0]), fullFilename);

        std::wstring pathR = fullFilename;


        if(loading==1)//tests for loading ok
        {
            int oldSamplehandle = sampleHandle;
            // open imbedded file.
            gmpi::IProtectedFile* file = 0;
            int r = getHost()->openProtectedFile( fullFilename, &file );

            if (r == gmpi::MP_OK )
            {

                sampleHandle = SampleManager::Instance()->Load( file, fullFilename, pinBank, pinPatch  );
                pinIdToDsp=sampleHandle;
                if(oldSamplehandle!=-1)SampleManager::Instance()->Release( oldSamplehandle );
                file->close();

               //wchar_t msg[200];
               //swprintf(msg, 200, L"GUI->SF2 loaded %d", sampleHandle);
               //MessageBox( 0, msg, fullFilename,MB_OK );
            }
        }

        if(loading==0 )//tests for loading failed
        {
        int oldSamplehandle = sampleHandle;
        pinIdToDsp=sampleHandle=-1;
        if(oldSamplehandle!=-1)SampleManager::Instance()->Release( oldSamplehandle );
        }

    buzy=0;
}
