Name:           pla8ma-window-decorator
Version:        1.0.0
Release:        1%{?dist}
Summary:        Flat KWin window decoration that follows the Plasma color scheme

# Code derived from Breeze: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
# Shadow renderer: GPL-2.0-or-later, settings module: MIT
License:        (GPL-2.0-only OR GPL-3.0-only) AND GPL-2.0-or-later AND MIT
URL:            https://github.com/pla8ma/pla8ma-window-decorator
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  extra-cmake-modules
BuildRequires:  gcc-c++
BuildRequires:  gettext
BuildRequires:  kf6-rpm-macros

BuildRequires:  cmake(Qt6Core)
BuildRequires:  cmake(Qt6DBus)
BuildRequires:  cmake(Qt6Gui)
BuildRequires:  cmake(Qt6Widgets)

BuildRequires:  cmake(KF6ColorScheme)
BuildRequires:  cmake(KF6Config)
BuildRequires:  cmake(KF6CoreAddons)
BuildRequires:  cmake(KF6GuiAddons)
BuildRequires:  cmake(KF6I18n)
BuildRequires:  cmake(KF6KCMUtils)
BuildRequires:  cmake(KF6WidgetsAddons)

BuildRequires:  cmake(KDecoration3)

# the plugin is loaded by KWin, rebuild it for every Plasma feature release
Requires:       kwin

%description
Pla8ma Window Decorator is a native KDecoration3 window decoration for KWin
(Plasma 6). It draws a flat, single colored frame with square corners and
caption buttons attached to the top edge.

By default the frame uses the accent color of the active KDE color scheme and
changes along with it. The frame color, intensity, inactive color, title text
color, title alignment, button size and shadow can be changed in System
Settings.


%prep
%autosetup -p1


%build
%cmake_kf6
%cmake_build


%install
%cmake_install
%find_lang pla8ma_kwin_deco


%files -f pla8ma_kwin_deco.lang
%license LICENSES/*
%doc README.md
%{_kf6_qtplugindir}/org.kde.kdecoration3/pla8madecoration.so
%{_kf6_qtplugindir}/org.kde.kdecoration3.kcm/kcm_pla8madecoration.so


%changelog
* Wed Oct 14 2026 pla8ma - 1.0.0-1
- Initial package
