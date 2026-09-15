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
    
# Init-PrgCh-Midi/
Init-PrgCh-Midi/PrgCh.cpp
Init-PrgCh-Midi/PrgCh.h
Init-PrgCh-Midi/PrgChGui.cpp
Init-PrgCh-Midi/PrgChGui.h
     
# MidiAllOff/
MidiAllOff/MidiAllOff.cpp
MidiAllOff/MidiAllOff.h
 
# MidiFilterAll/
MidiFilterAll/Kxmidifilterall.cpp
MidiFilterAll/Kxmidifilterall.h
 
# MidiOnOff/
MidiOnOff/MidiOnOff.cpp
MidiOnOff/MidiOnOff.h
     
# MidiSwitch2Ch/
MidiSwitch2Ch/KxMidiSwitch2Ch.cpp
MidiSwitch2Ch/KxMidiSwitch2Ch.h

# Polyblep-Oscillator/
Polyblep-Oscillator/Oscillator.cpp
     
# PolyphonyControl/
PolyphonyControl/v1_PolyphonyControlGui.cpp
PolyphonyControl/v1_PolyphonyControlGui.h
PolyphonyControl/v6_PolyphonyControlGui.cpp
PolyphonyControl/v6_PolyphonyControlGui.h
     
# Rel2Abs-PM/
Rel2Abs-PM/FullPathX.cpp
Rel2Abs-PM/FullPathX.h
Rel2Abs-PM/FullPathXGui.cpp
Rel2Abs-PM/FullPathXGui.h

# SampleOscillator3/
SampleOscillator3/CSoundFont.cpp
SampleOscillator3/CSoundFont.h
SampleOscillator3/mfc_emulation.h
# SampleOscillator3/protected_ifstream.cpp
# SampleOscillator3/protected_ifstream.h
SampleOscillator3/RiffFile2.cpp
SampleOscillator3/RiffFile2.h
SampleOscillator3/SampleLoader3.cpp
SampleOscillator3/SampleLoader3.h
SampleOscillator3/SampleLoader3Gui.cpp
SampleOscillator3/SampleLoader3Gui.h
SampleOscillator3/SampleManager.cpp
SampleOscillator3/SampleManager.h
SampleOscillator3/SampleOscillator3.cpp
SampleOscillator3/SampleOscillator3.h
SampleOscillator3/soundfont_user.cpp
SampleOscillator3/soundfont_user.h
      
# Scope-V4/
Scope-V4/Scope3.cpp
Scope-V4/Scope3.h
Scope-V4/Scope3Gui.cpp
Scope-V4/Scope3Gui.h
       
# Voice-test-Rnd/
Voice-test-Rnd/
Voice-test-Rnd/
      
# VoltConverter/
VoltConverter/KxVoltToHz.cpp
VoltConverter/KxVoltToHz.h
VoltConverter/KxVoltToHzGui.cpp
VoltConverter/KxVoltToHzGui.h

# WaveRecorder3/
WaveRecorder3/WaveRecorder3.cpp
WaveRecorder3/WaveRecorder3.h

)
