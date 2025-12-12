#include "midi-pad.hpp"
#include "services/misc.hpp"
#include "services/colors.hpp"
#include "services/json-help.hpp"
#include "hcl.hpp"

namespace pachde {

// See doc/hcl.md

const char * default_pad_name[] = {
    "A1", "A2", "A3", "A4",
    "B1", "B2", "B3", "B4",
    "C1", "C2", "C3", "C4",
    "D1", "D2", "D3", "D4"
};

MidiPad::MidiPad(int id) : id(id), color(0xff8c8c8c), text_color(0xff000000), error_pos(0)
{
    name = default_pad_name[id];
}

MidiPad::MidiPad(json_t *j) : error_pos(0)
{
    from_json(j);
}

bool MidiPad::compile()
{
    HclCompiler hc;
    ok = hc.compile(this->def, &this->midi);
    if (ok) {
        error_message = "";
        error_pos = 0;
    } else {
        error_message = hc.error_message;
        error_pos = hc.error_pos;
    }
    return ok;
}

json_t * MidiPad::to_json()
{
    json_t * root = json_object();
    set_json_int(root, "pad", id);
    set_json(root, "name", name.c_str());
    set_json(root, "midi", def.c_str());
    set_json(root, "color", hex_string(color).c_str());
    set_json(root, "text-color", hex_string(text_color).c_str());
    return root;
}

void MidiPad::from_json(json_t* root)
{
    id = get_json_int(root, "pad", -1);
    def = get_json_string(root, "midi");
    name = get_json_string(root, "name");
    color = parseColor(get_json_cstring(root, "color"), 0xff8c8c8c);
    text_color = parseColor(get_json_cstring(root, "text-color"), 0xff000000);

    //compile();
}

PadWidget::PadWidget()
{
    box.size = Vec(24.f, 24.f);
}

std::string PadWidget::extract_description()
{
    if (!pad || pad->def.empty()) return "";
    const char * p = pad->def.c_str();
    while (std::isspace(*p)) ++p;
    if (*p == '"') {
        ++p;
        const char * start = p;
        while (*p && *p != '"') {
            ++p;
        }
        return std::string(start, p);
    }
    return "";
}

void PadWidget::init(
    int identifier,
    std::shared_ptr<MidiPad> the_pad,
    Module* module,
    std::shared_ptr<SvgTheme> theme,
    std::function<void(int)> callback)
{
    id = identifier;
    on_click = callback;
    addChild(light = createLightCentered<TinyLight<WhiteLight>>(Vec(20,4), module, identifier));
    addChild(label = createLabel(Vec(12, 8.5), 24, the_pad ? the_pad->name : "", theme, LabelStyle{"", TextAlignment::Center, 12.f}));
    applyTheme(theme);
    set_pad(the_pad);
}

void PadWidget::set_pad(std::shared_ptr<MidiPad> the_pad)
{
    pad = the_pad;
    assert(!pad || id == pad->id);
    on_pad_change(true, true);
}

void PadWidget::on_pad_change(bool name, bool description)
{
    if (pad) {
        label->color(fromPacked(pad->text_color));
        if (name) label->text(pad->name);
        if (description) {
            auto desc = extract_description();
            desc = desc.empty() ? pad->name : pad->name + ": " + desc;
            if (pad->ok) {
                if (pad->midi.empty()) {
                    desc.append("\n(no midi defined)");
                }
            } else {
                desc.push_back('\n');
                desc.append(pad->error_message);
            }
            describe(desc);
        }
    } else {
        label->color(fromPacked(colors::Black));
        label->text("");
        describe("(undefined)");
    }
}

void PadWidget::onHover(const HoverEvent &e)
{
    Base::onHover(e);
    e.consume(this);
}

void PadWidget::onLeave(const LeaveEvent &e)
{
    Base::onLeave(e);
    button_down = false;
}

void PadWidget::onButton(const ButtonEvent &e)
{
    if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
        if (e.action == GLFW_PRESS) {
            button_down = true;
        } else if (e.action == GLFW_RELEASE) {
            button_down = false;
            if (on_click) {
                on_click(id);
            }
        }
    }
    Base::onButton(e);
}

bool PadWidget::applyTheme(std::shared_ptr<SvgTheme> theme)
{
    wire = (0 == theme->name.compare("Wire"));
    pad_style.apply_theme(theme);
    pad_sel_style.apply_theme(theme);
    pad_down_style.apply_theme(theme);
    return true;
}

void PadWidget::step()
{
    Base::step();

    NVGcolor co(fromPacked(0));
    if (selected) {
        co = pad->ok ? SCHEME_YELLOW : SCHEME_RED;
    } else if (pad) {
        co = pad->ok ? SCHEME_GREEN : SCHEME_RED;
    }
    (*light->baseColors.begin()) = co;
}

void PadWidget::draw(const DrawArgs& args)
{
    auto vg = args.vg;
    if (wire && (pad_style.width() > .01f)) {
        nvgBeginPath(vg);
        float o = button_down ? 1 : 0;
        nvgRoundedRect(vg, o, o, 24, 24, 1.5);
        PackedColor co = pad ? (button_down ? pad_down_style.fill_color : pad->color) : pad_style.stroke_color;
        nvgStrokeColor(vg, fromPacked(co));
        nvgStrokeWidth(vg, pad_style.width());
        nvgStroke(vg);
    } else {
        auto co = fromPacked(pad ? (button_down ? pad_down_style.fill_color : pad->color) : pad_style.fill_color);
        if (pad && !pad->empty()) {
            float o = button_down ? 1 : 0;
            RoundRect(vg, o, o, 24, 24, co, 1.5);
        } else {
            nvgBeginPath(vg);
            nvgRoundedRect(vg, 1, 1, 23, 23, 1.5);
            nvgStrokeColor(vg, co);
            nvgStrokeWidth(vg, .5f);
            nvgStroke(vg);
        }
    }

    Base::draw(args);

    if (selected) {
        nvgBeginPath(vg);
        nvgRoundedRect(vg, -.5, -.5, 25, 25, 1.5);
        nvgStrokeColor(vg, pad_sel_style.nvg_stroke_color());
        nvgStrokeWidth(vg, pad_sel_style.width());
        nvgStroke(vg);
    }
}


}