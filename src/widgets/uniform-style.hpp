#pragma once
#include "label-widget.hpp"

namespace pachde {
namespace style {

constexpr const float U1 = 15.f;
constexpr const float UHALF = 7.5f;
   
constexpr const ssize_t SSIZE_0 = 0;

constexpr const float PORT_SECTION = 278.f;
constexpr const float PORT_TOP = 305.f;
constexpr const float PORT_DX = 34.f;
constexpr const float PORT_DY = 36.f;
constexpr const float PORT_LABEL_DY = -20.f;

// port active attenuator light
constexpr const float PORT_ATT_DX = 12.75f;
constexpr const float PORT_ATT_DY = 12.75f;

constexpr const float CORE_LINK_LEFT = 12.f;
constexpr const float CORE_LINK_TEXT = 28.f;
constexpr const float CORE_LINK_TEXT_DY = 13.f;

extern pachde::LabelStyle warning_label; // "warning" 9px left
extern pachde::LabelStyle haken_label;   // "dytext" 10px left
extern pachde::LabelStyle in_port_label; // "in-label" 12px Center
extern pachde::LabelStyle control_label; // "ctl-label" 14px Center
extern pachde::LabelStyle control_label_left; // "ctl-label" 14px left
extern pachde::LabelStyle heading_label; // "ctl-label" 16px Center bold
extern pachde::LabelStyle pedal_label;   // "ped-assign" 9px Lext

extern const char * const NotConnected;
extern const char * const InputColorKey;
extern const char * const OutputColorKey;

}}