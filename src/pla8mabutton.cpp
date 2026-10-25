/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "pla8mabutton.h"

#include <KColorUtils>
#include <KDecoration3/DecoratedWindow>
#include <KDecoration3/ScaleHelpers>

#include <QPainter>
#include <QPainterPath>

namespace Pla8ma
{
using KDecoration3::DecorationButtonType;

namespace
{
// close button colors
const QColor s_closeNormal(199, 80, 80);
const QColor s_closeHover(224, 67, 67);
const QColor s_closePressed(153, 61, 61);

// classic hover colors of the other buttons
const QColor s_classicHover(54, 101, 179);
const QColor s_classicPressed(61, 96, 153);
}

//__________________________________________________________________
Button::Button(DecorationButtonType type, Decoration *decoration, QObject *parent)
    : DecorationButton(type, decoration, parent)
{
    connect(decoration->window(), &KDecoration3::DecoratedWindow::activeChanged, this, [this]() {
        update();
    });
}

//__________________________________________________________________
Button::Button(QObject *parent, const QVariantList &args)
    : Button(args.at(0).value<DecorationButtonType>(), args.at(1).value<Decoration *>(), parent)
{
    auto d = args.at(1).value<Decoration *>();
    setGeometry(QRectF(QPointF(0, 0), QSizeF(d->buttonWidth(type()), d->buttonHeight())));
}

//__________________________________________________________________
Button *Button::create(DecorationButtonType type, KDecoration3::Decoration *decoration, QObject *parent)
{
    if (auto d = qobject_cast<Decoration *>(decoration)) {
        Button *b = new Button(type, d, parent);
        const auto c = d->window();
        switch (type) {
        case DecorationButtonType::Close:
            b->setVisible(c->isCloseable());
            QObject::connect(c, &KDecoration3::DecoratedWindow::closeableChanged, b, &Pla8ma::Button::setVisible);
            break;

        case DecorationButtonType::Maximize:
            b->setVisible(c->isMaximizeable());
            QObject::connect(c, &KDecoration3::DecoratedWindow::maximizeableChanged, b, &Pla8ma::Button::setVisible);
            break;

        case DecorationButtonType::Minimize:
            b->setVisible(c->isMinimizeable());
            QObject::connect(c, &KDecoration3::DecoratedWindow::minimizeableChanged, b, &Pla8ma::Button::setVisible);
            break;

        case DecorationButtonType::ContextHelp:
            b->setVisible(c->providesContextHelp());
            QObject::connect(c, &KDecoration3::DecoratedWindow::providesContextHelpChanged, b, &Pla8ma::Button::setVisible);
            break;

        case DecorationButtonType::Shade:
            b->setVisible(c->isShadeable());
            QObject::connect(c, &KDecoration3::DecoratedWindow::shadeableChanged, b, &Pla8ma::Button::setVisible);
            break;

        case DecorationButtonType::Menu:
            QObject::connect(c, &KDecoration3::DecoratedWindow::iconChanged, b, [b]() {
                b->update();
            });
            break;

        default:
            break;
        }

        return b;
    }

    return nullptr;
}

//__________________________________________________________________
bool Button::isToggleButton() const
{
    switch (type()) {
    case DecorationButtonType::OnAllDesktops:
    case DecorationButtonType::KeepAbove:
    case DecorationButtonType::KeepBelow:
    case DecorationButtonType::Shade:
    case DecorationButtonType::ExcludeFromCapture:
        return true;
    default:
        return false;
    }
}

//__________________________________________________________________
void Button::paint(QPainter *painter, const QRectF &repaintRegion)
{
    Q_UNUSED(repaintRegion)

    auto d = qobject_cast<Decoration *>(decoration());
    if (!d) {
        return;
    }

    switch (type()) {
    case DecorationButtonType::Menu:
        drawWindowIcon(painter, d);
        break;

    case DecorationButtonType::Spacer:
        break;

    default: {
        painter->save();
        const QColor background = backgroundColor();
        if (background.isValid()) {
            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->fillRect(geometry(), background);
        }
        drawGlyph(painter, foregroundColor());
        painter->restore();
        break;
    }
    }
}

//__________________________________________________________________
void Button::drawWindowIcon(QPainter *painter, Decoration *decoration) const
{
    // the icon is vertically centered with the caption, not with the buttons
    const qreal iconSize = Metrics::Icon_Size * decoration->unit();
    const qreal centerY = std::min(decoration->borderTop() / 2, decoration->borderTop() - iconSize / 2);
    QRectF iconRect(0, 0, iconSize, iconSize);
    iconRect.moveCenter(QPointF(geometry().center().x(), centerY));

    const qreal scale = decoration->window()->scale();
    iconRect.moveTopLeft(QPointF(KDecoration3::snapToPixelGrid(iconRect.left(), scale), KDecoration3::snapToPixelGrid(iconRect.top(), scale)));

    decoration->window()->icon().paint(painter, iconRect.toRect());
}

//__________________________________________________________________
void Button::drawGlyph(QPainter *painter, const QColor &color) const
{
    auto d = qobject_cast<Decoration *>(decoration());
    const qreal scale = d->window()->scale();

    // glyph metrics are designed at 96 dpi for a 21 px high button
    const qreal s = geometry().height() / Metrics::Button_Height;
    const QPointF center = geometry().center();
    auto snap = [scale](qreal value) {
        return KDecoration3::snapToPixelGrid(value, scale);
    };
    // rectangle relative to the button center, aligned to the pixel grid
    auto rect = [&](qreal x, qreal y, qreal w, qreal h) {
        return QRectF(snap(center.x() + x * s), snap(center.y() + y * s), std::max(snap(w * s), KDecoration3::pixelSize(scale)), std::max(snap(h * s), KDecoration3::pixelSize(scale)));
    };
    // window shaped outline with a thick top edge
    auto windowPath = [&](const QRectF &outer) {
        const qreal thin = std::max(snap(s), KDecoration3::pixelSize(scale));
        const qreal thick = std::max(snap(2 * s), 2 * KDecoration3::pixelSize(scale));
        QPainterPath path;
        path.addRect(outer);
        path.addRect(outer.adjusted(thin, thick, -thin, -thin));
        return path;
    };

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);

    switch (type()) {
    case DecorationButtonType::Close: {
        QPen pen(color);
        pen.setWidthF(1.75 * s);
        pen.setCapStyle(Qt::FlatCap);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        const QRectF r = rect(-4.5, -4, 9, 8);
        painter->drawLine(r.topLeft(), r.bottomRight());
        painter->drawLine(r.topRight(), r.bottomLeft());
        break;
    }

    case DecorationButtonType::Maximize: {
        painter->setRenderHint(QPainter::Antialiasing, false);
        if (isChecked()) {
            // restore: two overlapping windows
            const QRectF front = rect(-4.5, -2, 7, 6);
            const QRectF back = rect(-2.5, -5, 7, 6);
            QPainterPath backPath = windowPath(back);
            QPainterPath frontArea;
            frontArea.addRect(front);
            painter->drawPath(backPath.subtracted(frontArea));
            painter->drawPath(windowPath(front));
        } else {
            painter->drawPath(windowPath(rect(-4.5, -4, 9, 8)));
        }
        break;
    }

    case DecorationButtonType::Minimize: {
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->drawRect(rect(-4.5, 2, 9, 2));
        break;
    }

    case DecorationButtonType::OnAllDesktops: {
        // a pin
        if (isChecked()) {
            painter->drawEllipse(rect(-3.5, -3.5, 7, 7));
        } else {
            QPen pen(color);
            pen.setWidthF(1.25 * s);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(rect(-3.5, -3.5, 7, 7));
        }
        break;
    }

    case DecorationButtonType::KeepAbove:
    case DecorationButtonType::KeepBelow:
    case DecorationButtonType::Shade: {
        QPen pen(color);
        pen.setWidthF(1.5 * s);
        pen.setJoinStyle(Qt::MiterJoin);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        const qreal dir = (type() == DecorationButtonType::KeepBelow || (type() == DecorationButtonType::Shade && isChecked())) ? 1 : -1;
        const QPointF c(center.x(), center.y() + (type() == DecorationButtonType::Shade ? 1.5 * s : 0));
        painter->drawPolyline(QList<QPointF>{c + QPointF(-4.5 * s, -2.25 * s * dir), c + QPointF(0, 2.25 * s * dir), c + QPointF(4.5 * s, -2.25 * s * dir)});
        if (type() == DecorationButtonType::Shade) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(color);
            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->drawRect(rect(-4.5, -5, 9, 1.5));
        }
        break;
    }

    case DecorationButtonType::ContextHelp: {
        QFont font = d->settings()->font();
        font.setBold(true);
        font.setPixelSize(qRound(13 * s));
        painter->setFont(font);
        painter->setPen(color);
        painter->drawText(geometry(), Qt::AlignCenter, QStringLiteral("?"));
        break;
    }

    case DecorationButtonType::ApplicationMenu: {
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->drawRect(rect(-4.5, -4, 9, 1.5));
        painter->drawRect(rect(-4.5, -0.75, 9, 1.5));
        painter->drawRect(rect(-4.5, 2.5, 9, 1.5));
        break;
    }

    case DecorationButtonType::ExcludeFromCapture: {
        // an eye, crossed out when the window is excluded
        QPen pen(color);
        pen.setWidthF(1.25 * s);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        QPainterPath eye;
        eye.moveTo(center + QPointF(-5 * s, 0));
        eye.quadTo(center + QPointF(0, -5.5 * s), center + QPointF(5 * s, 0));
        eye.quadTo(center + QPointF(0, 5.5 * s), center + QPointF(-5 * s, 0));
        painter->drawPath(eye);
        painter->setBrush(color);
        painter->drawEllipse(center, 1.5 * s, 1.5 * s);
        if (isChecked()) {
            painter->drawLine(center + QPointF(-5 * s, -4.5 * s), center + QPointF(5 * s, 4.5 * s));
        }
        break;
    }

    default:
        break;
    }
}

//__________________________________________________________________
QColor Button::backgroundColor() const
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d) {
        return QColor();
    }

    if (type() == DecorationButtonType::Close) {
        if (isPressed()) {
            return s_closePressed;
        } else if (isHovered()) {
            return s_closeHover;
        } else if (d->window()->isActive() && d->internalSettings()->redCloseButton()) {
            return s_closeNormal;
        } else if (!d->window()->isActive() && d->internalSettings()->inactiveColorMode() == InternalSettings::InactiveClassic) {
            // classic inactive close button: a darker shade of the frame
            return KColorUtils::mix(d->frameColor(), Qt::black, 0.2);
        }
        return QColor();
    }

    if (d->internalSettings()->hoverColorMode() == InternalSettings::HoverClassic) {
        if (isPressed()) {
            return s_classicPressed;
        } else if (isHovered()) {
            return s_classicHover;
        }
    }

    const QColor frame = d->frameColor();
    if (isPressed()) {
        return KColorUtils::mix(frame, Qt::black, 0.3);
    } else if (isHovered()) {
        // hovered buttons get a lighter tint of the frame color
        return KColorUtils::luma(frame) < 0.8 ? KColorUtils::mix(frame, Qt::white, 0.35) : KColorUtils::mix(frame, Qt::black, 0.12);
    } else if (isToggleButton() && isChecked()) {
        return KColorUtils::mix(frame, d->fontColor(), 0.2);
    }
    return QColor();
}

//__________________________________________________________________
QColor Button::foregroundColor() const
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d) {
        return QColor();
    }

    // white cross on the red close button
    if (type() == DecorationButtonType::Close && backgroundColor().isValid()) {
        return Qt::white;
    }

    // white glyph on the classic blue hover background
    if (d->internalSettings()->hoverColorMode() == InternalSettings::HoverClassic && (isHovered() || isPressed())) {
        return Qt::white;
    }

    QColor color = d->fontColor();
    if (!isEnabled()) {
        color = KColorUtils::mix(color, d->frameColor(), 0.6);
    }
    return color;
}

} // namespace Pla8ma
