#include "my-plugin.hpp"
#include "services/svg-query.hpp"
#include "services/svg-theme.hpp"
using namespace svg_theme;
#define DIALOG_THEMED
#include "widgets/dialog.hpp"
#include "widgets/dialog-help.hpp"
#include "widgets/label.hpp"

namespace widgetry {

struct DialogSvg {
    static std::string background() {
        return asset::plugin(pluginInstance, "res/dialogs/preset-midi-config.svg");
    }
};

struct ConfigPresetMidi : SvgDialog<DialogSvg> {
    using Base = SvgDialog<DialogSvg>;

    SvgCache my_svgs;
    DialogStyles styles;

    ConfigPresetMidi(ModuleWidget* source, ILoadSvg* svg_loader) :
        Base(source, svg_loader)
    {
    }

    void create_ui(std::shared_ptr<svg_theme::SvgTheme> svg_theme) {
        auto layout = Base::get_svg();
        applySvgTheme(layout, svg_theme);
        styles.createStyles(svg_theme);
        ::svg_query::BoundsIndex bounds;
        svg_query::addBounds(layout, "k:", bounds, true);

        add_close_button(this, bounds, "k:close", svg_theme);
        addChild(createLabel(bounds["k:dlg-title"], "Preset | Midi configuration", styles.title_style));
    }

};

void show_preset_midi_configuration(ModuleWidget* source, std::shared_ptr<svg_theme::SvgTheme> svg_theme) {
    SvgNoCache nocache;
    auto dlg = createDialog<ConfigPresetMidi, ModuleWidget>(source, Vec(source->box.size.x *.5, source->box.size.y *.5 - 15.f), &nocache, true);
    dlg->create_ui(svg_theme);
}

};