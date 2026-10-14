add_definitions(-DTRANSLATION_DOMAIN="pla8ma_kwin_deco")

################# shared code #################
add_library(pla8macommon STATIC
    pla8maboxshadowrenderer.cpp
)
set_target_properties(pla8macommon PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_link_libraries(pla8macommon PUBLIC Qt6::Core Qt6::Gui)

################# decoration plugin #################
set(pla8madecoration_SRCS
    pla8mabutton.cpp
    pla8madecoration.cpp
    pla8masettingsprovider.cpp
)

kconfig_add_kcfg_files(pla8madecoration_SRCS pla8masettings.kcfgc)
add_library(pla8madecoration MODULE ${pla8madecoration_SRCS})

set_target_properties(pla8madecoration PROPERTIES
    OUTPUT_NAME pla8madecoration
)

target_include_directories(pla8madecoration PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(pla8madecoration
    PRIVATE
        pla8macommon
        Qt6::DBus
        KF6::ColorScheme
        KF6::ConfigGui
        KF6::CoreAddons
        KF6::GuiAddons
        KF6::I18n
        KDecoration3::KDecoration
)

install(TARGETS pla8madecoration DESTINATION ${KDE_INSTALL_PLUGINDIR}/${KDECORATION_PLUGIN_DIR})

add_subdirectory(config)
