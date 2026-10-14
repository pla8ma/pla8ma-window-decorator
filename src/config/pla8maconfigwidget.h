/*
 * SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "pla8masettings.h"

#include <KCModule>

#include <QSharedPointer>

class KColorButton;
class QCheckBox;
class QComboBox;
class QSlider;
class QLabel;

namespace Pla8ma
{
class ConfigWidget : public KCModule
{
    Q_OBJECT

public:
    explicit ConfigWidget(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
    ~ConfigWidget() override = default;

    void defaults() override;
    void load() override;
    void save() override;

private Q_SLOTS:
    void updateChanged();
    void updateEnabledState();

private:
    void setupUi();

    //* write the given settings into the ui
    void assign(InternalSettings *settings);

    QSharedPointer<InternalSettings> m_internalSettings;

    QComboBox *m_colorMode = nullptr;
    KColorButton *m_customColor = nullptr;
    QSlider *m_colorIntensity = nullptr;
    QLabel *m_colorIntensityLabel = nullptr;
    QComboBox *m_inactiveColorMode = nullptr;
    KColorButton *m_inactiveCustomColor = nullptr;
    QComboBox *m_textColorMode = nullptr;
    KColorButton *m_customTextColor = nullptr;
    QComboBox *m_titleAlignment = nullptr;
    QComboBox *m_buttonSize = nullptr;
    QCheckBox *m_redCloseButton = nullptr;
    QCheckBox *m_drawOutline = nullptr;
    QComboBox *m_shadowSize = nullptr;
    QSlider *m_shadowStrength = nullptr;
};

} // namespace Pla8ma
