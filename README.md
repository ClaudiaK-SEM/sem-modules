# SEM Modules Monorepo

A centralized repository for managing multiple SynthEdit modules with organized SDK paths.

## Structure

```
sem-modules/
├── cmake/
│   └── plugin_helper.cmake       # Shared CMake build helpers
├── modules/
│   ├── CK_MidiToCv/              # Module v1.0
│   │   └── CMakeLists.txt
│   └── CK_MidiToCv-1.5/          # Module v1.5
│       └── CMakeLists.txt
├── CMakeLists.txt                # Root with SDK configuration
└── README.md                      # Documentation
```

## SDK Organization

The monorepo uses **CMake FetchContent** to automatically download and organize the SynthEdit SDK:

- **SDK Source**: `https://github.com/JeffMcClintock/SynthEdit_SDK`
- **Shared Paths**:
  - `se_sdk_folder`: Contains GMPI SDK files (headers, common, audio, gui)
  - `se_shared_folder`: Contains shared utility files
  - These paths are defined in the root `CMakeLists.txt` and available to all modules

## Building

### Prerequisites
- CMake 3.19+
- C++ 17 compiler
- Git (for FetchContent)

### Build All Modules

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Build Specific Module

```bash
cmake --build . --config Release --target CK_MidiToCv
```

## Adding a New Module

1. Create a new folder under `modules/`:
   ```bash
   mkdir modules/MyNewModule
   ```

2. Create `modules/MyNewModule/CMakeLists.txt`:
   ```cmake
   cmake_minimum_required(VERSION 3.19)
   project(MyNewModule)
   
   build_gmpi_plugin(
       PROJECT_NAME ${PROJECT_NAME}
       HAS_DSP
       HAS_GUI
       SOURCE_FILES 
           MyNewModule.cpp
           MyNewModuleGui.cpp
   )
   ```

3. Add source files (`.cpp`, `.h`, `.xml`)

4. Update root `CMakeLists.txt`:
   ```cmake
   add_subdirectory(modules/MyNewModule)
   ```

5. Rebuild:
   ```bash
   cmake --build . --config Release
   ```

## SDK Path Variables

All modules automatically have access to:
- `${se_sdk_folder}` - SynthEdit SDK files
- `${se_shared_folder}` - Shared utility files
- `${sdk_folder}` - General SDK reference

Use these in your module's `CMakeLists.txt` to reference SDK headers and source files.

## Platform Support

- **Windows**: MSVC with optimization flags
- **macOS**: x86_64 and arm64 (Apple Silicon) support
- **Linux**: Supported via CMake

## Local Development

For local builds, set `SE_LOCAL_BUILD=TRUE` to automatically copy built modules to:
- Windows: `C:\Program Files\Common Files\SynthEdit\modules\community_modules`
- macOS/Linux: Requires manual configuration
