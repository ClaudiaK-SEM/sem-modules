# CK_tools - Monolithic Plugin

A single .sem plugin containing multiple tool modules.

## Structure

```
CK_tools/
├── CMakeLists.txt           # Single build config - compiles all modules into one .sem
├── CK_tools.xml             # Shared XML with all module definitions
├── CK_Tool1/                # Module 1 source folder
│   ├── CK_Tool1.h
│   ├── CK_Tool1.cpp
│   ├── CK_Tool1Gui.h
│   └── CK_Tool1Gui.cpp
├── CK_Tool2/                # Module 2 source folder
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
