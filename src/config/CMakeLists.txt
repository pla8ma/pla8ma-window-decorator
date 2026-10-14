kcoreaddons_add_plugin(kcm_pla8madecoration
    SOURCES
        kcm_pla8madecoration.cpp
        pla8maconfigwidget.cpp
    INSTALL_NAMESPACE "${KDECORATION_KCM_PLUGIN_DIR}"
)
kconfig_add_kcfg_files(kcm_pla8madecoration ../pla8masettings.kcfgc)
target_include_directories(kcm_pla8madecoration PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(kcm_pla8madecoration
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::DBus
        KF6::ConfigCore
        KF6::ConfigGui
        KF6::CoreAddons
        KF6::I18n
        KF6::KCMUtils
        KF6::WidgetsAddons
)
