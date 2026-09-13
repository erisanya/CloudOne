cmake_minimum_required(VERSION 3.22)

project(CloudOne VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- JUCE ---------------------------------------------------------------
# Option A: point this at a local JUCE checkout you already have:
#   cmake -B build -DJUCE_DIR=/path/to/JUCE
# Option B: leave JUCE_DIR empty and CMake will fetch JUCE from GitHub
#   (requires network access on first configure).
set(JUCE_DIR "" CACHE PATH "Path to a local JUCE checkout (optional)")

if(JUCE_DIR)
    add_subdirectory(${JUCE_DIR} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        7.0.12
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# --- Plugin target --------------------------------------------------------
juce_add_plugin(CloudOne
    COMPANY_NAME                "erisa"
    PRODUCT_NAME                "Cloud One"
    PLUGIN_MANUFACTURER_CODE    Clou
    PLUGIN_CODE                 Cld1
    FORMATS                     VST3 Standalone
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     TRUE
    VST3_CATEGORIES             Fx EQ
)

target_sources(CloudOne PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(CloudOne PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(CloudOne
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# --- End-user installer (.exe) -------------------------------------------
# Requires NSIS installed (https://nsis.sourceforge.io/Download), free.
# After building Release, generate the installer with:
#   cd build
#   cpack
# This produces "Cloud One-1.0.0-win64.exe" — a normal installer with a
# Next/Next/Install wizard and a page where the user can pick the install
# folder. It does NOT require Visual Studio, CMake, or JUCE on their end.
if(WIN32)
    install(DIRECTORY "${CMAKE_BINARY_DIR}/CloudOne_artefacts/Release/VST3/Cloud One.vst3"
            DESTINATION ".")

    set(CPACK_PACKAGE_NAME "Cloud One")
    set(CPACK_PACKAGE_VENDOR "erisa")
    set(CPACK_PACKAGE_VERSION "1.0.0")
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "Cloud One VST3")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "VST3")
    # Default install location — the standard VST3 folder every DAW scans.
    # NSIS still shows a "choose folder" page; users only need to change it
    # if they use a nonstandard VST3 path.
    set(CPACK_NSIS_INSTALL_ROOT "C:\\\\Program Files\\\\Common Files")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    include(CPack)
endif()
