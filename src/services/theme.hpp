#pragma once
#include <rack.hpp>
#include "colors.hpp"
using namespace ::rack;

namespace theme {

// Special theme names are proxies for FollowRackUi and FollowRackPreferDark
namespace theme_name {
    extern const char * Ui;
    extern const char * PreferDark;
}
enum class RackUiTheme {
    Light, Dark, HighContrast
};

RackUiTheme get_rack_ui_theme();
inline bool is_special_theme(const std::string& theme) { return *theme.cbegin() == '~'; }
std::string get_actual_theme(const std::string& theme);
inline std::string get_default_theme() { return get_actual_theme(theme_name::PreferDark); }
std::string themeDisplayName(const std::string& theme_name);

struct IThemeHolder
{
    virtual std::string getThemeName() = 0;
    virtual void setThemeName(const std::string& theme_name, void *context) = 0;
};

}
