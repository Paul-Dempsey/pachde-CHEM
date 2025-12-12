#include "theme.hpp"

namespace theme {

namespace theme_name {
    const char * Ui = "~u";
    const char * PreferDark = "~d";
}

std::string get_actual_theme(const std::string &theme)
{
    if (theme.empty()) return "Dark";

    if (is_special_theme(theme)) {
        assert(theme.size() == 2);
        switch (theme[1]) {
            case 'u':
                if (::rack::settings::uiTheme == "dark")   return "Dark";
                if (::rack::settings::uiTheme == "light")  return "Light";
                if (::rack::settings::uiTheme == "hcdark") return "High Contrast";
                return "Dark";

            case 'd':
                if (::rack::settings::preferDarkPanels) {
                    if (::rack::settings::uiTheme == "hcdark") return "High Contrast";
                    return "Dark";
                }
                return "Light";

            default:
                assert(false);
                return "Dark";
        }
    }
    return theme;
}

RackUiTheme get_rack_ui_theme()
{
    //if (::rack::settings::uiTheme == "dark")   return RackUiTheme::Dark;
    if (::rack::settings::uiTheme == "light")  return RackUiTheme::Light;
    if (::rack::settings::uiTheme == "hcdark") return RackUiTheme::HighContrast;
    return RackUiTheme::Dark;
}

}