//use gmpi::IProtectedFile* instead of gmpi::IProtectedFile2*
// I completly rewrote this file to use the graphic thread to load large SF2 files without broken the DSP stream !!

// prevent MS CPP - 'swprintf' was declared deprecated warning
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning(disable : 4996)

#include "SampleLoader3Gui.h"
#include "rifffile2.h"
#include "csoundFont.h"
#include "SampleManager.h"


//#include <string>
//#include <fstream>
#include <sstream>
//using namespace std;

REGISTER_GUI_PLUGIN( SampleLoader3Gui, L"SampleLoader4Gui" );

SampleLoader3Gui::SampleLoader3Gui(IMpUnknown* host) : MpGuiBase(host)
{

	initializePin( 0, pinFilename, static_cast<MpGuiBaseMemberPtr>( &SampleLoader3Gui::test ) ); //to call the loading when this pin is updated
	initializePin( 1, pinBank, static_cast<MpGuiBaseMemberPtr>( &SampleLoader3Gui::test ) ); //to call the loading when this pin is updated
	initializePin( 2, pinPatch, static_cast<MpGuiBaseMemberPtr>( &SampleLoader3Gui::test ) ); //to call the loading when this pin is updated
	initializePin( 3, pinBankNames );
	initializePin( 4, pinPatchNames );
 	initializePin( 5, pinPatchReset );
 	initializePin( 6, pinBankReset );
    initializePin( 7, pinIdToDsp );
    initializePin( 8, pinIdToGui );
    initializePin( 9, pinRndIn, static_cast<MpGuiBaseMemberPtr>( &SampleLoader3Gui::test ));
    //initializePin( 9, pinRndIn );

    init=0; // alway set to 0 because the gui pins are alway initialized when the gui editor is going to on.
    //memFilename=L"";
    //memBank=0;
    //memPatch=0;
}
int32_t SampleLoader3Gui::initialize()//init
{


        SampleManager::Instance()->GeditOn (1);
        //sampleHandle=pinIdToGui;
        init=1;// now the gui editor can call the sample manager to load the sf2!!
        //init=0;
        //test();

	    return MpGuiBase::initialize();
}
SampleLoader3Gui::~SampleLoader3Gui()
{
    init=0;
    SampleManager::Instance()->GeditOn (0);
}
/*void SampleLoader3Gui::loadingGUI()
{
    if(pinLoadingGui==1.0f)
    {
    if(init==0)sampleHandle=pinIdToGui;
    init=1;
    SampleManager::Instance()->GeditOn (1);
    }

}*/

void SampleLoader3Gui::test()
{
    int loading=0;

    const int maxpath = 500;
    wchar_t fullFilename[maxpath];
    getHost()->resolveFilename( pinFilename.getValue().c_str(), maxpath, fullFilename ); //max path =260

    std::wstring pathR = fullFilename;


    if(!pinFilename.getValue().empty())
    {




                    if(pathR.length()>7)
                    {

                            if(pathR[pathR.length()-1=='2'] && (pathR[pathR.length()-2]=='f' || pathR[pathR.length()-2]=='F') && (pathR[pathR.length()-3]=='s' || pathR[pathR.length()-3]=='S')&& pathR[pathR.length()-4]=='.')
                            {

                                //std::ifstream file(pathR);
                                //if ( file )

                                // open imbedded (or not) file.
                                gmpi::IProtectedFile* file = 0;
                                int r = getHost()->openProtectedFile( fullFilename, &file );

                                if( r == gmpi::MP_OK )
                                {


                                            //CRiffFile riff;
                                            //DWORD riff_type;

                                            RiffFile2 riff;
                                            uint32_t riff_type;



                                            //if( r != gmpi::MP_OK )
                                            //{
                                               //pinPatchNames = L"<none>";
                                               // pinBankNames = L"<none>";
                                                //return;
                                            //}

                                            riff.Open( file, riff_type );
                                            //riff.Open( fullFilename, riff_type );
                                            // Create list of chunks we could use

                                            //if( riff_type != mmioFOURCC('s', 'f', 'b', 'k') )	// sf2 file
                                            if( riff_type != MAKEFOURCC('s', 'f', 'b', 'k') )	// sf2 file
                                            {
                                            pinPatchNames = L"0";
                                            pinBankNames = L"0";
                                            pinPatchReset = 0;
                                            pinBankReset = 0;
                                            file->close();
                                            //file.close();
                                            return;
                                            }

                                            unsigned int count_phdr;
                                            sfPresetHeader* chunk_phdr = 0;

                                            riff.REG_CHUNK( "Preset Headers", "pdta", "phdr", &chunk_phdr, &count_phdr, sfPresetHeader );

                                            riff.ReadFile();

                                            //file.close();
                                            file->close();

                                            std::string presetNames;

                                            std::map<int,std::string> presets;
                                            std::map<int,int> banks;


                                            if( chunk_phdr == 0 ) // file not loaded or corrupt
                                            {
                                            pinPatchNames = L"0";
                                            pinBankNames = L"0";
                                            pinPatchReset = 0;
                                            pinBankReset = 0;
                                            return;
                                            }


                                            /*sfPresetHeader *phdr = chunk_phdr;
                                            sfPresetHeader *last_phdr = chunk_phdr + count_phdr - 1;

                                            // collect patch/bank numbers, duplicates will fail to insert.
                                            int currentBank = pinBank;
                                            while( phdr < last_phdr )
                                            {
                                                banks.insert( std::pair<int,int>( phdr->wBank, 0) );
                                                if( currentBank == phdr->wBank )
                                                {
                                                    presets.insert( std::pair<int,std::string>( phdr->wPreset, phdr->achPresetName) );
                                                }
                                                ++phdr;
                                            }*/

                                            //to prevent patch name bug... KX' changes
                                            //std::map<int,std::string>::iterator it2 = presets.find(pinPatch);
                                            //if( it2 == presets.end() )
                                            //{
                                                //pinPatch = presets.begin()->first; // choose first availalbe preset.
                                            //}
                                            //new SDK3 code
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

                                           /* // Convert patch names to UNICODE wide-string.
                                            std::wstring presetNamesWideChar;

                                            if( presets.size() != 0 )
                                            {
                                                presetNames.clear();

                                                int index = 0;
                                                for( std::map<int,std::string>::iterator it = presets.begin() ; it != presets.end() ; ++it )
                                                {
                                                    char presetName[100];
                                                    if( index == 0 )
                                                    {
                                                        sprintf( presetName, "%3d %s", (*it).first, (*it).second.c_str() );
                                                    }
                                                    else
                                                    {
                                                        sprintf( presetName, ",%3d %s", (*it).first, (*it).second.c_str() );
                                                    }
                                                    presetNames.append( presetName );

                                                    // Add index if needed e.g. ",mypatch=23".
                                                    if( (*it).first != index )
                                                    {
                                                        index = (*it).first;
                                                        sprintf( presetName, "=%d", index );
                                                        presetNames.append( presetName );
                                                    }

                                                    ++index;
                                                }

                                                presetNamesWideChar.clear();
                                                presetNamesWideChar.assign( presetNames.length(), L' ' );
                                                std::copy( presetNames.begin(), presetNames.end(), presetNamesWideChar.begin() );
                                                pinPatchNames = presetNamesWideChar;
                                            }
                                            else
                                            {
                                                pinPatchNames = L"0";
                                                pinPatchReset = 0;
                                            }

                                            // convert bank numbers to string.
                                            if( banks.size() != 0 )
                                            {
                                                presetNames.clear();

                                                int index = 0;
                                                for( std::map<int,int>::iterator it = banks.begin() ; it != banks.end() ; ++it )
                                                {
                                                    char presetName[10];

                                                    if( index == 0 )
                                                    {
                                                        sprintf( presetName, "%d", (*it).first );
                                                    }
                                                    else
                                                    {
                                                        sprintf( presetName, ",%d", (*it).first );
                                                    }
                                                    presetNames.append( presetName );

                                                    // Add index if needed e.g. ",mypatch=23".
                                                    if( (*it).first != index )
                                                    {
                                                        index = (*it).first;
                                                        sprintf( presetName, "=%d", index );
                                                        presetNames.append( presetName );
                                                    }

                                                    ++index;
                                                }

                                                presetNamesWideChar.clear();
                                                presetNamesWideChar.assign( presetNames.length(), L' ' );
                                                std::copy( presetNames.begin(), presetNames.end(), presetNamesWideChar.begin() );
                                                pinBankNames = presetNamesWideChar;
                                            }
                                            else
                                            {
                                                pinBankNames = L"0";
                                                pinBankReset = 0;
                                            }*/

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

                                            pinBankReset = pinBank;
                                            pinPatchReset = pinPatch;


                                            loading=1;
                                            //memFilename=fullFilename;
                                            //memBank=pinBank;
                                            //memPatch=pinPatch;


                                }
                                else
                                {

                                pinPatchNames = L"0";
                                pinBankNames = L"0";
                                pinPatchReset = 0;
                                pinBankReset = 0;
                                loading=0;
                                //memFilename=fullFilename;
                                //memBank=0;
                                //memPatch=0;
                                }

                        }
                        else
                        {

                        pinPatchNames = L"0";
                        pinBankNames = L"0";
                        pinPatchReset = 0;
                        pinBankReset = 0;
                        loading=0;
                        //memFilename=fullFilename;
                        //memBank=0;
                        //memPatch=0;
                        }
                    }
                    else
                    {

                    pinPatchNames = L"0";
                    pinBankNames = L"0";
                    pinPatchReset = 0;
                    pinBankReset = 0;
                    loading=0;
                    //memFilename=fullFilename;
                    //memBank=0;
                    //memPatch=0;
                    }

    }
    else
    {

    pinPatchNames = L"0";
    pinBankNames = L"0";
    pinPatchReset = 0;
    pinBankReset = 0;
    loading=0;
    //memFilename=fullFilename;
    //memBank=0;
    //memPatch=0;
    }



    if(init==1 && (memFilename!=fullFilename ||  memBank!=pinBank || memPatch!=pinPatch || memRnd!=pinRndIn))
    {
    load(loading);
    memFilename=fullFilename;
    memBank=pinBank;
    memPatch=pinPatch;
    memRnd=pinRndIn;
    }


/*  if(init==1)
  {
    load(loading);
    memFilename=fullFilename;
    memBank=pinBank;
    memPatch=pinPatch;
    memRnd=pinRndIn;
  }
*/

  if(init==0)
  {
  sampleHandle=pinIdToGui;
  memFilename=fullFilename;
  memBank=pinBank;
  memPatch=pinPatch;
  memRnd=pinRndIn;
  }

}
void SampleLoader3Gui::load(int loading)
{

        const int maxpath = 500;
        wchar_t fullFilename[maxpath];
        getHost()->resolveFilename( pinFilename.getValue().c_str(), maxpath, fullFilename ); //max path =260

        std::wstring pathR = fullFilename;





        if(loading==1)//if the gui module is initialized
        {
            //if(sampleHandle!=-1)SampleManager::Instance()->Release( sampleHandle );
            int oldSamplehandle = sampleHandle;

            //sampleHandle = SampleManager::Instance()->Load( fullFilename, pinBank, pinPatch  );

            // open imbedded file.
            gmpi::IProtectedFile* file = 0;
            int r = getHost()->openProtectedFile( fullFilename, &file );

            if (r == gmpi::MP_OK )
            {

                sampleHandle = SampleManager::Instance()->Load( file, fullFilename, pinBank, pinPatch  );



            pinIdToDsp=sampleHandle;
            if(oldSamplehandle!=-1)SampleManager::Instance()->Release( oldSamplehandle );
            memFilename=fullFilename;
            memBank=pinBank;
            memPatch=pinPatch;
            //wchar_t msg[200];
            //swprintf(msg, 200, L"%d", memBank+memPatch);
            //MessageBox( 0, msg, memFilename,MB_OK );
            file->close();
            }
        }


        if(loading==0 )//if the gui module is initialized
        {
        //if(sampleHandle!=-1)SampleManager::Instance()->Release( sampleHandle );
        int oldSamplehandle = sampleHandle;
        pinIdToDsp=sampleHandle=-1;
        if(oldSamplehandle!=-1)SampleManager::Instance()->Release( oldSamplehandle );
        }



/*	LAST sdk3 CODE


    RiffFile2 riff;
	uint32_t riff_type;

	// open imbedded (or not) file.
	gmpi::IProtectedFile* file = 0;

	int r = getHost()->openProtectedFile( pinFilename.getValue().c_str(), &file );

	if( r != gmpi::MP_OK )
	{
		pinPatchNames = L"<none>";
		pinBankNames = L"<none>";
		return;
	}

	riff.Open( file, riff_type );

	// Create list of chunks we could use
	if( riff_type != MAKEFOURCC('s', 'f', 'b', 'k') )	// sf2 file
	{
		file->close();
		return;
	}

	unsigned int count_phdr;
	sfPresetHeader* chunk_phdr = 0;

	riff.REG_CHUNK( "Preset Headers", "pdta", "phdr", &chunk_phdr, &count_phdr, sfPresetHeader );

	riff.ReadFile();

	file->close();

	std::string presetNames;

	std::map<int,std::string> presets;
	std::map<int,int> banks;

	if( chunk_phdr == 0 ) // file not loaded or corrupt
	{
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
		pinPatchNames = L"";
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
		pinBankNames = L"";
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
	}*/
}
