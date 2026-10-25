/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2018 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 * SPDX-FileCopyrightText: 2021 Paul McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "pla8madecoration.h"

#include "pla8maboxshadowrenderer.h"
#include "pla8mabutton.h"
#include "pla8masettingsprovider.h"

#include <KDecoration3/DecorationButtonGroup>
#include <KDecoration3/DecorationShadow>
#include <KDecoration3/ScaleHelpers>

#include <KColorUtils>
#include <KPluginFactory>

#include <QDBusConnection>
#include <QPainter>
#include <QTimer>

K_PLUGIN_FACTORY_WITH_JSON(Pla8maDecoFactory, "pla8ma.json", registerPlugin<Pla8ma::Decoration>(); registerPlugin<Pla8ma::Button>();)

namespace
{
struct ShadowParams {
    QPoint offset;
    int radius = 0;
    qreal opacity = 0;
};

struct CompositeShadowParams {
    QPoint offset;
    ShadowParams shadow1;
    ShadowParams shadow2;

    bool isNone() const
    {
        return qMax(shadow1.radius, shadow2.radius) == 0;
    }
};

// soft, fairly tight shadow around square windows
const CompositeShadowParams s_shadowParams[] = {
    // None
    CompositeShadowParams(),
    // Small
    CompositeShadowParams{QPoint(0, 2), ShadowParams{QPoint(0, 0), 10, 0.45}, ShadowParams{QPoint(0, 0), 4, 0.15}},
    // Medium
    CompositeShadowParams{QPoint(0, 4), ShadowParams{QPoint(0, 0), 20, 0.5}, ShadowParams{QPoint(0, -2), 8, 0.15}},
    // Large
    CompositeShadowParams{QPoint(0, 8), ShadowParams{QPoint(0, 0), 36, 0.55}, ShadowParams{QPoint(0, -4), 14, 0.15}},
};

CompositeShadowParams lookupShadowParams(int size)
{
    switch (size) {
    case Pla8ma::InternalSettings::ShadowNone:
        return s_shadowParams[0];
    case Pla8ma::InternalSettings::ShadowSmall:
        return s_shadowParams[1];
    case Pla8ma::InternalSettings::ShadowLarge:
        return s_shadowParams[3];
    case Pla8ma::InternalSettings::ShadowMedium:
    default:
        return s_shadowParams[2];
    }
}
}

namespace Pla8ma
{
using KDecoration3::ColorGroup;
using KDecoration3::ColorRole;
using KDecoration3::DecorationButtonType;

//________________________________________________________________
static int g_sDecoCount = 0;
static int g_shadowSizeEnum = InternalSettings::ShadowMedium;
static int g_shadowStrength = 160;
static std::shared_ptr<KDecoration3::DecorationShadow> g_sShadow;
static std::shared_ptr<KDecoration3::DecorationShadow> g_sShadowInactive;

//________________________________________________________________
Decoration::Decoration(QObject *parent, const QVariantList &args)
    : KDecoration3::Decoration(parent, args)
{
    g_sDecoCount++;
}

//________________________________________________________________
Decoration::~Decoration()
{
    g_sDecoCount--;
    if (g_sDecoCount == 0) {
        // last deco destroyed, clean up shadow
        g_sShadow.reset();
        g_sShadowInactive.reset();
    }
}

//________________________________________________________________
QColor Decoration::activeFrameColor() const
{
    QColor color;
    switch (m_internalSettings->colorMode()) {
    case InternalSettings::ColorCustom:
        color = m_internalSettings->customColor();
        break;
    case InternalSettings::ColorSchemeTitleBar:
        color = window()->color(ColorGroup::Active, ColorRole::TitleBar);
        break;
    case InternalSettings::ColorAccent:
    default:
        // KDE writes the accent color into the Highlight role of the color scheme
        color = window()->palette().color(QPalette::Active, QPalette::Highlight);
        break;
    }

    // "color intensity": fade towards the window background, like a classic intensity slider
    const qreal intensity = m_internalSettings->colorIntensity() / 100.0;
    if (intensity < 1.0) {
        color = KColorUtils::mix(window()->palette().color(QPalette::Active, QPalette::Window), color, intensity);
    }
    return color;
}

//________________________________________________________________
QColor Decoration::inactiveFrameColor() const
{
    switch (m_internalSettings->inactiveColorMode()) {
    case InternalSettings::InactiveSameAsActive:
        return activeFrameColor();
    case InternalSettings::InactiveSchemeTitleBar:
        return window()->color(ColorGroup::Inactive, ColorRole::TitleBar);
    case InternalSettings::InactiveCustom:
        return m_internalSettings->inactiveCustomColor();
    case InternalSettings::InactiveClassic:
        // neutral light gray, independent of the frame color
        return QColor(235, 235, 235);
    case InternalSettings::InactiveFaded:
    default:
        // inactive frames are a pale, washed out version of the frame color
        return KColorUtils::mix(window()->palette().color(QPalette::Inactive, QPalette::Window), activeFrameColor(), 0.3);
    }
}

//________________________________________________________________
QColor Decoration::frameColor() const
{
    return window()->isActive() ? activeFrameColor() : inactiveFrameColor();
}

//________________________________________________________________
QColor Decoration::fontColor() const
{
    const bool active = window()->isActive();
    QColor color;
    switch (m_internalSettings->textColorMode()) {
    case InternalSettings::TextScheme:
        return window()->color(active ? ColorGroup::Active : ColorGroup::Inactive, ColorRole::Foreground);
    case InternalSettings::TextCustom:
        color = m_internalSettings->customTextColor();
        break;
    case InternalSettings::TextAutomatic:
    default: {
        // dark gray caption, white when the frame is too dark for it
        const QColor frame = frameColor();
        const QColor dark(40, 40, 40);
        const QColor white(255, 255, 255);
        color = KColorUtils::contrastRatio(frame, dark) >= KColorUtils::contrastRatio(frame, white) ? dark : white;
        break;
    }
    }

    // the classic inactive look keeps the caption at full strength
    if (!active && m_internalSettings->inactiveColorMode() != InternalSettings::InactiveClassic) {
        color = KColorUtils::mix(color, frameColor(), 0.45);
    }
    return color;
}

//________________________________________________________________
QColor Decoration::outlineColor() const
{
    return KColorUtils::mix(frameColor(), Qt::black, window()->isActive() ? 0.23 : 0.1);
}

//________________________________________________________________
QColor Decoration::innerOutlineColor() const
{
    return KColorUtils::mix(frameColor(), Qt::black, window()->isActive() ? 0.15 : 0.07);
}

//________________________________________________________________
qreal Decoration::unit() const
{
    // On X11, the smallSpacing value is used for scaling.
    // On Wayland, this value has constant factor of 2.
    return settings()->smallSpacing() / 2.0;
}

//________________________________________________________________
qreal Decoration::buttonHeight() const
{
    qreal factor = 1.0;
    switch (m_internalSettings->buttonSize()) {
    case InternalSettings::ButtonSmall:
        factor = 0.85;
        break;
    case InternalSettings::ButtonLarge:
        factor = 1.25;
        break;
    case InternalSettings::ButtonDefault:
    default:
        break;
    }

    // never smaller than the caption font
    const qreal height = std::max<qreal>(Metrics::Button_Height * factor * unit(), settings()->fontMetrics().height() + 2 * unit());
    return KDecoration3::snapToPixelGrid(height, window()->nextScale());
}

//________________________________________________________________
qreal Decoration::buttonWidth(DecorationButtonType type) const
{
    const qreal ratio = buttonHeight() / Metrics::Button_Height;
    qreal width = Metrics::Button_Width;
    switch (type) {
    case DecorationButtonType::Close:
        width = Metrics::Button_CloseWidth;
        break;
    case DecorationButtonType::Maximize:
        width = Metrics::Button_MaximizeWidth;
        break;
    case DecorationButtonType::Menu:
        width = Metrics::Button_IconWidth;
        break;
    case DecorationButtonType::Spacer:
        width = Metrics::Button_Width / 2;
        break;
    default:
        break;
    }
    return KDecoration3::snapToPixelGrid(width * ratio, window()->nextScale());
}

//________________________________________________________________
bool Decoration::init()
{
    m_internalSettings = SettingsProvider::self()->internalSettings();

    // use DBus connection to update on global settings change (fonts, colors, ...)
    auto dbus = QDBusConnection::sessionBus();
    dbus.connect(QString(),
                 QStringLiteral("/KGlobalSettings"),
                 QStringLiteral("org.kde.KGlobalSettings"),
                 QStringLiteral("notifyChange"),
                 this,
                 SLOT(reconfigure()));

    reconfigure();
    updateTitleBar();

    auto s = settings();
    connect(s.get(), &KDecoration3::DecorationSettings::borderSizeChanged, this, &Decoration::recalculateBorders);

    // a change in font might cause the borders to change
    connect(s.get(), &KDecoration3::DecorationSettings::fontChanged, this, &Decoration::recalculateBorders);
    connect(s.get(), &KDecoration3::DecorationSettings::spacingChanged, this, &Decoration::recalculateBorders);

    // buttons
    connect(s.get(), &KDecoration3::DecorationSettings::spacingChanged, this, &Decoration::updateButtonsGeometryDelayed);
    connect(s.get(), &KDecoration3::DecorationSettings::decorationButtonsLeftChanged, this, &Decoration::updateButtonsGeometryDelayed);
    connect(s.get(), &KDecoration3::DecorationSettings::decorationButtonsRightChanged, this, &Decoration::updateButtonsGeometryDelayed);

    // full reconfiguration
    connect(s.get(), &KDecoration3::DecorationSettings::reconfigured, SettingsProvider::self(), &SettingsProvider::reconfigure, Qt::UniqueConnection);
    connect(s.get(), &KDecoration3::DecorationSettings::reconfigured, this, &Decoration::reconfigure);
    connect(s.get(), &KDecoration3::DecorationSettings::reconfigured, this, &Decoration::updateButtonsGeometryDelayed);

    // the frame color follows the color scheme / accent color of the window
    connect(window(), &KDecoration3::DecoratedWindow::paletteChanged, this, &Decoration::recalculateBorders);
    connect(window(), &KDecoration3::DecoratedWindow::paletteChanged, this, [this]() {
        update();
    });

    connect(window(), &KDecoration3::DecoratedWindow::activeChanged, this, &Decoration::recalculateBorders);
    connect(window(), &KDecoration3::DecoratedWindow::activeChanged, this, &Decoration::updateShadow);
    connect(window(), &KDecoration3::DecoratedWindow::activeChanged, this, [this]() {
        update();
    });
    connect(window(), &KDecoration3::DecoratedWindow::adjacentScreenEdgesChanged, this, &Decoration::recalculateBorders);
    connect(window(), &KDecoration3::DecoratedWindow::maximizedHorizontallyChanged, this, &Decoration::recalculateBorders);
    connect(window(), &KDecoration3::DecoratedWindow::maximizedVerticallyChanged, this, &Decoration::recalculateBorders);
    connect(window(), &KDecoration3::DecoratedWindow::shadedChanged, this, &Decoration::recalculateBorders);
    connect(window(), &KDecoration3::DecoratedWindow::captionChanged, this, [this]() {
        // update the caption area
        update(titleBar());
    });

    connect(this, &KDecoration3::Decoration::bordersChanged, this, &Decoration::updateTitleBar);
    connect(window(), &KDecoration3::DecoratedWindow::adjacentScreenEdgesChanged, this, &Decoration::updateTitleBar);
    connect(window(), &KDecoration3::DecoratedWindow::widthChanged, this, &Decoration::updateTitleBar);
    connect(window(), &KDecoration3::DecoratedWindow::maximizedChanged, this, &Decoration::updateTitleBar);
    connect(window(), &KDecoration3::DecoratedWindow::maximizedChanged, this, &Decoration::setOpaque);

    connect(this, &KDecoration3::Decoration::bordersChanged, this, &Decoration::updateButtonsGeometry);
    connect(window(), &KDecoration3::DecoratedWindow::widthChanged, this, &Decoration::updateButtonsGeometry);
    connect(window(), &KDecoration3::DecoratedWindow::maximizedChanged, this, &Decoration::updateButtonsGeometry);
    connect(window(), &KDecoration3::DecoratedWindow::adjacentScreenEdgesChanged, this, &Decoration::updateButtonsGeometry);
    connect(window(), &KDecoration3::DecoratedWindow::shadedChanged, this, &Decoration::updateButtonsGeometry);

    connect(window(), &KDecoration3::DecoratedWindow::nextScaleChanged, this, &Decoration::recalculateBorders);
    connect(window(), &KDecoration3::DecoratedWindow::nextScaleChanged, this, &Decoration::updateButtonsGeometryDelayed);

    createButtons();
    updateShadow();
    return true;
}

//________________________________________________________________
void Decoration::updateTitleBar()
{
    // The titlebar rect has margins around it so the window can be resized by dragging a decoration edge.
    const bool maximized = isMaximized();
    const qreal sideMargin = maximized ? 0 : 2 * unit();
    const qreal topMargin = (maximized || isTopEdge()) ? 0 : 3 * unit();
    setTitleBar(QRectF(sideMargin, topMargin, window()->width() - 2 * sideMargin, borderTop() - topMargin));
}

//________________________________________________________________
qreal Decoration::frameWidth(bool bottom, qreal scale) const
{
    qreal width = 0;
    switch (settings()->borderSize()) {
    case KDecoration3::BorderSize::None:
        width = 0;
        break;
    case KDecoration3::BorderSize::NoSides:
        width = bottom ? 4 : 0;
        break;
    case KDecoration3::BorderSize::Tiny:
        width = 4;
        break;
    case KDecoration3::BorderSize::Normal:
    default:
        width = Metrics::Frame_Normal;
        break;
    case KDecoration3::BorderSize::Large:
        width = 10;
        break;
    case KDecoration3::BorderSize::VeryLarge:
        width = 12;
        break;
    case KDecoration3::BorderSize::Huge:
        width = 16;
        break;
    case KDecoration3::BorderSize::VeryHuge:
        width = 20;
        break;
    case KDecoration3::BorderSize::Oversized:
        width = 32;
        break;
    }
    return KDecoration3::snapToPixelGrid(width * unit(), scale);
}

//________________________________________________________________
qreal Decoration::buttonsTopOffset() const
{
    // buttons hang from the top edge, with a one pixel gap
    if (isMaximized() || isTopEdge()) {
        return 0;
    }
    return KDecoration3::snapToPixelGrid(unit(), window()->nextScale());
}

//________________________________________________________________
void Decoration::reconfigure()
{
    m_internalSettings = SettingsProvider::self()->internalSettings();

    // borders
    recalculateBorders();

    // shadow
    updateShadow();

    update();
}

//________________________________________________________________
QMarginsF Decoration::bordersFor(qreal scale) const
{
    const qreal left = isLeftEdge() ? 0 : frameWidth(false, scale);
    const qreal right = isRightEdge() ? 0 : frameWidth(false, scale);
    const qreal bottom = (window()->isShaded() || isBottomEdge()) ? 0 : frameWidth(true, scale);

    // caption buttons plus the frame below them, independent of the buttons' top offset
    qreal top = buttonHeight();
    if (isMaximized() || isTopEdge()) {
        top += 2 * unit();
    } else {
        top += std::max<qreal>(frameWidth(true, scale), 4 * unit()) + Metrics::TitleBar_BottomExtra * unit();
    }

    // make sure the caption always fits
    top = std::max<qreal>(top, settings()->fontMetrics().height() + 6 * unit());

    return QMarginsF(left, KDecoration3::snapToPixelGrid(top, scale), right, bottom);
}

//________________________________________________________________
void Decoration::recalculateBorders()
{
    const qreal scale = window()->nextScale();
    setBorders(bordersFor(scale));

    // extended sizes, so windows without borders can still be resized
    const qreal extSize = KDecoration3::snapToPixelGrid(settings()->largeSpacing(), scale);
    qreal extSides = 0;
    qreal extBottom = 0;
    if (hasNoBorders()) {
        if (!window()->isMaximizedHorizontally()) {
            extSides = extSize;
        }
        if (!window()->isMaximizedVertically()) {
            extBottom = extSize;
        }

    } else if (hasNoSideBorders() && !window()->isMaximizedHorizontally()) {
        extSides = extSize;
    }

    setResizeOnlyBorders(QMarginsF(extSides, 0, extSides, extBottom));

    // square corners
    setBorderRadius(KDecoration3::BorderRadius());

    if (isMaximized() || !m_internalSettings->drawOutline()) {
        setBorderOutline(KDecoration3::BorderOutline());
    } else {
        const qreal thickness = std::max(KDecoration3::pixelSize(scale), KDecoration3::snapToPixelGrid(1, scale));
        setBorderOutline(KDecoration3::BorderOutline(thickness, outlineColor(), KDecoration3::BorderRadius()));
    }

    update();
}

//________________________________________________________________
void Decoration::createButtons()
{
    m_leftButtons = new KDecoration3::DecorationButtonGroup(KDecoration3::DecorationButtonGroup::Position::Left, this, &Button::create);
    m_rightButtons = new KDecoration3::DecorationButtonGroup(KDecoration3::DecorationButtonGroup::Position::Right, this, &Button::create);
    updateButtonsGeometry();
}

//________________________________________________________________
void Decoration::updateButtonsGeometryDelayed()
{
    QTimer::singleShot(0, this, &Decoration::updateButtonsGeometry);
}

//________________________________________________________________
void Decoration::updateButtonsGeometry()
{
    if (!m_leftButtons || !m_rightButtons) {
        return;
    }

    const auto buttonList = m_leftButtons->buttons() + m_rightButtons->buttons();
    for (KDecoration3::DecorationButton *button : buttonList) {
        button->setGeometry(QRectF(QPointF(0, 0), QSizeF(buttonWidth(button->type()), buttonHeight())));
    }

    // caption buttons touch each other
    const qreal y = buttonsTopOffset();

    // left buttons (usually just the window icon)
    if (!m_leftButtons->buttons().isEmpty()) {
        m_leftButtons->setSpacing(0);
        const qreal x = isLeftEdge() ? 2 * unit() : std::max<qreal>(borderLeft(), 2 * unit());
        m_leftButtons->setPos(QPointF(x, y));
    }

    // right buttons: covering the innermost column of the frame (where the inner outline is), or flush with the screen edge when maximized
    if (!m_rightButtons->buttons().isEmpty()) {
        m_rightButtons->setSpacing(0);
        const qreal rightMargin = isRightEdge() ? 0 : std::max<qreal>(0, borderRight() - KDecoration3::snapToPixelGrid(unit(), window()->nextScale()));
        const qreal x = size().width() - m_rightButtons->geometry().width() - rightMargin;
        m_rightButtons->setPos(QPointF(x, y));
    }

    update();
}

//________________________________________________________________
void Decoration::paint(QPainter *painter, const QRectF &repaintRegion)
{
    auto s = settings();
    const QColor frame = frameColor();

    // the whole frame is one flat color
    if (!window()->isShaded()) {
        painter->fillRect(rect(), frame);
    }

    paintTitleBar(painter, repaintRegion);

    // thin line between frame and client area
    if (m_internalSettings->drawOutline() && !isMaximized() && !window()->isShaded()) {
        const qreal px = std::max(KDecoration3::pixelSize(window()->scale()), KDecoration3::snapToPixelGrid(1, window()->scale()));
        const QRectF client(borderLeft(), borderTop(), size().width() - borderLeft() - borderRight(), size().height() - borderTop() - borderBottom());
        const QColor color = innerOutlineColor();

        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(color);
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->drawRect(QRectF(client.left(), client.top() - px, client.width(), px));
        if (borderLeft() > 0) {
            painter->drawRect(QRectF(client.left() - px, client.top() - px, px, client.height() + 2 * px));
        }
        if (borderRight() > 0) {
            painter->drawRect(QRectF(client.right(), client.top() - px, px, client.height() + 2 * px));
        }
        if (borderBottom() > 0) {
            painter->drawRect(QRectF(client.left(), client.bottom(), client.width(), px));
        }
        painter->restore();
    }

    // without compositing KWin cannot draw the outline for us
    if (!s->isAlphaChannelSupported() && !isMaximized()) {
        const QColor color = borderOutline().color();
        if (color.alphaF() > 0) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->setBrush(Qt::NoBrush);
            painter->setPen(color);
            painter->drawRect(rect().adjusted(0, 0, -1, -1));
            painter->restore();
        }
    }
}

//________________________________________________________________
void Decoration::paintTitleBar(QPainter *painter, const QRectF &repaintRegion)
{
    const QRectF titleRect(QPointF(0, 0), QSizeF(size().width(), borderTop()));
    if (!titleRect.intersects(repaintRegion)) {
        return;
    }

    if (window()->isShaded()) {
        painter->fillRect(titleRect, frameColor());
    }

    // draw caption
    painter->setFont(settings()->font());
    painter->setPen(fontColor());
    const auto [captionRectangle, alignment] = captionRect();
    const QString caption = painter->fontMetrics().elidedText(window()->caption(), Qt::ElideRight, captionRectangle.width());
    painter->drawText(captionRectangle, alignment | Qt::TextSingleLine, caption);

    // draw all buttons
    m_leftButtons->paint(painter, repaintRegion);
    m_rightButtons->paint(painter, repaintRegion);
}

//________________________________________________________________
QPair<QRectF, Qt::Alignment> Decoration::captionRect() const
{
    const qreal margin = Metrics::TitleBar_SideMargin * unit();
    const qreal leftOffset = m_leftButtons->buttons().isEmpty() ? std::max<qreal>(borderLeft(), margin) + margin
                                                                : m_leftButtons->geometry().right() + margin;
    const qreal rightOffset = m_rightButtons->buttons().isEmpty() ? std::max<qreal>(borderRight(), margin) + margin
                                                                  : size().width() - m_rightButtons->geometry().left() + margin;

    // the caption is centered over the whole title area, slightly below the buttons' center
    const qreal height = borderTop();
    const QRectF maxRect(leftOffset, 0, size().width() - leftOffset - rightOffset, height);

    switch (m_internalSettings->titleAlignment()) {
    case InternalSettings::AlignLeft:
        return qMakePair(maxRect, Qt::AlignVCenter | Qt::AlignLeft);

    case InternalSettings::AlignRight:
        return qMakePair(maxRect, Qt::AlignVCenter | Qt::AlignRight);

    case InternalSettings::AlignCenter:
        return qMakePair(maxRect, Qt::AlignCenter);

    default:
    case InternalSettings::AlignCenterFullWidth: {
        // full caption rect
        const QRectF fullRect(0, 0, size().width(), height);
        QRectF boundingRect(settings()->fontMetrics().boundingRect(window()->caption()));

        // text bounding rect
        boundingRect.setTop(0);
        boundingRect.setHeight(height);
        boundingRect.moveLeft((size().width() - boundingRect.width()) / 2);

        if (boundingRect.left() < leftOffset) {
            return qMakePair(maxRect, Qt::AlignVCenter | Qt::AlignLeft);
        } else if (boundingRect.right() > size().width() - rightOffset) {
            return qMakePair(maxRect, Qt::AlignVCenter | Qt::AlignRight);
        } else {
            return qMakePair(fullRect, Qt::AlignCenter);
        }
    }
    }
}

//________________________________________________________________
void Decoration::updateShadow()
{
    if (g_shadowSizeEnum != m_internalSettings->shadowSize() || g_shadowStrength != m_internalSettings->shadowStrength()) {
        g_sShadow.reset();
        g_sShadowInactive.reset();
        g_shadowSizeEnum = m_internalSettings->shadowSize();
        g_shadowStrength = m_internalSettings->shadowStrength();
    }

    if (!g_sShadow && !g_sShadowInactive) {
        g_sShadow = createShadowObject(1.0);
        g_sShadowInactive = createShadowObject(0.5);
    }
    setShadow(window()->isActive() ? g_sShadow : g_sShadowInactive);
}

//________________________________________________________________
std::shared_ptr<KDecoration3::DecorationShadow> Decoration::createShadowObject(qreal strengthScale)
{
    const CompositeShadowParams params = lookupShadowParams(m_internalSettings->shadowSize());
    if (params.isNone()) {
        return nullptr;
    }

    auto withOpacity = [](const QColor &color, qreal opacity) -> QColor {
        QColor c(color);
        c.setAlphaF(opacity);
        return c;
    };

    const QSize boxSize =
        BoxShadowRenderer::calculateMinimumBoxSize(params.shadow1.radius).expandedTo(BoxShadowRenderer::calculateMinimumBoxSize(params.shadow2.radius));

    BoxShadowRenderer shadowRenderer;
    shadowRenderer.setBorderRadius(0);
    shadowRenderer.setBoxSize(boxSize);

    const qreal strength = m_internalSettings->shadowStrength() / 255.0 * strengthScale;
    shadowRenderer.addShadow(params.shadow1.offset, params.shadow1.radius, withOpacity(Qt::black, params.shadow1.opacity * strength));
    shadowRenderer.addShadow(params.shadow2.offset, params.shadow2.radius, withOpacity(Qt::black, params.shadow2.opacity * strength));

    QImage shadowTexture = shadowRenderer.render();

    QPainter painter(&shadowTexture);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRectF outerRect = shadowTexture.rect();

    QRectF boxRect(QPoint(0, 0), boxSize);
    boxRect.moveCenter(outerRect.center());

    // Mask out inner rect.
    const QMarginsF padding = QMarginsF(boxRect.left() - outerRect.left() - Metrics::Shadow_Overlap - params.offset.x(),
                                        boxRect.top() - outerRect.top() - Metrics::Shadow_Overlap - params.offset.y(),
                                        outerRect.right() - boxRect.right() - Metrics::Shadow_Overlap + params.offset.x(),
                                        outerRect.bottom() - boxRect.bottom() - Metrics::Shadow_Overlap + params.offset.y());
    QRectF innerRect = outerRect - padding;
    // Push the shadow slightly under the window, which helps avoiding glitches with fractional scaling
    innerRect.adjust(2, 2, -2, -2);

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    painter.drawRect(innerRect);

    painter.end();

    auto ret = std::make_shared<KDecoration3::DecorationShadow>();
    ret->setPadding(padding);
    ret->setInnerShadowRect(QRectF(outerRect.center(), QSizeF(1, 1)));
    ret->setShadow(shadowTexture);
    return ret;
}

} // namespace Pla8ma

#include "pla8madecoration.moc"
