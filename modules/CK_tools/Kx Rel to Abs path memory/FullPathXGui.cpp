// prevent MS CPP - 'swprintf' was declared deprecated warning
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning(disable : 4996)

#include "FullPathXGui.h"
#include "../shared/unicode_conversion.h"
#include "../shared/it_enum_list.h"
#include "../shared/string_utilities.h"
#include "../se_sdk3/MpString.h"
#include <filesystem>

namespace fs = std::filesystem;

//using namespace std;

REGISTER_GUI_PLUGIN( FullPathXGui, L"KX Full Path Memory-x16" );

#define INIT_PINB( pinId, name ) initializePin( pinId, pin##name, static_cast<MpGuiBaseMemberPtr>( &FullPathXGui::on##name##Changed ) );

FullPathXGui::FullPathXGui(IMpUnknown* host) : MpGuiBase(host)
{
	INIT_PINB(0, FilenameIn);//to dsp
	INIT_PINB(1, Name);//out
	INIT_PINB(2, Filename);//out
	INIT_PINB(3, Path);//out
	INIT_PINB(4, FullPath);//out
	//INIT_PINB(5, AbsPath);
}
/*int32_t FullPathXGui::initialize()//init
{
    update();
	return MpGuiBase::initialize();
}*/
void FullPathXGui::onFilenameInChanged()//real filename in
{
        pinFilename=pinFilenameIn;
        update();
}
/*void FullPathXGui::onAbsPathChanged()//gui value out
{
        update();
}*/
void FullPathXGui::onFilenameChanged()//real filename in
{
        pinFilenameIn=pinFilename;
        update();
}
void FullPathXGui::update()//gui path
{


         pinName = (StripPath((std::string) pinFilenameIn)).c_str();
         pinPath = (StripFilename((std::string) pinFilenameIn)).c_str();
         pinFullPath=pinFilenameIn;

        /*fs::path p(pinPath.getValue().c_str());


         if(fs::is_directory(p))
         {

            pinPath=pinAbsPath;


            //wchar_t fullFilename[MAX_PATH];
            //getHost()->resolveFilename( pinFilenameIn.getValue().c_str(), MAX_PATH, fullFilename );
            //pinName=StripPath(fullFilename);

            pinName =(StripPath((std::string) pinFilenameIn)).c_str();

            std::wstring path = pinAbsPath;
            std::wstring name = pinName;
            path.insert(path.length(),L"\\");
            path.insert(path.length(),name);

            pinFullPath=path.c_str();
            pinFilenameIn=path.c_str();
            //pinFilename=pinFilenameIn;


         }
         else
         {
            wchar_t fullFilename[MAX_PATH];
            getHost()->resolveFilename( pinFilenameIn.getValue().c_str(), MAX_PATH, fullFilename );

            pinFullPath=fullFilename;
            pinFilenameIn=fullFilename;
            //pinFilename=pinFilenameIn;

            pinName = (StripPath((std::string) pinFullPath)).c_str();
            pinPath = (StripFilename((std::string) pinFullPath)).c_str();
         }*/

}
void FullPathXGui::onNameChanged()
{
}
void FullPathXGui::onPathChanged()//gui value out
{
}
void FullPathXGui::onFullPathChanged()//gui value out
{
}

