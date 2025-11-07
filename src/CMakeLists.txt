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

# The module definitions below are to help the qml linter

qt_add_qml_module(ee_tags_plugin_name
    URI ee.tags.plugin.name
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

qt_add_qml_module(ee_pipewire
    URI ee.pipewire
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

qt_add_qml_module(ee_database
    URI ee.database
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

qt_add_qml_module(ee_presets
    URI ee.presets
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

qt_add_qml_module(ee_pipeline
    URI ee.pipeline
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

target_sources(easyeffects PRIVATE
    autogain.cpp
    autogain_preset.cpp
    autostart.cpp
    bass_enhancer.cpp
    bass_enhancer_preset.cpp
    bass_loudness.cpp
    bass_loudness_preset.cpp
    command_line_parser.cpp
    compressor.cpp
    compressor_preset.cpp
    convolver.cpp
    convolver_preset.cpp
    crossfeed.cpp
    crossfeed_preset.cpp
    crystalizer.cpp
    crystalizer_preset.cpp
    db_manager.cpp
    delay.cpp
    delay_preset.cpp
    deepfilternet.cpp
    deepfilternet_preset.cpp
    deesser.cpp
    deesser_preset.cpp
    echo_canceller.cpp
    echo_canceller_preset.cpp
    effects_base.cpp
    equalizer.cpp
    equalizer_apo.cpp
    equalizer_preset.cpp
    exciter.cpp
    exciter_preset.cpp
    expander.cpp
    expander_preset.cpp
    filter.cpp
    filter_preset.cpp
    fir_filter_bandpass.cpp
    fir_filter_base.cpp
    fir_filter_highpass.cpp
    fir_filter_lowpass.cpp
    gate.cpp
    gate_preset.cpp
    global_shortcuts.cpp
    kconfig_base_ee.cpp
    ladspa_wrapper.cpp
    level_meter.cpp
    level_meter_preset.cpp
    limiter.cpp
    limiter_preset.cpp
    local_client.cpp
    local_server.cpp
    loudness.cpp
    loudness_preset.cpp
    lv2_ui.cpp
    lv2_wrapper.cpp
    main.cpp
    maximizer.cpp
    maximizer_preset.cpp
    multiband_compressor.cpp
    multiband_compressor_preset.cpp
    multiband_gate.cpp
    multiband_gate_preset.cpp
    output_level.cpp
    pitch.cpp
    pitch_preset.cpp
    plugin_base.cpp
    plugin_preset_base.cpp
    presets_autoload_manager.cpp
    presets_community_manager.cpp
    presets_directory_manager.cpp
    presets_irs_manager.cpp
    presets_rnnoise_manager.cpp
    presets_manager.cpp
    presets_list_model.cpp
    pw_manager.cpp
    pw_client_manager.cpp
    pw_link_manager.cpp
    pw_metadata_manager.cpp
    pw_module_manager.cpp
    pw_node_manager.cpp
    pw_model_clients.cpp
    pw_model_modules.cpp
    pw_model_nodes.cpp
    resampler.cpp
    reverb.cpp
    reverb_preset.cpp
    rnnoise.cpp
    rnnoise_preset.cpp
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

target_include_directories(easyeffects SYSTEM PRIVATE
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
    ${LIBSAMPLERATE_INCLUDE_DIRS}
    ${LIBSNDFILE_INCLUDE_DIRS}
    ${LIBRNNOISE_INCLUDE_DIRS}
    ${LIBSOUNDTOUCH_INCLUDE_DIRS}
    ${LIBPORTAL_INCLUDE_DIRS}
    ${LIBPORTALQT_INCLUDE_DIRS}
    ${LIBWEBRTC_INCLUDE_DIRS}
)

target_link_libraries(easyeffects PRIVATE
    KF${QT_MAJOR_VERSION}::ConfigCore
    KF${QT_MAJOR_VERSION}::ConfigGui
    KF${QT_MAJOR_VERSION}::CoreAddons
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::IconThemes
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::DBus
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Graphs
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::Qml
    Qt${QT_MAJOR_VERSION}::Quick
    Qt${QT_MAJOR_VERSION}::QuickControls2
    Qt${QT_MAJOR_VERSION}::Widgets
    Qt${QT_MAJOR_VERSION}::WebEngineQuick
    TBB::tbb
    GSL::gsl
    GSL::gslcblas
    #SoundTouch::SoundTouch # As of SoundTouch 2.4.0 its cmake files are bugged
    ${LIBPIPEWIRE_LIBRARIES}
    ${LIBLILV_LIBRARIES}
    ${LIBEBUR128_LIBRARIES}
    ${LIBFFTW3_LIBRARIES}
    ${LIBFFTW3f_LIBRARIES}
    ${LIBSPEEXDSP_LIBRARIES}
    ${LIBNLOHMANNJSON_LIBRARIES}
    ${LIBGSL_LIBRARIES}
    ${LIBBS2B_LIBRARIES}
    ${LIBSAMPLERATE_LIBRARIES}
    ${LIBSNDFILE_LIBRARIES}
    ${LIBRNNOISE_LIBRARIES}
    ${LIBSOUNDTOUCH_LIBRARIES}
    ${LIBPORTAL_LIBRARIES}
    ${LIBPORTALQT_LIBRARIES}
    ${LIBWEBRTC_LIBRARIES}
    ${LIBZITACONVOLVER}
)

target_compile_definitions(easyeffects PRIVATE QT_NO_KEYWORDS=1)
# target_compile_definitions(easyeffects PRIVATE QT_NO_KEYWORDS=1 QT_QML_DEBUG=1)

if(ENABLE_SANATIZER)
    MESSAGE(STATUS "Enabling the compiler's sanatizer")
    target_compile_options(easyeffects PRIVATE "-fsanitize=address")
    target_link_options(easyeffects PRIVATE "-fsanitize=address")
endif(ENABLE_SANATIZER)

if(ENABLE_MOLD)
    MESSAGE(STATUS "Enabling mold")
    target_compile_definitions(easyeffects PRIVATE ENABLE_MOLD=1)
endif(ENABLE_MOLD)

if(ENABLE_RNNOISE)
    MESSAGE(STATUS "Enabling rnnoise")
    target_compile_definitions(easyeffects PRIVATE ENABLE_RNNOISE=1)
endif(ENABLE_RNNOISE)

if(ENABLE_LIBCPP_WORKAROUNDS)
    MESSAGE(STATUS "Enabling workarounds for lib++ systems")
    target_compile_definitions(easyeffects PRIVATE ENABLE_LIBCPP_WORKAROUNDS=1)
endif(ENABLE_LIBCPP_WORKAROUNDS)

install(TARGETS easyeffects ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
