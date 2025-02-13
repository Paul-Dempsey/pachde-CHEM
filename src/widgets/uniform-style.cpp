#include "uniform-style.hpp"
namespace pachde {
namespace style {

const char * const NotConnected = "[not connected]";

LabelStyle warning_label {"warning",   TextAlignment::Left,      9.f};
LabelStyle haken_label   {"dytext",    TextAlignment::Left,     10.f};
LabelStyle in_port_label {"in-label",  TextAlignment::Center,   12.f, true};
LabelStyle control_label {"ctl-label", TextAlignment::Center,   14.f, false};
LabelStyle heading_label {"ctl-label", TextAlignment::Center,   16.f, true};
LabelStyle pedal_label   {"ped-assign", TextAlignment::Left,      9.f, false};

}}