// Copyright (C) Paul Chase Dempsey
#include "themed_widgets.hpp"
#include "../services/colors.hpp"

using namespace pachde;

void ThemeColorPort::draw(const DrawArgs& args)
{
    app::SvgPort::draw(args);

    if (pachde::isColorTransparent(ring)) {
        return;
    }
    OpenCircle(args.vg, 8, 8, 4.0f, ring, 2.0f);
}