Last update, version 1.11: 24/09/2026

Source code of 17 SEM modules, grouped in only one SEM.
Include my SampleOscillator3 prefab extracted of my Kx PolyMod plug-in! Inside the SEM, there is midi modules, tools modules and my customized SE modules for special features :  Scope V4 (with variable buffer), WaveRecorder3 (Kx Wavetracer sampling features), sampleoscillator3 (GUI SF2 loading to avoid DSP drops), new Polyblep oscillator with my 8 order IIR filters done with Winfilter (Chebychev, fc 20k, fs 384k rp 0.5dB, fs 192k rp 1dB, fs 96k rp 1dB) to add oversampling features on this one.


1/10/2024:
- Filters added, main sources available based on various "old" code found on archives from musicdsp.org but reworked a lot, fine tuning of cut off frequency, control of resonance level, oversampling, etc... Only the main filters are inside the source files (Moog, MoogLadder, SVF). 
All SE Modular ME filters are compiled inside the SEM (Moog, MoogLadder, SVF, CS, VCS, various filters of my freeware plugins released since many years.

10/11/2024: Update of Polyblep oscillator: 
- "ResetF();"  added in "int32_t MP_STDCALL open()", Shared lookup tables used now and same sleep process like OscillatorHD.

20/12/2024: Update of CK Open Filters: 
- onSetPins(void) modified,"ResetVoice();" added when "res_mode.isUpdated()". Improvement of MgLnr and MgL filters stability (cut off limited to 10.4 volts instead of 10.5 volts).

02/02/2025: Update of Polyblep oscillator:
- Shared lookup tables not used, regression, possible Cubase 12 initialisation bug with shared lookup tables (Yamaha ASIO drivers + Intel CPU), observed on a complex session with a lot of SE PolyMod KX plugin instances (built with SE 1.4709), like my usual tracks of course.

17/02/2025: Update of filters:
- More robust initialisation of shared lookup tables (mutex) and better implementation of voice reset.

31/10/2025: Update of SoundFontOscillator3:
- Now it supports independent loop points for stereo samples -> last Jeff's code
- A crossfade has been added to the end of the loop to prevent clicks if the loop end point is too close to the end of the sample data (interpolation related bug) -> last DAM's code, I adapted the DAM's fix to the last Jeff's code.
https://groups.io/g/synthedit/topic/sf2_player_sample_oscillator/114888390

01/08/2026:
- x16 Path Memory module added, I built this module to force my plugins to use an absolute path in place of the relative path saved in my old presets, so even if the path saved in preset will not change if the user do nothing, with this feature I can retrieve all x16 files, just I must copy them inside the X16 folder of the plugin. So I can use it also for the waveforms files. The user can choose between a relative path or an absolute path, nice for SE 1.5 or SE 1.6.
- SampleOscillator3, ID error during program change or preset loading if the SF2 filename does not change when the plugin GUI is open.
To solve this bug I added an input Int GUI pin, so if the preset name is changed even the SF2 name is not changed, the SF2 is loaded with the GUI thread. I added also a random generator in some processes of my KX_INIT_PrgChange module for do this.
In my main plugins built with SE 1.4 each preset generates an ID random number, I use it to detect when the preset is changed.

24/09/2026: all sources on of CK_Tools SEM on GitHub:
- All CK AllFilters module added, total of 39 filters, all tuned with love ^^, include my Synthi filters!
- CK-VoiceControl DSP/GUI module added, I created this module to replace the old MidiToCv and the boring PolyphonyControl module. The DSP pins control the hostconnect GUI pins by using a simple BlobToGui parameter to the GUI.
Possibility to have the settings like old MidiToCv module: Portamento amount, Glide Rate and Auto Glide, by this way it will be more easy to replace the old MidiToCv for using the poly glide feature and to avoid the hassle to use out GUI pins of PolyphonyControl module.
Note: after lot of tests my MidiToCv project is aborted, replaced by the last CK-VoiceControl, this one is really better, stable and simple to use! 
- Sem-version.rc file added.
- Multi-Platform releases (Windows, Apple, Linux) built on GitHub, repository base from:
https://github.com/JeffMcClintock/synthedit-module-example

But I did another one here to compile grouped SEM modules like I do usually:
https://github.com/ClaudiaK-SEM/sem-modules

http://kx77free.free.fr
Claudia Kalensky (KX77FREE), 24/09/2026


# CK_tools - Only one Plugin ^^ for you.

A single .sem plugin containing multiple tool modules.

## Structure

```
CK_tools/
├── CMakeLists.txt        # Single build config, compiles all modules into one sem
├── CK_tools.xml          # Shared XML with all module definitions
├── CK_Tool1/             # Module 1 source folder
│   ├── CK_Tool1.h
│   ├── CK_Tool1.cpp
│   ├── CK_Tool1Gui.h
│   └── CK_Tool1Gui.cpp
├── CK_Tool2/             # Module 2 source folder
│   ├── CK_Tool2.h
│   ├── CK_Tool2.cpp
│   ├── CK_Tool2Gui.h
│   └── CK_Tool2Gui.cpp
└── README.md
```

## Building

Compiles all modules into a **single CK_tools.sem** file:

```bash
cd build
cmake --build . --config Release --target CK_tools
```

## Adding New Modules

### Step 1: Create module folder
```bash
mkdir modules/CK_tools/CK_Tool3
```

### Step 2: Create source files
Create the four files in `CK_Tool3/`:
- `CK_Tool3.h` - DSP header
- `CK_Tool3.cpp` - DSP implementation
- `CK_Tool3Gui.h` - GUI header
- `CK_Tool3Gui.cpp` - GUI implementation

Use existing modules (CK_Tool1, CK_Tool2) as templates.

### Step 3: Update CMakeLists.txt
Add your module source files to the `SOURCE_FILES` list:

```cmake
SOURCE_FILES
    # ... existing modules ...
    CK_Tool3/CK_Tool3.cpp
    CK_Tool3/CK_Tool3.h
    CK_Tool3/CK_Tool3Gui.cpp
    CK_Tool3/CK_Tool3Gui.h
```

### Step 4: Update CK_tools.xml
Add a new `<Plugin>` entry:

```xml
<Plugin id="prodID_CK_Tool3" name="CK Tool3" Vendor="CK" Description="Third tool module" Version="1.0">
  <Audio>
    <Pin name="Audio In" datatype="float" direction="in" />
    <Pin name="Audio Out" datatype="float" direction="out" />
  </Audio>
  <Parameters>
    <Pin name="Param 1" datatype="float" default="1.0" />
  </Parameters>
</Plugin>
```

### Step 5: Rebuild
```bash
cmake --build . --config Release --target CK_tools
```

## Key Points

- **One .sem file**: All modules compile into `CK_tools.sem`
- **Organized source**: Each module's code in its own folder
- **Shared XML**: Single XML file defines all modules
- **Easy to expand**: Just add folders and update CMakeLists.txt + XML
- **Single compilation**: No separate module compilation needed
