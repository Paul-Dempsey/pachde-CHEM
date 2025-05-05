#include "XM.hpp"
#include "../../em/em-hardware.h"
#include "../../widgets/click-region-widget.hpp"
#include "../../widgets/logo-widget.hpp"
#include "../../widgets/uniform-style.hpp"

namespace S = pachde::style;
using namespace svg_theme;
using namespace pachde;
constexpr const float PANEL_WIDTH   = 45.f;
constexpr const float CENTER        = PANEL_WIDTH*.5f;
constexpr const float KNOBS_TOP     = 28;
constexpr const float KNOB_DY       = 32;
constexpr const float KNOB_LABEL_DY = 9;
constexpr const float PORT_TOP_CY   = 286;
constexpr const float PORT_DY       = 19.f;
constexpr const float PORT_MOD_DX   = 10.F;
constexpr const float PORT_MOD_DY   = 8.F;
constexpr const float PORT_OFFSET   = 9.f;

XMUi::XMUi(XMModule *module) :
    my_module(module)
{
    setModule(module);
    title_bg = my_module ? my_module->title_bg : GetPackedStockColor(StockColor::pachde_blue_medium);
    title_fg = my_module ? my_module->title_fg : GetPackedStockColor(StockColor::Gray_65p);

    initThemeEngine();
    auto theme = theme_engine.getTheme(getThemeName());
    auto panel = createThemedPanel(panelFilename(), theme_engine, theme);
    panelBorder = attachPartnerPanelBorder(panel, theme_engine, theme);
    setPanel(panel);

    macro_data = my_module ? &my_module->macros : new MacroData();
    if (macro_data->empty()) {
        macro_data->init(8);
    }

//    float x, y;
    bool browsing = !module;

    title_bar = createWidget<Swatch>(Vec(0, 0));
    title_bar->box.size = Vec(PANEL_WIDTH, 15.f);
    title_bar->color = title_bg;
    addChild(title_bar);
    LabelStyle no_style{"", TextAlignment::Center, 12.f, true};
    addChild(title = createLabel(Vec(CENTER, 1.5), PANEL_WIDTH, my_module ? my_module->title : "XM", theme_engine, nullptr, no_style));
    title->color(fromPacked(title_fg));

    LabelStyle macro_label_style{"ctl--label", TextAlignment::Center, 10.f, false};
    for (int i = 0; i < 8; ++i) {
        MacroUi& macro = macro_ui[i];
        auto pos = knob_center(i);
        macro.knob = createChemKnob<GreenTrimPot>(pos, my_module, i, theme_engine, theme);
        addChild(macro.knob);

        macro.track = createTrackWidget(macro.knob, theme_engine, theme);
        addChild(macro.track);

        pos.y += KNOB_LABEL_DY;
        auto md = macro_data->get_macro(i);
        macro.label = createLabel(pos, 45, md ? md->name : "", theme_engine, theme, macro_label_style);
        addChild(macro.label);
    }

    // inputs
    const NVGcolor co_port = PORT_CORN;
    float y = PORT_TOP_CY;
    addChild(createChemKnob<TrimPot>(Vec(CENTER, y), module, XMModule::P_MODULATION, theme_engine, theme));

    y += PORT_DY;
    float top = y;
    float x = CENTER - PORT_OFFSET;
    float axis = -1.f;
    for (int i = 0; i < 8; ++i) {
        addChild(Center(createThemedColorInput(Vec(x, y), my_module, i, S::InputColorKey, co_port, theme_engine, theme)));
        addChild(Center(createLight<TinySimpleLight<GreenLight>>(Vec(x + (axis*PORT_MOD_DX), y - PORT_MOD_DY), my_module, i)));
        if (my_module) {
            addChild(Center(createClickRegion(x + 2*axis, y - .5f, 21.75f, 19.f, i, [=](int id, int mods) { my_module->set_modulation_target(id); })));
        }
        if (i == 3) {
            axis = 1.f;
            x = CENTER + PORT_OFFSET;
            y = top;
        } else {
            y += PORT_DY;
        }
    }

    // Browsing UI

    if (browsing) {
    }

    // init
    if (!my_module || my_module->glow_knobs) {
        glowing_knobs(true);
    }

    if (my_module) {
        my_module->set_chem_ui(this);
//        onConnectHost(my_module->chem_host);
    }
}

void XMUi::glowing_knobs(bool glow) {
    for (int i = 0; i < 8; ++i) {
        macro_ui[i].knob->glowing(glow);
    }
}

void XMUi::center_knobs()
{
    if (!my_module) return;
}

Module *XMUi::get_edit_module()
{
    if (!my_module) return nullptr;
    Module * mod = my_module->getRightExpander().module;
    if (mod && mod->getModel() == modelXMEdit) {
        return mod;
    }
    return nullptr;
}

Vec XMUi::knob_center(int index)
{
    return Vec(CENTER, KNOBS_TOP + (KNOB_DY * index));
}

void XMUi::setThemeName(const std::string& name, void * context)
{
    auto theme = theme_engine.getTheme(name);
    current_style.apply_theme(theme);
    Base::setThemeName(name, context);
}

void XMUi::onConnectHost(IChemHost* host)
{
    chem_host = host;
    onConnectionChange(ChemDevice::Haken, host ? host->host_connection(ChemDevice::Haken) : nullptr);
}

void XMUi::onConnectionChange(ChemDevice device, std::shared_ptr<MidiDeviceConnection> connection)
{
    //onConnectionChangeUiImpl(this, device, connection);
}

bool XMUi::host_connected() {
    if (!my_module) return false;
    if (!chem_host) return false;
    return true;
}

void XMUi::sync_labels()
{
}

void XMUi::onHoverKey(const HoverKeyEvent &e)
{
    if (my_module) {
        if (e.action == GLFW_PRESS && ((e.mods & RACK_MOD_MASK) == 0)) {
            switch (e.key) {
            case GLFW_KEY_0:
                e.consume(this);
                //my_module->modulation.zero_modulation();
                return;
            case GLFW_KEY_5:
                center_knobs();
                e.consume(this);
                return;
            }
        }
    }
    Base::onHoverKey(e);
}

std::string XMUi::get_title()
{
    return title->getText();
}

PackedColor XMUi::get_header_color()
{
    return title_bg;
}

PackedColor XMUi::get_header_text_color()
{
    return title_fg;
}

void XMUi::set_header_color(PackedColor color)
{
    title_bg = color;
    title_bar->color = color;
    if (my_module) my_module->title_bg = color;
}

void XMUi::set_header_text_color(PackedColor color)
{
    title_fg = color;
    title->color(fromPacked(color));
    if (my_module) my_module->title_fg = color;
}

void XMUi::set_header_text(std::string title)
{
    this->title->text(title);
    if (my_module) my_module->title = title;
}

void XMUi::set_macro_edit(int index)
{
    edit_item = index;
}

std::shared_ptr<MacroDescription> XMUi::get_macro(int index)
{
    return macro_data->get_macro(index);
}

void XMUi::add_macro(int index)
{

}

void XMUi::remove_macro(int index)
{
    MacroUi& macro = macro_ui[index];
    if (macro.knob) {
        macro.knob->setVisible(false);
    }
    if (macro.track) {
        macro.track->setVisible(false);
    }
    if (macro.label) macro.label->text("");

    auto data = get_macro(index);
    if (data) data->clear();
}

void XMUi::on_macro_change(int index)
{
    MacroUi& macro = macro_ui[index];
    auto data = get_macro(index);
    macro.label->text(data->name);

}

void XMUi::step()
{
    Base::step();
    if (!my_module) return;
    //bind_host(my_module);

    bool left = panelBorder->left;
    bool right = panelBorder->right;
    if (!left) {
        auto e = my_module->getLeftExpander().module;
        left = e && e->getModel() == modelXM;
    }
    if (!right) {
        auto e = my_module->getRightExpander().module;
        right = e && ((e->getModel() == modelXM) || (e->getModel() == modelXMEdit));
    }
    panelBorder->setPartners(left, right);

    //knobs[K_MODULATION]->enable(my_module->modulation.has_target());

    // for (int i = 0; i < K_MODULATION; ++i) {
    //     tracks[i]->set_value(my_module->modulation.get_port(i).modulated());
    //     tracks[i]->set_active(my_module->getInput(i).isConnected());
    // }

    sync_labels();
}

void XMUi::draw(const DrawArgs& args)
{
    Base::draw(args);
    auto vg = args.vg;

    if (editing()) {
        auto pos = knob_center(edit_item);
        auto co = current_style.nvg_color();

        float stroke_width = current_style.width();
        float half_width = stroke_width*.5f;
        float y = pos.y - 12.5f;
        float y2 = pos.y + KNOB_LABEL_DY + 11.5f;
        float right = box.size.x;

        Line(vg, 0, y, right, y, co, stroke_width);
        Line(vg, 0, y2, right, y2, co, stroke_width);
        Line(vg, half_width, y, half_width, y2, co, stroke_width);
        right -= half_width;
        Line(vg, right, y, right, y + 10.f, co, stroke_width);
        Line(vg, right, y2 - 10.f, right, y2, co, stroke_width);
    }
}

void XMUi::appendContextMenu(Menu *menu)
{
    if (!module) return;
    menu->addChild(new MenuSeparator);

    // bool unconnected = (my_module->inputs.end() == std::find_if(my_module->inputs.begin(), my_module->inputs.end(), [](Input& in){ return in.isConnected(); }));
    // menu->addChild(createMenuItem("Zero modulation", "0", [this](){
    //     my_module->modulation.zero_modulation();
    // }, unconnected));
    
    menu->addChild(createMenuItem("Center knobs", "5", [this](){ center_knobs(); }));

    menu->addChild(createCheckMenuItem("Glowing knobs", "", 
        [this](){ return my_module->glow_knobs; },
        [this](){
            my_module->glow_knobs = !my_module->glow_knobs; 
            glowing_knobs(my_module->glow_knobs);
        }
    ));
    Base::appendContextMenu(menu);
}
