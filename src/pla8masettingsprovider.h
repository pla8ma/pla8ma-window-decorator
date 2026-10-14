/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "pla8ma.h"

#include <QObject>

namespace Pla8ma
{
//* shared settings for all decorations (one config file read per reconfigure)
class SettingsProvider : public QObject
{
    Q_OBJECT

public:
    ~SettingsProvider() override;

    //* singleton
    static SettingsProvider *self();

    InternalSettingsPtr internalSettings() const
    {
        return m_settings;
    }

public Q_SLOTS:
    void reconfigure();

private:
    SettingsProvider();

    InternalSettingsPtr m_settings;

    static SettingsProvider *s_self;
};

} // namespace Pla8ma
