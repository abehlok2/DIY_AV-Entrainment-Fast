#include "Themes.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <map>

using namespace juce;

// A struct to hold both the JUCE ColourScheme and an associated CSS-like style
// string.
struct Theme {
  LookAndFeel_V4::ColourScheme scheme;
  String styleSheet;
};

//==============================================================================
// 1. Standard Dark Theme (New & Improved)
// A modern, professional dark theme focused on readability and comfort.
//==============================================================================
static LookAndFeel_V4::ColourScheme createDarkScheme() {
  return LookAndFeel_V4::ColourScheme{
      Colour::fromString("#1E1E1E"), // windowBackground
      Colour::fromString("#2D2D2D"), // widgetBackground
      Colour::fromString("#252525"), // menuBackground
      Colour::fromString("#4A4A4A"), // outline
      Colour::fromString("#E1E1E1"), // defaultText
      Colour::fromString("#2D2D2D"), // defaultFill
      Colour::fromString("#FFFFFF"), // highlightedText
      Colour::fromString("#007ACC"), // highlightedFill (Modern Accent Blue)
      Colour::fromString("#E1E1E1")  // menuText
  };
}

static const char *darkStyle = R"(
    QTreeWidget {
        color: #E1E1E1;
    }
    QLineEdit, QComboBox, QSlider {
        background-color: #252525;
        border: 1px solid #4A4A4A;
        color: #E1E1E1;
    }
)";

//==============================================================================
// 2. Dracula Theme
// A popular, modern theme with vibrant accent colors.
//==============================================================================
static LookAndFeel_V4::ColourScheme createDraculaScheme() {
  return LookAndFeel_V4::ColourScheme{
      Colour::fromString("#282a36"), // windowBackground
      Colour::fromString("#282a36"), // widgetBackground
      Colour::fromString("#44475a"), // menuBackground
      Colour::fromString("#6272a4"), // outline
      Colour::fromString("#f8f8f2"), // defaultText
      Colour::fromString("#44475a"), // defaultFill
      Colour::fromString("#f8f8f2"), // highlightedText
      Colour::fromString("#bd93f9"), // highlightedFill (Purple)
      Colour::fromString("#f8f8f2")  // menuText
  };
}

static const char *draculaStyle = R"(
    QLineEdit, QComboBox, QSlider {
        background-color: #44475a;
        border: 1px solid #6272a4;
        color: #f8f8f2;
    }
)";

//==============================================================================
// 3. Nord Theme
// An arctic, north-bluish color palette.
//==============================================================================
static LookAndFeel_V4::ColourScheme createNordScheme() {
  return LookAndFeel_V4::ColourScheme{
      Colour::fromString("#2E3440"), // windowBackground
      Colour::fromString("#3B4252"), // widgetBackground
      Colour::fromString("#2E3440"), // menuBackground
      Colour::fromString("#4C566A"), // outline
      Colour::fromString("#E5E9F0"), // defaultText
      Colour::fromString("#3B4252"), // defaultFill
      Colour::fromString(
          "#2E3440"), // highlightedText (for contrast on light highlight)
      Colour::fromString("#88C0D0"), // highlightedFill (Frost Blue)
      Colour::fromString("#E5E9F0")  // menuText
  };
}

static const char *nordStyle = R"(
    QLineEdit, QComboBox, QSlider {
        background-color: #434C5E;
        border: 1px solid #4C566A;
        color: #ECEFF4;
    }
)";

//==============================================================================
// 4. Solarized Dark Theme
// A theme with a precise, low-contrast palette for optimal readability.
//==============================================================================
static LookAndFeel_V4::ColourScheme createSolarizedDarkScheme() {
  return LookAndFeel_V4::ColourScheme{
      Colour::fromString("#002b36"), // windowBackground
      Colour::fromString("#073642"), // widgetBackground
      Colour::fromString("#002b36"), // menuBackground
      Colour::fromString("#586e75"), // outline
      Colour::fromString("#839496"), // defaultText
      Colour::fromString("#073642"), // defaultFill
      Colour::fromString(
          "#fdf6e3"), // highlightedText (light color for contrast)
      Colour::fromString("#268bd2"), // highlightedFill (Blue)
      Colour::fromString("#93a1a1")  // menuText
  };
}

static const char *solarizedStyle = R"(
    QLineEdit, QComboBox, QSlider {
        background-color: #002b36;
        border: 1px solid #586e75;
        color: #93a1a1;
    }
)";

//==============================================================================
// Theme Map
// The central map that holds all available themes.
//==============================================================================
static std::map<String, Theme> themes{
    {"Dark", {createDarkScheme(), darkStyle}},
    {"Dracula", {createDraculaScheme(), draculaStyle}},
    {"Nord", {createNordScheme(), nordStyle}},
    {"Solarized Dark", {createSolarizedDarkScheme(), solarizedStyle}}};

//==============================================================================
// applyTheme Function
// Finds and applies a theme by name to a LookAndFeel object.
//==============================================================================
void applyTheme(LookAndFeel_V4 &lf, const String &name) {
  auto it = themes.find(name);
  if (it == themes.end()) {
    // Fallback to the first theme if the requested one isn't found
    it = themes.begin();
  }
  lf.setColourScheme(it->second.scheme);

  // Note: The 'styleSheet' part is not used by default JUCE components.
  // It is included here in case you have custom integrations (e.g., with Qt)
  // that can make use of CSS-like styling.
}
