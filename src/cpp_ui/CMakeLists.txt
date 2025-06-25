cmake_minimum_required(VERSION 3.14)

project(DIY_AV_UI_CPP)

find_package(JUCE CONFIG REQUIRED)

juce_add_gui_app(diy_av_ui_cpp
    PRODUCT_NAME "DIY AV UI CPP"
    BUNDLE_ID com.example.diyavcpp.ui
    VERSION 0.1
    SOURCES
        main.cpp
        Simulator.cpp
        StepConfigPanel.cpp
        StepListPanel.cpp
        CollapsibleBox.cpp
        DefaultVoiceDialog.cpp
        FrequencyTesterDialog.cpp
        NoiseGeneratorDialog.cpp
        OverlayClipDialog.cpp
        PreferencesDialog.cpp
        SubliminalDialog.cpp
        VoiceEditorDialog.cpp
        Themes.cpp
        StepPreviewComponent.h
        StepPreviewComponent.cpp
)

juce_generate_juce_header(diy_av_ui_cpp)

# Link basic GUI modules for now
target_link_libraries(diy_av_ui_cpp
    PRIVATE
        juce::juce_gui_basics
        juce::juce_gui_extra
)
