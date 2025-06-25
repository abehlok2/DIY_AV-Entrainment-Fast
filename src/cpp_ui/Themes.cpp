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
    s.setUIColour(LookAndFeel_V4::ColourScheme::windowBackground, Colour(53,53,53));
    s.setUIColour(LookAndFeel_V4::ColourScheme::widgetBackground, Colour(53,53,53));
    s.setUIColour(LookAndFeel_V4::ColourScheme::menuBackground,   Colour(25,25,25));
    s.setUIColour(LookAndFeel_V4::ColourScheme::outline,          Colour(80,80,80));
    s.setUIColour(LookAndFeel_V4::ColourScheme::defaultText,      Colour(255,255,255));
    s.setUIColour(LookAndFeel_V4::ColourScheme::defaultFill,      Colour(53,53,53));
    s.setUIColour(LookAndFeel_V4::ColourScheme::highlightedText,  Colour(0,0,0));
    s.setUIColour(LookAndFeel_V4::ColourScheme::highlightedFill,  Colour(42,130,218));
    s.setUIColour(LookAndFeel_V4::ColourScheme::menuText,         Colour(255,255,255));
    return s;
}

static const char* darkStyle = R"(
QTreeWidget {
    color: #ffffff;
}
)";

static LookAndFeel_V4::ColourScheme createGreenScheme()
{
    LookAndFeel_V4::ColourScheme s;
    s.setUIColour(LookAndFeel_V4::ColourScheme::windowBackground, Colour(0x0a,0x0a,0x0a));
    s.setUIColour(LookAndFeel_V4::ColourScheme::widgetBackground, Colour(0x1a,0x1a,0x1a));
    s.setUIColour(LookAndFeel_V4::ColourScheme::menuBackground,   Colour(0x15,0x20,0x15));
    s.setUIColour(LookAndFeel_V4::ColourScheme::outline,          Colour(0x00,0xff,0x88,0x60));
    s.setUIColour(LookAndFeel_V4::ColourScheme::defaultText,      Colour(0x00,0xff,0xaa));
    s.setUIColour(LookAndFeel_V4::ColourScheme::defaultFill,      Colour(0x00,0x88,0x44,0x60));
    s.setUIColour(LookAndFeel_V4::ColourScheme::highlightedText,  Colour(0xff,0xff,0xff));
    s.setUIColour(LookAndFeel_V4::ColourScheme::highlightedFill,  Colour(0x00,0xff,0x88,0xaa));
    s.setUIColour(LookAndFeel_V4::ColourScheme::menuText,         Colour(0x00,0xff,0xcc));
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
    LookAndFeel_V4::ColourScheme s;
    s.setUIColour(LookAndFeel_V4::ColourScheme::windowBackground, Colour(240,248,255));
    s.setUIColour(LookAndFeel_V4::ColourScheme::widgetBackground, Colour(255,255,255));
    s.setUIColour(LookAndFeel_V4::ColourScheme::menuBackground,   Colour(230,240,250));
    s.setUIColour(LookAndFeel_V4::ColourScheme::outline,          Colour(160,160,160));
    s.setUIColour(LookAndFeel_V4::ColourScheme::defaultText,      Colour(0,0,0));
    s.setUIColour(LookAndFeel_V4::ColourScheme::defaultFill,      Colour(225,238,255));
    s.setUIColour(LookAndFeel_V4::ColourScheme::highlightedText,  Colour(255,255,255));
    s.setUIColour(LookAndFeel_V4::ColourScheme::highlightedFill,  Colour(0,120,215));
    s.setUIColour(LookAndFeel_V4::ColourScheme::menuText,         Colour(0,0,0));
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
    LookAndFeel_V4::ColourScheme s;
    s.setUIColour(LookAndFeel_V4::ColourScheme::windowBackground, Colour(250,250,250));
    s.setUIColour(LookAndFeel_V4::ColourScheme::widgetBackground, Colour(255,255,255));
    s.setUIColour(LookAndFeel_V4::ColourScheme::menuBackground,   Colour(245,245,245));
    s.setUIColour(LookAndFeel_V4::ColourScheme::outline,          Colour(208,208,208));
    s.setUIColour(LookAndFeel_V4::ColourScheme::defaultText,      Colour(33,33,33));
    s.setUIColour(LookAndFeel_V4::ColourScheme::defaultFill,      Colour(238,238,238));
    s.setUIColour(LookAndFeel_V4::ColourScheme::highlightedText,  Colour(255,255,255));
    s.setUIColour(LookAndFeel_V4::ColourScheme::highlightedFill,  Colour(255,87,34));
    s.setUIColour(LookAndFeel_V4::ColourScheme::menuText,         Colour(33,33,33));
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

