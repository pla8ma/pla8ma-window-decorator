/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "pla8ma.h"

#include <KDecoration3/DecoratedWindow>
#include <KDecoration3/Decoration>
#include <KDecoration3/DecorationButton>
#include <KDecoration3/DecorationSettings>

#include <QVariant>

namespace KDecoration3
{
class DecorationButtonGroup;
class DecorationShadow;
}

namespace Pla8ma
{
class Decoration : public KDecoration3::Decoration
{
    Q_OBJECT

public:
    explicit Decoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    ~Decoration() override;

    void paint(QPainter *painter, const QRectF &repaintRegion) override;

    InternalSettingsPtr internalSettings() const
    {
        return m_internalSettings;
    }

    //*@name colors
    //@{
    //* frame color of the active window, follows the KDE color scheme / accent color
    QColor activeFrameColor() const;

    //* frame color of inactive windows
    QColor inactiveFrameColor() const;

    //* frame color for the current state
    QColor frameColor() const;

    //* caption and button glyph color
    QColor fontColor() const;

    //* thin line around the frame and around the client area
    QColor outlineColor() const;
    //@}

    //*@name metrics
    //@{
    //* scale factor for X11 (smallSpacing is constant 2 on Wayland)
    qreal unit() const;

    qreal buttonHeight() const;
    qreal buttonWidth(KDecoration3::DecorationButtonType type) const;
    //@}

    //*@name maximization modes
    //@{
    bool isMaximized() const
    {
        return window()->isMaximized();
    }

    bool isLeftEdge() const
    {
        return window()->isMaximizedHorizontally() || window()->adjacentScreenEdges().testFlag(Qt::LeftEdge);
    }

    bool isRightEdge() const
    {
        return window()->isMaximizedHorizontally() || window()->adjacentScreenEdges().testFlag(Qt::RightEdge);
    }

    bool isTopEdge() const
    {
        return window()->isMaximizedVertically() || window()->adjacentScreenEdges().testFlag(Qt::TopEdge);
    }

    bool isBottomEdge() const
    {
        return window()->isMaximizedVertically() || window()->adjacentScreenEdges().testFlag(Qt::BottomEdge);
    }
    //@}

public Q_SLOTS:
    bool init() override;

private Q_SLOTS:
    void reconfigure();
    void recalculateBorders();
    void updateButtonsGeometry();
    void updateButtonsGeometryDelayed();
    void updateTitleBar();
    void updateShadow();

private:
    //* return the rect in which caption will be drawn
    QPair<QRectF, Qt::Alignment> captionRect() const;

    void createButtons();
    void paintTitleBar(QPainter *painter, const QRectF &repaintRegion);
    std::shared_ptr<KDecoration3::DecorationShadow> createShadowObject(qreal strengthScale);

    //*@name border size
    //@{
    //* width of the left / right (bottom = false) or bottom frame
    qreal frameWidth(bool bottom, qreal scale) const;

    //* space above the caption buttons (a thin strip of frame above them)
    qreal buttonsTopOffset() const;

    bool hasNoBorders() const
    {
        return settings()->borderSize() == KDecoration3::BorderSize::None;
    }

    bool hasNoSideBorders() const
    {
        return settings()->borderSize() == KDecoration3::BorderSize::NoSides;
    }

    QMarginsF bordersFor(qreal scale) const;
    //@}

    InternalSettingsPtr m_internalSettings;
    KDecoration3::DecorationButtonGroup *m_leftButtons = nullptr;
    KDecoration3::DecorationButtonGroup *m_rightButtons = nullptr;
};

} // namespace Pla8ma
