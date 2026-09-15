cmake_minimum_required(VERSION 3.19)

project(CK_MidiToCv)

build_gmpi_plugin(
    PROJECT_NAME ${PROJECT_NAME}
    HAS_DSP
    HAS_GUI
    SOURCE_FILES 
        ${se_sdk_folder}/smart_audio_pin.h
        ${se_sdk_folder}/smart_audio_pin.cpp        
        CK_MidiToCv.cpp
        CK_MidiToCv.h
        CK_MidiToCvGui.cpp
        CK_MidiToCvGui.h
)
