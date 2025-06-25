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
    return LookAndFeel_V4::ColourScheme{
        Colour::fromRGB(53, 53, 53),  // windowBackground
        Colour::fromRGB(53, 53, 53),  // widgetBackground
        Colour::fromRGB(25, 25, 25),  // menuBackground
        Colour::fromRGB(80, 80, 80),  // outline
        Colour::fromRGB(255, 255, 255), // defaultText
        Colour::fromRGB(53, 53, 53),  // defaultFill
        Colour::fromRGB(0, 0, 0),     // highlightedText
        Colour::fromRGB(42, 130, 218),// highlightedFill
        Colour::fromRGB(255, 255, 255) // menuText
    };
}

static const char* darkStyle = R"(
QTreeWidget {
    color: #ffffff;
}
)";

static LookAndFeel_V4::ColourScheme createGreenScheme()
{
    return LookAndFeel_V4::ColourScheme{
        Colour::fromRGB(0x0a, 0x0a, 0x0a),             // windowBackground
        Colour::fromRGB(0x1a, 0x1a, 0x1a),             // widgetBackground
        Colour::fromRGB(0x15, 0x20, 0x15),             // menuBackground
        Colour::fromRGBA(0x00, 0xff, 0x88, 0x60),      // outline
        Colour::fromRGB(0x00, 0xff, 0xaa),             // defaultText
        Colour::fromRGBA(0x00, 0x88, 0x44, 0x60),      // defaultFill
        Colour::fromRGB(0xff, 0xff, 0xff),             // highlightedText
        Colour::fromRGBA(0x00, 0xff, 0x88, 0xaa),      // highlightedFill
        Colour::fromRGB(0x00, 0xff, 0xcc)              // menuText
    };
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
    return LookAndFeel_V4::ColourScheme{
        Colour::fromRGB(240, 248, 255), // windowBackground
        Colour::fromRGB(255, 255, 255), // widgetBackground
        Colour::fromRGB(230, 240, 250), // menuBackground
        Colour::fromRGB(160, 160, 160), // outline
        Colour::fromRGB(0, 0, 0),       // defaultText
        Colour::fromRGB(225, 238, 255), // defaultFill
        Colour::fromRGB(255, 255, 255), // highlightedText
        Colour::fromRGB(0, 120, 215),   // highlightedFill
        Colour::fromRGB(0, 0, 0)        // menuText
    };
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
    return LookAndFeel_V4::ColourScheme{
        Colour::fromRGB(250, 250, 250), // windowBackground
        Colour::fromRGB(255, 255, 255), // widgetBackground
        Colour::fromRGB(245, 245, 245), // menuBackground
        Colour::fromRGB(208, 208, 208), // outline
        Colour::fromRGB(33, 33, 33),    // defaultText
        Colour::fromRGB(238, 238, 238), // defaultFill
        Colour::fromRGB(255, 255, 255), // highlightedText
        Colour::fromRGB(255, 87, 34),   // highlightedFill
        Colour::fromRGB(33, 33, 33)     // menuText
    };
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

