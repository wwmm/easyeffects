configure_file(config.h.in config.h @ONLY)

add_subdirectory(contents)

add_executable(easyeffects)

kde_target_enable_exceptions(easyeffects PRIVATE)

kconfig_add_kcfg_files(easyeffects GENERATE_MOC ${KCFGC_FILES})

# install(FILES ${KCFG_FILES} DESTINATION ${KDE_INSTALL_KCFGDIR})

set_source_files_properties(contents/ui/Validators.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_policy(SET QTP0004 NEW)

qt_add_qml_module(easyeffects
    URI ee.ui
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    SOURCES
        presets_list_model.cpp
        presets_list_model.hpp
        units.hpp
    QML_FILES
        contents/ui/AboutPage.qml
        contents/ui/Autogain.qml
        contents/ui/BassEnhancer.qml
        contents/ui/BassLoudness.qml
        contents/ui/BlocklistDialog.qml
        contents/ui/Compressor.qml
        contents/ui/Convolver.qml
        contents/ui/ConvolverImpulseDialog.qml
        contents/ui/ConvolverSofaDialog.qml
        contents/ui/Crossfeed.qml
        contents/ui/Crusher.qml
        contents/ui/Crystalizer.qml
        contents/ui/CrystalizerBand.qml
        contents/ui/Deesser.qml
        contents/ui/Delay.qml
        contents/ui/DeepFilterNet.qml
        contents/ui/DelegateClientsList.qml
        contents/ui/DelegateModulesList.qml
        contents/ui/DelegatePluginsList.qml
        contents/ui/DelegateStreamsList.qml
        contents/ui/EchoCanceller.qml
        contents/ui/EeCard.qml
        contents/ui/EeChart.qml
        contents/ui/EeAudioLevel.qml
        contents/ui/EeInputOutputGain.qml
        contents/ui/EeListSectionHeader.qml
        contents/ui/EeProgressBar.qml
        contents/ui/EeSpinBox.qml
        contents/ui/EeSwitch.qml
        contents/ui/Equalizer.qml
        contents/ui/EqualizerBand.qml
        contents/ui/EqualizerBandMenu.qml
        contents/ui/Exciter.qml
        contents/ui/Expander.qml
        contents/ui/Filter.qml
        contents/ui/Gate.qml
        contents/ui/LevelMeter.qml
        contents/ui/Limiter.qml
        contents/ui/Loudness.qml
        contents/ui/Maximizer.qml
        contents/ui/MultibandCompressor.qml
        contents/ui/MultibandGate.qml
        contents/ui/Main.qml
        contents/ui/MenuAddPlugins.qml
        contents/ui/PageStreamsEffects.qml
        contents/ui/Pitch.qml
        contents/ui/PipeWirePage.qml
        contents/ui/PluginNotAvailable.qml
        contents/ui/PreferencesSheet.qml
        contents/ui/LspPreMixDialog.qml
        contents/ui/PresetsDialog.qml
        contents/ui/PresetsAutoloadPage.qml
        contents/ui/PresetsLocalPage.qml
        contents/ui/PresetsCommunityPage.qml
        contents/ui/Reverb.qml
        contents/ui/RNNoise.qml
        contents/ui/ShortcutsSheet.qml
        contents/ui/Speex.qml
        contents/ui/StereoTools.qml
        contents/ui/Validators.qml
        contents/ui/Common.js
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

qt_add_qml_module(ee_autostart
    URI ee.autostart
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
    convolver_kernel_fft.cpp
    convolver_kernel_manager.cpp
    convolver_preset.cpp
    convolver_zita.cpp
    crossfeed.cpp
    crossfeed_preset.cpp
    crusher.cpp
    crusher_preset.cpp
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
    kcolor_manager.cpp
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
    pw_device_manager.cpp
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
)

target_include_directories(easyeffects SYSTEM PRIVATE
    ${LIBZITACONVOLVER_INCLUDE_DIRS}
)

target_link_libraries(easyeffects PRIVATE
    KF6::ColorScheme
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::CoreAddons
    KF6::I18n
    KF6::IconThemes
    Qt::Core
    Qt::DBus
    Qt::Gui
    Qt::Graphs
    Qt::Network
    Qt::Qml
    Qt::Quick
    Qt::QuickControls2
    Qt::Widgets
    TBB::tbb
    GSL::gsl
    GSL::gslcblas
    #SoundTouch::SoundTouch # As of SoundTouch 2.4.0 its cmake files are bugged
    PkgConfig::LIBPIPEWIRE
    PkgConfig::LIBLILV
    PkgConfig::LIBEBUR128
    PkgConfig::LIBFFTW3
    PkgConfig::LIBFFTW3f
    PkgConfig::LIBSPEEXDSP
    PkgConfig::LIBNLOHMANNJSON
    PkgConfig::LIBGSL
    PkgConfig::LIBBS2B
    PkgConfig::LIBSNDFILE
    PkgConfig::LIBSOUNDTOUCH
    PkgConfig::LIBWEBRTC
    PkgConfig::LIBMYSOFA
    ${LIBZITACONVOLVER}
)

target_compile_definitions(easyeffects PRIVATE QT_NO_KEYWORDS=1)
# target_compile_definitions(easyeffects PRIVATE QT_NO_KEYWORDS=1 QT_QML_DEBUG=1)

if(ENABLE_SANITIZER)
    MESSAGE(STATUS "Enabling the compiler's sanitizer")
    target_compile_options(easyeffects PRIVATE "-fsanitize=address")
    target_link_options(easyeffects PRIVATE "-fsanitize=address")
endif(ENABLE_SANITIZER)

if(ENABLE_RNNOISE)
    MESSAGE(STATUS "Enabling rnnoise")
    target_compile_definitions(easyeffects PRIVATE ENABLE_RNNOISE=1)
    target_link_libraries(easyeffects PRIVATE PkgConfig::LIBRNNOISE)
endif(ENABLE_RNNOISE)

if(ENABLE_LIBPORTAL)
    MESSAGE(STATUS "Flatpak build")
    target_compile_definitions(easyeffects PRIVATE ENABLE_LIBPORTAL=1)
    target_link_libraries(easyeffects PRIVATE PkgConfig::LIBPORTAL PkgConfig::LIBPORTALQT)
endif(ENABLE_LIBPORTAL)

if(ENABLE_LIBCPP_WORKAROUNDS)
    MESSAGE(STATUS "Enabling workarounds for lib++ systems")
    target_compile_definitions(easyeffects PRIVATE ENABLE_LIBCPP_WORKAROUNDS=1)
endif(ENABLE_LIBCPP_WORKAROUNDS)

install(TARGETS easyeffects ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
