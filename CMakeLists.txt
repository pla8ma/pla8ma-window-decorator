cmake_minimum_required(VERSION 3.16)
project(pla8madecoration VERSION 1.0.0)

set(QT_MIN_VERSION "6.10.0")
set(KF6_MIN_VERSION "6.24.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

set(QT_MAJOR_VERSION 6)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS Core Gui Widgets DBus)
find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    ColorScheme
    Config
    CoreAddons
    GuiAddons
    I18n
    KCMUtils
    WidgetsAddons
)
find_package(KDecoration3 REQUIRED)

add_subdirectory(src)

ki18n_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
