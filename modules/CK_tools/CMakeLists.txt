cmake_minimum_required(VERSION 3.19)

project(CK_tools)

build_gmpi_plugin(
    PROJECT_NAME ${PROJECT_NAME}
    HAS_DSP
    HAS_GUI
    SOURCE_FILES
        # Module 1: Tool1
        CK_Tool1/CK_Tool1.cpp
        CK_Tool1/CK_Tool1.h
        CK_Tool1/CK_Tool1Gui.cpp
        CK_Tool1/CK_Tool1Gui.h
        
        # Module 2: Tool2
        CK_Tool2/CK_Tool2.cpp
        CK_Tool2/CK_Tool2.h
        CK_Tool2/CK_Tool2Gui.cpp
        CK_Tool2/CK_Tool2Gui.h
        
        # Add more modules here as needed
        # CK_Tool3/CK_Tool3.cpp
        # CK_Tool3/CK_Tool3.h
        # CK_Tool3/CK_Tool3Gui.cpp
        # CK_Tool3/CK_Tool3Gui.h
)
