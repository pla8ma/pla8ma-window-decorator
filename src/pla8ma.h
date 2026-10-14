/*
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "pla8masettings.h"

#include <QSharedPointer>

namespace Pla8ma
{
//* convenience typedefs
using InternalSettingsPtr = QSharedPointer<InternalSettings>;

//* metrics, in logical pixels (designed at 96 dpi)
namespace Metrics
{
//* height of the caption buttons for the default button size
static constexpr qreal Button_Height = 21;

//* width of minimize / maximize (and every other non-close) button
static constexpr qreal Button_Width = 26;

//* width of the close button
static constexpr qreal Button_CloseWidth = 46;

//* width of the window menu (icon) button
static constexpr qreal Button_IconWidth = 22;

//* size of the window icon
static constexpr qreal Icon_Size = 16;

//* horizontal space between buttons and caption
static constexpr qreal TitleBar_SideMargin = 6;

//* frame width for the default ("Normal") KWin border size
static constexpr qreal Frame_Normal = 8;

// shadow dimensions (pixels)
static constexpr int Shadow_Overlap = 2;
}

} // namespace Pla8ma
