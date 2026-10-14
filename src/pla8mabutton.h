/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "pla8madecoration.h"

#include <KDecoration3/DecorationButton>

namespace Pla8ma
{
class Button : public KDecoration3::DecorationButton
{
    Q_OBJECT

public:
    //* constructor used by the plugin factory (e.g. the button preview in System Settings)
    explicit Button(QObject *parent, const QVariantList &args);

    ~Button() override = default;

    //* button creation
    static Button *create(KDecoration3::DecorationButtonType type, KDecoration3::Decoration *decoration, QObject *parent);

    //* render
    void paint(QPainter *painter, const QRectF &repaintRegion) override;

private:
    explicit Button(KDecoration3::DecorationButtonType type, Decoration *decoration, QObject *parent = nullptr);

    //* draw the window icon
    void drawWindowIcon(QPainter *painter, Decoration *decoration) const;

    //* draw button glyph centered in the button
    void drawGlyph(QPainter *painter, const QColor &color) const;

    //*@name colors
    //@{
    QColor foregroundColor() const;
    QColor backgroundColor() const;
    //@}

    //* true for the toggle buttons (keep above, shade, ...)
    bool isToggleButton() const;
};

} // namespace Pla8ma
