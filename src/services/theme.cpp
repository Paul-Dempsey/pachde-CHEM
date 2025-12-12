#include "theme.hpp"

namespace theme {

namespace theme_name {
    const char * Ui = "~u";
    const char * PreferDark = "~d";
}

std::string get_actual_theme(const std::string &theme_name)
{
    if (theme_name.empty()) return "Dark";

    if (is_special_theme(theme_name)) {
        assert(theme_name.size() == 2);
        switch (theme_name[1]) {
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
    return theme_name;
}

std::string themeDisplayName(const std::string &theme_name) {
    if (is_special_theme(theme_name)) {
        switch (theme_name[1]) {
        case 'u': return "Follow Rack UI theme";
        case 'd': return "Follow Rack 'Prefer dark panels'";
        default:
            assert(false);
            return "[UNEXPECTED INTERNAL ERROR]";
        }
    }
    return theme_name;
}

RackUiTheme get_rack_ui_theme()
{
    //if (::rack::settings::uiTheme == "dark")   return RackUiTheme::Dark;
    if (::rack::settings::uiTheme == "light")  return RackUiTheme::Light;
    if (::rack::settings::uiTheme == "hcdark") return RackUiTheme::HighContrast;
    return RackUiTheme::Dark;
}

}