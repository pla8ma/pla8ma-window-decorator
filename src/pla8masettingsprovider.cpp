/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "pla8masettingsprovider.h"

namespace Pla8ma
{
SettingsProvider *SettingsProvider::s_self = nullptr;

//__________________________________________________________________
SettingsProvider::SettingsProvider()
    : m_settings(new InternalSettings())
{
    reconfigure();
}

//__________________________________________________________________
SettingsProvider::~SettingsProvider()
{
    s_self = nullptr;
}

//__________________________________________________________________
SettingsProvider *SettingsProvider::self()
{
    if (!s_self) {
        s_self = new SettingsProvider();
    }
    return s_self;
}

//__________________________________________________________________
void SettingsProvider::reconfigure()
{
    // load() re-parses the file, the KCM writes it from another process
    m_settings->load();
}

} // namespace Pla8ma
