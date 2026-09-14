#include "FullPathX.h"
#include "../shared/unicode_conversion.h"
#include "../shared/it_enum_list.h"
#include "../shared/string_utilities.h"
#include "../se_sdk3/MpString.h"
#include <filesystem>

namespace fs = std::filesystem;

REGISTER_PLUGIN( FullPathX, L"KX Full Path Memory-x16" );

FullPathX::FullPathX(IMpUnknown* host) : MpBase(host)
{
	initializePin( 0, Filename );
	//initializePin( 1, AbsPath );
	initializePin( 1, pinAbsPath );
	initializePin( 2, pinPathOut );
}

void FullPathX::onSetPins(void)  // one or more pins_ updated.  Check pin update flags to determin which ones.
{

    //if(pinAbsPath.isUpdated())AbsPath = pinAbsPath;

	if( Filename.isUpdated() || pinAbsPath.isUpdated())
	{

         //pinName = (StripPath((std::string) pinFilenameIn)).c_str();
         //pinPath = (StripFilename((std::string) pinFilenameIn)).c_str();

         //fs::path ap(StripFilename(pinAbsPath.getValue().c_str() ));
         fs::path ap(pinAbsPath.getValue().c_str());


            if(pinAbsPath.getValue().empty())
            {
            wchar_t fullFilename[MAX_PATH];
            getHost()->resolveFilename( Filename.getValue().c_str(), MAX_PATH, fullFilename );
            pinPathOut=fullFilename;
            //FileName=fullFilename;
            //std::wstring name=StripPath(Filename.getValue().c_str() );
            //std::wstring path = L"C:\Program Files\Common Files\VST3\Kx PolyM CSE\Seq-x16";
            //path.insert(path.length(),L"\\");
            //path.insert(path.length(),name);
            //pinPathOut=path.c_str();

            }
            else
            {
                 if(fs::is_directory(ap))
                 {
                    //wchar_t fullFilename[MAX_PATH];
                    //getHost()->resolveFilename( Filename.getValue().c_str(), MAX_PATH, fullFilename );
                    //std::wstring name=StripPath(fullFilename);

                    std::wstring name=StripPath(Filename.getValue().c_str() );
                    std::wstring path = pinAbsPath;
                    path.insert(path.length(),L"\\");
                    path.insert(path.length(),name);
                    pinPathOut=path.c_str();
                    //FileName=path.c_str();
                 }
            }
	}

}







