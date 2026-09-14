cmake_minimum_required(VERSION 3.19)

project(CK_tools)

build_gmpi_plugin(
    PROJECT_NAME ${PROJECT_NAME}
    HAS_DSP
    HAS_GUI
    SOURCE_FILES
        # CK Open Filters
        CK-Open-Filters/Filters.cpp
        CK-Open-Filters/Filters.h
        # Converter
        Converter/KxConvGui.cpp
        Converter/KxConvGui.h
)
