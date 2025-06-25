#include <juce_gui_basics/juce_gui_basics.h>
#include <map>

using namespace juce;

struct Theme
{
    LookAndFeel_V4::ColourScheme scheme;
    String styleSheet;
};

static LookAndFeel_V4::ColourScheme createDarkScheme()
{
    LookAndFeel_V4::ColourScheme s;
    using UI = LookAndFeel_V4::ColourScheme::UIColour;

    s.setUIColour(UI::windowBackground, Colour::fromRGB(53, 53, 53));
    s.setUIColour(UI::widgetBackground, Colour::fromRGB(53, 53, 53));
    s.setUIColour(UI::menuBackground,   Colour::fromRGB(25, 25, 25));
    s.setUIColour(UI::outline,          Colour::fromRGB(80, 80, 80));
    s.setUIColour(UI::defaultText,      Colour::fromRGB(255, 255, 255));
    s.setUIColour(UI::defaultFill,      Colour::fromRGB(53, 53, 53));
    s.setUIColour(UI::highlightedText,  Colour::fromRGB(0, 0, 0));
    s.setUIColour(UI::highlightedFill,  Colour::fromRGB(42, 130, 218));
    s.setUIColour(UI::menuText,         Colour::fromRGB(255, 255, 255));
    return s;
}

static const char* darkStyle = R"(
QTreeWidget {
    color: #ffffff;
}
)";

static LookAndFeel_V4::ColourScheme createGreenScheme()
{
    using UI = LookAndFeel_V4::ColourScheme::UIColour;

    LookAndFeel_V4::ColourScheme s;
    s.setUIColour(UI::windowBackground, Colour::fromRGB(0x0a, 0x0a, 0x0a));
    s.setUIColour(UI::widgetBackground, Colour::fromRGB(0x1a, 0x1a, 0x1a));
    s.setUIColour(UI::menuBackground,   Colour::fromRGB(0x15, 0x20, 0x15));
    s.setUIColour(UI::outline,          Colour::fromRGBA(0x00, 0xff, 0x88, 0x60));
    s.setUIColour(UI::defaultText,      Colour::fromRGB(0x00, 0xff, 0xaa));
    s.setUIColour(UI::defaultFill,      Colour::fromRGBA(0x00, 0x88, 0x44, 0x60));
    s.setUIColour(UI::highlightedText,  Colour::fromRGB(0xff, 0xff, 0xff));
    s.setUIColour(UI::highlightedFill,  Colour::fromRGBA(0x00, 0xff, 0x88, 0xaa));
    s.setUIColour(UI::menuText,         Colour::fromRGB(0x00, 0xff, 0xcc));
    return s;
}

static const char* greenStyle = R"(
/* Base Widget Styling */
QWidget {
    font-size: 10pt;
    background-color: #0a0a0a;
    color: #00ffaa;
    font-family: 'Consolas', 'Courier New', monospace;
}

/* Group Boxes */
QGroupBox {
    background-color: #1a1a1a;
    border: 1px solid rgba(0, 255, 136, 0.4);
    border-radius: 4px;
    margin-top: 8px;
    padding-top: 8px;
}

QGroupBox::title {
    color: #00ffaa;
    subcontrol-origin: margin;
    left: 10px;
    padding: 0 3px 0 3px;
    background-color: #1a1a1a;
}

/* Push Buttons */
QPushButton {
    background-color: rgba(0, 255, 136, 0.25);
    border: 1px solid #00ff88;
    color: #00ffaa;
    padding: 4px 12px;
    border-radius: 4px;
    font-weight: bold;
}

QPushButton:hover {
    background-color: rgba(0, 255, 136, 0.4);
    border: 1px solid #00ffcc;
    box-shadow: 0 0 10px rgba(0, 255, 136, 0.6);
}

QPushButton:pressed {
    background-color: rgba(0, 255, 136, 0.6);
}

QPushButton:disabled {
    background-color: rgba(0, 136, 68, 0.2);
    border: 1px solid rgba(0, 255, 136, 0.2);
    color: rgba(0, 255, 136, 0.5);
}

/* Column Headers */
QHeaderView::section {
    background-color: #000000;
    color: #00ffaa;
}

QLineEdit, QComboBox, QSlider {
    background-color: #202020;
    border: 1px solid #555555;
    color: #ffffff;     /* use white text */
}
)";

static LookAndFeel_V4::ColourScheme createLightBlueScheme()
{
    using UI = LookAndFeel_V4::ColourScheme::UIColour;

    LookAndFeel_V4::ColourScheme s;
    s.setUIColour(UI::windowBackground, Colour::fromRGB(240, 248, 255));
    s.setUIColour(UI::widgetBackground, Colour::fromRGB(255, 255, 255));
    s.setUIColour(UI::menuBackground,   Colour::fromRGB(230, 240, 250));
    s.setUIColour(UI::outline,          Colour::fromRGB(160, 160, 160));
    s.setUIColour(UI::defaultText,      Colour::fromRGB(0, 0, 0));
    s.setUIColour(UI::defaultFill,      Colour::fromRGB(225, 238, 255));
    s.setUIColour(UI::highlightedText,  Colour::fromRGB(255, 255, 255));
    s.setUIColour(UI::highlightedFill,  Colour::fromRGB(0, 120, 215));
    s.setUIColour(UI::menuText,         Colour::fromRGB(0, 0, 0));
    return s;
}

static const char* lightBlueStyle = R"(
QTreeWidget {
    color: #000000;
}
QLineEdit, QComboBox, QSlider {
    background-color: #ffffff;
    border: 1px solid #a0a0a0;
    color: #000000;
}
)";

static LookAndFeel_V4::ColourScheme createMaterialScheme()
{
    using UI = LookAndFeel_V4::ColourScheme::UIColour;

    LookAndFeel_V4::ColourScheme s;
    s.setUIColour(UI::windowBackground, Colour::fromRGB(250, 250, 250));
    s.setUIColour(UI::widgetBackground, Colour::fromRGB(255, 255, 255));
    s.setUIColour(UI::menuBackground,   Colour::fromRGB(245, 245, 245));
    s.setUIColour(UI::outline,          Colour::fromRGB(208, 208, 208));
    s.setUIColour(UI::defaultText,      Colour::fromRGB(33, 33, 33));
    s.setUIColour(UI::defaultFill,      Colour::fromRGB(238, 238, 238));
    s.setUIColour(UI::highlightedText,  Colour::fromRGB(255, 255, 255));
    s.setUIColour(UI::highlightedFill,  Colour::fromRGB(255, 87, 34));
    s.setUIColour(UI::menuText,         Colour::fromRGB(33, 33, 33));
    return s;
}

static const char* materialStyle = R"(
QTreeWidget {
    color: #212121;
}
QGroupBox {
    background-color: #ffffff;
    border: 1px solid #d0d0d0;
    border-radius: 8px;
    margin-top: 12px;
    padding-top: 8px;
    padding-left: 8px;
    padding-right: 8px;
    box-shadow: 0 2px 6px rgba(0, 0, 0, 0.2);
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 14px;
    padding: 0 4px 0 4px;
}
QPushButton {
    background-color: #009688;
    border: none;
    color: white;
    padding: 6px 16px;
    border-radius: 4px;
}
QPushButton:hover {
    background-color: #26a69a;
}
QPushButton:pressed {
    background-color: #00796b;
}
QLineEdit, QComboBox, QSlider {
    background-color: #ffffff;
    border: 1px solid #bdbdbd;
    color: #212121;
    border-radius: 4px;
}
)";

static std::map<String, Theme> themes{
    { "Dark",     { createDarkScheme(),     darkStyle } },
    { "Green",    { createGreenScheme(),    greenStyle } },
    { "light-blue", { createLightBlueScheme(), lightBlueStyle } },
    { "Material", { createMaterialScheme(), materialStyle } }
};

void applyTheme (LookAndFeel_V4& lf, const String& name)
{
    auto it = themes.find(name);
    if (it == themes.end())
        return;
    lf.setColourScheme(it->second.scheme);
}

