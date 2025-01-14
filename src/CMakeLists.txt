configure_file(config.h.in config.h @ONLY)

add_subdirectory(contents)

add_executable(easyeffects)

kde_target_enable_exceptions(easyeffects PRIVATE)

kconfig_add_kcfg_files(easyeffects GENERATE_MOC ${KCFGC_FILES})

# install(FILES ${KCFG_FILES} DESTINATION ${KDE_INSTALL_KCFGDIR})

qt_add_qml_module(easyeffects
    URI ee.type.presets
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    SOURCES
        presets_list_model.cpp presets_list_model.hpp
)

target_sources(easyeffects PRIVATE
    autogain.cpp
    autogain_preset.cpp
    bass_enhancer.cpp
    bass_enhancer_preset.cpp
    command_line_parser.cpp
    compressor.cpp
    crossfeed.cpp
    crystalizer.cpp
    crystalizer_preset.cpp
    db_manager.cpp
    effects_base.cpp
    exciter.cpp
    exciter_preset.cpp
    filter.cpp
    fir_filter_bandpass.cpp
    fir_filter_base.cpp
    fir_filter_highpass.cpp
    fir_filter_lowpass.cpp
    gate.cpp
    kconfig_base_ee.cpp
    limiter.cpp
    limiter_preset.cpp
    local_client.cpp
    local_server.cpp
    lv2_wrapper.cpp
    main.cpp
    maximizer.cpp
    maximizer_preset.cpp
    output_level.cpp
    plugin_base.cpp
    plugin_preset_base.cpp
    presets_manager.cpp
    presets_list_model.cpp
    pw_manager.cpp
    pw_model_clients.cpp
    pw_model_modules.cpp
    pw_model_nodes.cpp
    spectrum.cpp
    speex.cpp
    speex_preset.cpp
    stereo_tools.cpp
    stereo_tools_preset.cpp
    stream_input_effects.cpp
    stream_output_effects.cpp
    tags_plugin_name.cpp
    test_signals.cpp
    util.cpp
    resources.qrc
)

target_include_directories(easyeffects PRIVATE
    ${LIBPIPEWIRE_INCLUDE_DIRS}
    ${LIBLILV_INCLUDE_DIRS}
    ${LIBEBUR128_INCLUDE_DIRS}
    ${LIBFFTW3_INCLUDE_DIRS}
    ${LIBFFTW3f_INCLUDE_DIRS}
    ${LIBSPEEXDSP_INCLUDE_DIRS}
    ${LIBNLOHMANNJSON_INCLUDE_DIRS}
    ${LIBZITACONVOLVER_INCLUDE_DIRS}
    ${LIBGSL_INCLUDE_DIRS}
    ${LIBBS2B_INCLUDE_DIRS}
)

target_link_libraries(easyeffects PRIVATE
    Qt${QT_MAJOR_VERSION}::Graphs
    KF${QT_MAJOR_VERSION}::ConfigCore
    KF${QT_MAJOR_VERSION}::ConfigGui
    Qt${QT_MAJOR_VERSION}::Core
    KF${QT_MAJOR_VERSION}::CoreAddons
    Qt${QT_MAJOR_VERSION}::Gui
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::IconThemes
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::Qml
    Qt${QT_MAJOR_VERSION}::Quick
    Qt${QT_MAJOR_VERSION}::QuickControls2
    Qt${QT_MAJOR_VERSION}::Widgets
    ${LIBPIPEWIRE_LIBRARIES}
    ${LIBLILV_LIBRARIES}
    ${LIBEBUR128_LIBRARIES}
    ${LIBFFTW3_LIBRARIES}
    ${LIBFFTW3f_LIBRARIES}
    ${LIBSPEEXDSP_LIBRARIES}
    ${LIBNLOHMANNJSON_LIBRARIES}
    ${LIBGSL_LIBRARIES}
    ${LIBBS2B_LIBRARIES}
    ${LIBZITACONVOLVER}
)

if(ENABLE_LIBPORTAL)
    MESSAGE(STATUS "Enabling libportal")
    target_compile_definitions(easyeffects PRIVATE ENABLE_LIBPORTAL=1)
endif(ENABLE_LIBPORTAL)

if(ENABLE_MOLD)
    MESSAGE(STATUS "Enabling mold")
    target_compile_definitions(easyeffects PRIVATE ENABLE_MOLD=1)
endif(ENABLE_MOLD)

if(ENABLE_RNNOISE)
    MESSAGE(STATUS "Enabling rnnoise")
    target_compile_definitions(easyeffects PRIVATE ENABLE_RNNOISE=1)
endif(ENABLE_RNNOISE)

if(ENABLE_DEVEL)
    MESSAGE(STATUS "Enabling devel")
    target_compile_definitions(easyeffects PRIVATE ENABLE_DEVEL=1)
endif(ENABLE_DEVEL)

if(ENABLE_LIBCPP_WORKAROUNDS)
    MESSAGE(STATUS "Enabling workarounds for lib++ systems")
    target_compile_definitions(easyeffects PRIVATE ENABLE_LIBCPP_WORKAROUNDS=1)
endif(ENABLE_LIBCPP_WORKAROUNDS)

install(TARGETS easyeffects ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
