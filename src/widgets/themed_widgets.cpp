// Copyright (C) Paul Chase Dempsey
#include "themed_widgets.hpp"
#include "../services/colors.hpp"

namespace pachde {

void notifyChange(Widget* widget) {
    EventContext ctx;
    Widget::ChangeEvent eChange;
    eChange.context = &ctx;
    widget->onChange(eChange);
}

void ThemeColorPort::draw(const DrawArgs& args)
{
    app::SvgPort::draw(args);

    if (pachde::isColorTransparent(ring)) {
        return;
    }
    OpenCircle(args.vg, 8, 8, 4.0f, ring, 2.0f);
}
}