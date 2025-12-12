// Copyright (C) Paul Chase Dempsey
#include "themed-widgets.hpp"
#include "services/colors.hpp"

namespace widgetry {

void notifyChange(Widget* widget)
{
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
    pachde::OpenCircle(args.vg, 8, 8, 4.0f, ring, ring_width);
}
}