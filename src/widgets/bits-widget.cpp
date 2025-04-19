#include "bits-widget.hpp"

namespace pachde {

constexpr const float ROW_HEIGHT = 10.5f;
constexpr const float MARGIN_DX = 3.5f;
constexpr const float MARGIN_DY = 3.5f;
constexpr const float COLUMN_SEP = 1.5f;
constexpr const float CHECK_DX = 6.f;

BitsWidget::BitsWidget(
    const std::string& name,
    int rows,
    float item_width,
    const std::vector<std::string>& items,
    SvgThemeEngine& theme_engine,
    std::shared_ptr<svg_theme::SvgTheme> theme,
    std::function<void(uint64_t state)> on_change
) :
    name(name),
    count(items.size()),
    rows(rows),
    label_width(item_width),
    change_fn(on_change)
{
    int cols = (items.size() / rows) + ((items.size() % rows) > 0);
    box.size.x = (MARGIN_DX * 2) + (cols * (label_width + CHECK_DX)) + ((cols-1) * COLUMN_SEP);
    float x, y;

    addChild(title = createLabel<TextLabel>(Vec(box.size.x*.5f, MARGIN_DY), 100.f, name, theme_engine, theme, title_style));

    auto r = exit_box_rect();
    addChild(createHoverClickRegion(RECT_ARGS(r), 0, [this](int, int) { close(); }, "option-exit"));

    LabelStyle style{"choice", TextAlignment::Center, 9.f, false};

    y = MARGIN_DY + title->box.size.y + MARGIN_DY;

    TextLabel* none_label = createLabel<TextLabel>(Vec(box.size.x*.5f, y), 32.f, "[ any ]", theme_engine, theme, style);
    addChild(none_label);
    addChild(createHoverClickRegion(RECT_ARGS(none_label->box), 0, [=](int id, int mods) { 
        state = 0;
        if (change_fn) change_fn(state);
    }, "choice-hover"));

    style.align = TextAlignment::Left;
    auto item_it = items.cbegin();
    x = MARGIN_DX + CHECK_DX;
    y += ROW_HEIGHT;
    float top = y;
    for (size_t i = 0; i < items.size(); ++i, item_it++) {

        auto label = createLabel<TextLabel>(Vec(x,y+1), label_width, *item_it, theme_engine, theme, style);
        labels.push_back(label);
        addChild(label);

        Rect r = label->box;
        r.pos.x -= CHECK_DX;
        r.size.x += CHECK_DX;
        addChild(createHoverClickRegion(RECT_ARGS(r), i, [=](int id, int mods) { select_item(id, mods); }, "choice-hover"));

        if (0 == ((i+1) % rows)) {
            x += label_width + COLUMN_SEP + CHECK_DX;
            y = top;
        } else {
            y += ROW_HEIGHT;
        }
    }
    box.size.y = top + (rows * ROW_HEIGHT) + MARGIN_DY;
    ApplyChildrenTheme(this, theme_engine, theme);
}

bool BitsWidget::applyTheme(svg_theme::SvgThemeEngine& theme_engine, std::shared_ptr<svg_theme::SvgTheme> theme)
{
    envelope.apply_theme(theme);
    control_frame.apply_theme(theme);
    control_glyph.apply_theme(theme);
    check_style.apply_theme(theme);
    return true;
}

std::string BitsWidget::make_summary()
{
    if (0 == state) return name + ": (any)";
    std::string result(name);
    result.append(": ");
    int char_count = result.size();
    int index{0};
    bool separator = false;
    for (TextLabel*label: labels) {
        if (state & (uint64_t(1) << index)) {
            if (separator) {
                if (char_count > 26) {
                    result.append(",\n");
                    char_count = 0;
                } else {
                    result.append(", ");
                }
            }
            auto text = label->getText();
            char_count += text.size();
            result.append(text);
            separator = true;
        }
        ++index;
    }
    return result;
}

Vec BitsWidget::check_pos(int i)
{
    Vec pos = labels[i]->box.pos;
    pos.x -= CHECK_DX;
    pos.y += ROW_HEIGHT*.5f;
    return pos;
}

Rect BitsWidget::exit_box_rect() {
    return {box.size.x -  MARGIN_DX - 8.f, MARGIN_DY, 8.f, 8.f};
}

void BitsWidget::close()
{
    setVisible(false);
    auto p = getParent();
    if (p) {
        p->removeChild(this);
    }
}

void BitsWidget::select_item(int id, int mods)
{
    uint64_t bit = uint64_t(1) << id;
    switch (mods) {
    case 0:
        if (state & bit) {
            state = state & ~bit;
        } else {
            state |= bit;
        }
        break;
    case RACK_MOD_CTRL: 
        state = bit;
        break;
    default: 
        return;
    }
    if (change_fn) change_fn(state);
}

void BitsWidget::onSelectKey(const SelectKeyEvent& e)
{
    if (APP->event->getSelectedWidget() != this) {
        Base::onSelectKey(e);
        return;
    }

    if ((e.action == GLFW_PRESS || e.action == GLFW_REPEAT))
    {
        switch (e.key) {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_ENTER:
            if (0 == (e.mods & RACK_MOD_MASK))  {
                APP->event->setSelectedWidget(parent);
                e.consume(this);
                close();
                return;
            }
            break;
        }
    }
}

void BitsWidget::onShow(const ShowEvent& e)
{
    APP->event->setSelectedWidget(this);
    Base::onShow(e);
}

void BitsWidget::onHide(const HideEvent& e)
{
    APP->event->setSelectedWidget(parent);
    Base::onHide(e);
}

void BitsWidget::draw(const DrawArgs& args)
{
    auto vg = args.vg;

    // shadow
    nvgScissor(vg, 0, 0, box.size.x+60, box.size.y +60);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.f, 3.5f, box.size.x+6.f, box.size.y+15.f, 12.f);
    auto p = nvgBoxGradient(vg, 12, 12, box.size.x-30.f, box.size.y-24.f, 12.f, 50.f, nvgRGB(0,0,0), nvgRGBAf(0,0,0,0));
    nvgFillPaint(vg, p);
    nvgFill(vg);
    nvgResetScissor(vg);
    
    FillRect(vg, 0.f, 0.f, box.size.x, box.size.y, envelope.nvg_color());

    float y = title->box.pos.y + title->box.size.y + MARGIN_DY*.5;
    Line(vg, 0, y, box.size.x, y, control_frame.nvg_stroke_color(), .5f);

    FittedBoxRect(vg, 0.f, 0.f, box.size.x, box.size.y, envelope.nvg_stroke_color(), Fit::Inside, envelope.width());
    
    auto r = exit_box_rect();
    BoxRect(vg, RECT_ARGS(r), control_frame.nvg_stroke_color(), .5f);
    r = r.grow({-1.25, -1.25});
    auto co = control_glyph.nvg_stroke_color();
    Line(vg, r.pos.x, r.pos.y, r.pos.x + r.size.x, r.pos.y + r.size.y, co, .65f);
    Line(vg, r.pos.x + r.size.x, r.pos.y, r.pos.x, r.pos.y + r.size.y, co, .65f);

    Base::draw(args);

    auto font = GetPluginFontSemiBold();
    if (!FontOk(font)) return;

    uint64_t bit = 1;
    for (size_t i = 0; i < count; ++i, bit <<= 1) {
        if (bit & state) {
            auto pos = check_pos(i);
            SetTextStyle(vg, font, check_style.nvg_color(), 5.f);
            nvgText(vg, pos.x, pos.y, "✔", nullptr);
        }
    }
}


}