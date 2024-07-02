configure_file(config.h.in config.h)

add_subdirectory(contents)

add_executable(easyeffects)

kde_target_enable_exceptions(easyeffects PRIVATE)

target_sources(easyeffects PRIVATE
    main.cpp
    util.cpp
    resources.qrc
)

target_link_libraries(easyeffects PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Quick
    Qt${QT_MAJOR_VERSION}::Qml
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::QuickControls2
    Qt${QT_MAJOR_VERSION}::Widgets
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::CoreAddons
    KF${QT_MAJOR_VERSION}::ConfigCore
    KF${QT_MAJOR_VERSION}::ConfigGui
)

kconfig_add_kcfg_files(easyeffects GENERATE_MOC ${KCFGC_FILES})

# install(FILES ${KCFG_FILES} DESTINATION ${KDE_INSTALL_KCFGDIR})

install(TARGETS easyeffects ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})