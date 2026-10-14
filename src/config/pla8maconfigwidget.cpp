/*
 * SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: MIT
 */

#include "pla8maconfigwidget.h"

#include <KColorButton>
#include <KLocalizedString>

#include <QCheckBox>
#include <QComboBox>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>

namespace Pla8ma
{
//_________________________________________________________
ConfigWidget::ConfigWidget(QObject *parent, const KPluginMetaData &data, const QVariantList & /*args*/)
    : KCModule(parent, data)
{
    setupUi();

    // track ui changes
    for (QComboBox *combo : {m_colorMode, m_inactiveColorMode, m_textColorMode, m_titleAlignment, m_buttonSize, m_shadowSize}) {
        connect(combo, &QComboBox::currentIndexChanged, this, &ConfigWidget::updateChanged);
        connect(combo, &QComboBox::currentIndexChanged, this, &ConfigWidget::updateEnabledState);
    }
    for (KColorButton *button : {m_customColor, m_inactiveCustomColor, m_customTextColor}) {
        connect(button, &KColorButton::changed, this, &ConfigWidget::updateChanged);
    }
    for (QCheckBox *box : {m_redCloseButton, m_drawOutline}) {
        connect(box, &QAbstractButton::toggled, this, &ConfigWidget::updateChanged);
    }
    for (QSlider *slider : {m_colorIntensity, m_shadowStrength}) {
        connect(slider, &QSlider::valueChanged, this, &ConfigWidget::updateChanged);
    }
    connect(m_colorIntensity, &QSlider::valueChanged, this, [this](int value) {
        m_colorIntensityLabel->setText(i18nc("percentage", "%1 %", value));
    });
}

//_________________________________________________________
void ConfigWidget::setupUi()
{
    auto layout = new QFormLayout(widget());

    // frame color
    m_colorMode = new QComboBox(widget());
    m_colorMode->addItems({i18n("Accent color of the color scheme"), i18n("Title bar color of the color scheme"), i18n("Custom color")});
    layout->addRow(i18n("Frame color:"), m_colorMode);

    m_customColor = new KColorButton(widget());
    layout->addRow(i18n("Custom color:"), m_customColor);

    auto intensityLayout = new QHBoxLayout();
    m_colorIntensity = new QSlider(Qt::Horizontal, widget());
    m_colorIntensity->setRange(0, 100);
    m_colorIntensityLabel = new QLabel(widget());
    m_colorIntensityLabel->setMinimumWidth(m_colorIntensityLabel->fontMetrics().horizontalAdvance(QStringLiteral("100 %")));
    intensityLayout->addWidget(m_colorIntensity);
    intensityLayout->addWidget(m_colorIntensityLabel);
    layout->addRow(i18n("Color intensity:"), intensityLayout);

    // inactive windows
    m_inactiveColorMode = new QComboBox(widget());
    m_inactiveColorMode->addItems({i18n("Faded frame color"),
                                   i18n("Same as active window"),
                                   i18n("Inactive title bar color of the color scheme"),
                                   i18n("Custom color")});
    layout->addRow(i18n("Inactive windows:"), m_inactiveColorMode);

    m_inactiveCustomColor = new KColorButton(widget());
    layout->addRow(i18n("Custom inactive color:"), m_inactiveCustomColor);

    // caption
    m_textColorMode = new QComboBox(widget());
    m_textColorMode->addItems({i18n("Automatic (black or white, whichever has more contrast)"), i18n("Text color of the color scheme"), i18n("Custom color")});
    layout->addRow(i18n("Title text:"), m_textColorMode);

    m_customTextColor = new KColorButton(widget());
    layout->addRow(i18n("Custom text color:"), m_customTextColor);

    m_titleAlignment = new QComboBox(widget());
    m_titleAlignment->addItems({i18n("Left"), i18n("Center"), i18n("Center (full width)"), i18n("Right")});
    layout->addRow(i18n("Title alignment:"), m_titleAlignment);

    // buttons
    m_buttonSize = new QComboBox(widget());
    m_buttonSize->addItems({i18nc("@item:inlistbox button size", "Small"), i18nc("@item:inlistbox button size", "Normal"), i18nc("@item:inlistbox button size", "Large")});
    layout->addRow(i18n("Button size:"), m_buttonSize);

    m_redCloseButton = new QCheckBox(i18n("Always draw the close button of active windows in red"), widget());
    layout->addRow(QString(), m_redCloseButton);

    m_drawOutline = new QCheckBox(i18n("Draw a thin outline"), widget());
    layout->addRow(QString(), m_drawOutline);

    // shadow
    m_shadowSize = new QComboBox(widget());
    m_shadowSize->addItems({i18nc("@item:inlistbox shadow size", "None"), i18nc("@item:inlistbox shadow size", "Small"), i18nc("@item:inlistbox shadow size", "Medium"), i18nc("@item:inlistbox shadow size", "Large")});
    layout->addRow(i18n("Shadow:"), m_shadowSize);

    m_shadowStrength = new QSlider(Qt::Horizontal, widget());
    m_shadowStrength->setRange(25, 255);
    layout->addRow(i18n("Shadow strength:"), m_shadowStrength);
}

//_________________________________________________________
void ConfigWidget::assign(InternalSettings *settings)
{
    m_colorMode->setCurrentIndex(settings->colorMode());
    m_customColor->setColor(settings->customColor());
    m_colorIntensity->setValue(settings->colorIntensity());
    m_colorIntensityLabel->setText(i18nc("percentage", "%1 %", settings->colorIntensity()));
    m_inactiveColorMode->setCurrentIndex(settings->inactiveColorMode());
    m_inactiveCustomColor->setColor(settings->inactiveCustomColor());
    m_textColorMode->setCurrentIndex(settings->textColorMode());
    m_customTextColor->setColor(settings->customTextColor());
    m_titleAlignment->setCurrentIndex(settings->titleAlignment());
    m_buttonSize->setCurrentIndex(settings->buttonSize());
    m_redCloseButton->setChecked(settings->redCloseButton());
    m_drawOutline->setChecked(settings->drawOutline());
    m_shadowSize->setCurrentIndex(settings->shadowSize());
    m_shadowStrength->setValue(settings->shadowStrength());
    updateEnabledState();
}

//_________________________________________________________
void ConfigWidget::load()
{
    m_internalSettings = QSharedPointer<InternalSettings>(new InternalSettings());
    m_internalSettings->load();
    assign(m_internalSettings.data());
    setNeedsSave(false);
}

//_________________________________________________________
void ConfigWidget::save()
{
    if (!m_internalSettings) {
        return;
    }

    m_internalSettings->setColorMode(m_colorMode->currentIndex());
    m_internalSettings->setCustomColor(m_customColor->color());
    m_internalSettings->setColorIntensity(m_colorIntensity->value());
    m_internalSettings->setInactiveColorMode(m_inactiveColorMode->currentIndex());
    m_internalSettings->setInactiveCustomColor(m_inactiveCustomColor->color());
    m_internalSettings->setTextColorMode(m_textColorMode->currentIndex());
    m_internalSettings->setCustomTextColor(m_customTextColor->color());
    m_internalSettings->setTitleAlignment(m_titleAlignment->currentIndex());
    m_internalSettings->setButtonSize(m_buttonSize->currentIndex());
    m_internalSettings->setRedCloseButton(m_redCloseButton->isChecked());
    m_internalSettings->setDrawOutline(m_drawOutline->isChecked());
    m_internalSettings->setShadowSize(m_shadowSize->currentIndex());
    m_internalSettings->setShadowStrength(m_shadowStrength->value());
    m_internalSettings->save();

    setNeedsSave(false);

    // tell KWin to reload the decoration settings
    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/KWin"), QStringLiteral("org.kde.KWin"), QStringLiteral("reloadConfig"));
    QDBusConnection::sessionBus().send(message);
}

//_________________________________________________________
void ConfigWidget::defaults()
{
    InternalSettings defaults;
    defaults.setDefaults();
    assign(&defaults);
    updateChanged();
}

//_________________________________________________________
void ConfigWidget::updateEnabledState()
{
    m_customColor->setEnabled(m_colorMode->currentIndex() == InternalSettings::ColorCustom);
    m_inactiveCustomColor->setEnabled(m_inactiveColorMode->currentIndex() == InternalSettings::InactiveCustom);
    m_customTextColor->setEnabled(m_textColorMode->currentIndex() == InternalSettings::TextCustom);
    m_shadowStrength->setEnabled(m_shadowSize->currentIndex() != InternalSettings::ShadowNone);
}

//_________________________________________________________
void ConfigWidget::updateChanged()
{
    if (!m_internalSettings) {
        return;
    }

    const bool modified = m_colorMode->currentIndex() != m_internalSettings->colorMode() || m_customColor->color() != m_internalSettings->customColor()
        || m_colorIntensity->value() != m_internalSettings->colorIntensity() || m_inactiveColorMode->currentIndex() != m_internalSettings->inactiveColorMode()
        || m_inactiveCustomColor->color() != m_internalSettings->inactiveCustomColor() || m_textColorMode->currentIndex() != m_internalSettings->textColorMode()
        || m_customTextColor->color() != m_internalSettings->customTextColor() || m_titleAlignment->currentIndex() != m_internalSettings->titleAlignment()
        || m_buttonSize->currentIndex() != m_internalSettings->buttonSize() || m_redCloseButton->isChecked() != m_internalSettings->redCloseButton()
        || m_drawOutline->isChecked() != m_internalSettings->drawOutline() || m_shadowSize->currentIndex() != m_internalSettings->shadowSize()
        || m_shadowStrength->value() != m_internalSettings->shadowStrength();

    setNeedsSave(modified);
}

} // namespace Pla8ma
