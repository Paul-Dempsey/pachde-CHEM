#pragma once
#include "label-widget.hpp"

namespace pachde {
namespace style {

constexpr const float U1 = 15.f;
constexpr const float UHALF = 7.5f;

constexpr const ssize_t SSIZE_0 = 0;

constexpr const float PORT_SECTION = 290.f;
constexpr const float PORT_TOP = PORT_SECTION + 26;
constexpr const float PORT_DX = 30.f;
constexpr const float PORT_DY = 34.f;
constexpr const float PORT_LABEL_DY = -20.f;
constexpr const float CLICK_WIDTH = PORT_DX - 2.f;
constexpr const float CLICK_HEIGHT = 21.f;
constexpr const float CLICK_DY = 14.f;

// port active modulation light
constexpr const float PORT_MOD_DX = 12.75f;
constexpr const float PORT_MOD_DY = 12.75f;

constexpr const float CORE_LINK_LEFT = 12.f;
constexpr const float CORE_LINK_TEXT = 28.f;
constexpr const float CORE_LINK_TEXT_DY = 13.f;

extern widgetry::LabelStyle warning_label; // "warning" 9px left
extern widgetry::LabelStyle haken_label;   // "dytext" 10px left
extern widgetry::LabelStyle in_port_label; // "in-label" 12px center
extern widgetry::LabelStyle control_label; // "ctl-label" 14px center
extern widgetry::LabelStyle control_label_left;  // "ctl-label" 14px left
extern widgetry::LabelStyle small_control_label; // "label" 10px center
extern widgetry::LabelStyle med_control_label;   // "label" 12px center

extern widgetry::LabelStyle heading_label; // "ctl-label" 16px center bold
extern widgetry::LabelStyle pedal_label;   // "ped-assign" 9px left

extern const char * const NotConnected;
extern const char * const InputColorKey;
extern const char * const OutputColorKey;

bool show_screws();
bool show_browser_logo();

}}