configure_file(config.h.in config.h)

add_subdirectory(contents)

add_executable(easyeffects)

kde_target_enable_exceptions(easyeffects PRIVATE)

qt_add_qml_module(easyeffects 
    URI "TagsPluginName" 
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    NO_RESOURCE_TARGET_PATH
    SOURCES
        tags_plugin_name.hpp
)

target_sources(easyeffects PRIVATE
    main.cpp
    pipe_manager.cpp
    tags_plugin_name.cpp
    util.cpp
    resources.qrc
)

target_include_directories(easyeffects PRIVATE 
    ${LIBPIPEWIRE_INCLUDE_DIRS}
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
    Qt${QT_MAJOR_VERSION}::Charts
    ${LIBPIPEWIRE_LIBRARIES}
)

if(ENABLE_LIBCPP_WORKAROUNDS)
    MESSAGE(STATUS "Enabling workarounds for lib++ systems")
    target_compile_definitions(easyeffects PRIVATE ENABLE_LIBCPP_WORKAROUNDS=1)
endif(ENABLE_LIBCPP_WORKAROUNDS)

kconfig_add_kcfg_files(easyeffects GENERATE_MOC ${KCFGC_FILES})

# install(FILES ${KCFG_FILES} DESTINATION ${KDE_INSTALL_KCFGDIR})

install(TARGETS easyeffects ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})