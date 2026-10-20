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
	initializePin( 1, pinAbsPath );
	initializePin( 2, pinPathOut );
}
void FullPathX::onSetPins(void)
{
    if (Filename.isUpdated() || pinAbsPath.isUpdated())
    {
        fs::path ap(pinAbsPath.getValue());
        std::error_code ec;

        if (pinAbsPath.getValue().empty())
        {
            std::wstring filename = Filename.getValue();
            wchar_t fullFilename[500];
            getHost()->resolveFilename(filename.c_str(), sizeof(fullFilename) / sizeof(fullFilename[0]), fullFilename);
            pinPathOut = fullFilename;
        }
        else if (fs::is_directory(ap, ec) && !ec)
        {
            fs::path base = pinAbsPath.getValue();
            fs::path filename = StripPath(Filename.getValue().c_str());
            fs::path finalPath = base / filename;
            pinPathOut = finalPath.wstring();
        }
        else
        {
            std::wstring filename = Filename.getValue();
            wchar_t fullFilename[500];
            getHost()->resolveFilename(filename.c_str(),sizeof(fullFilename) / sizeof(fullFilename[0]),fullFilename);
            pinPathOut = fullFilename;
        }
    }
}




