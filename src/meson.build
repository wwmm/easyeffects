easyeffects_sources = [
	'easyeffects.cpp',
	'application.cpp',
	'application_ui.cpp',
	'apps_box.cpp',
	'app_info.cpp',
	'autogain.cpp',
	'autogain_preset.cpp',
	'autogain_ui.cpp',
	'bass_enhancer.cpp',
	'bass_enhancer_preset.cpp',
	'bass_enhancer_ui.cpp',
	'bass_loudness.cpp',
	'bass_loudness_preset.cpp',
	'bass_loudness_ui.cpp',
	'blocklist_menu.cpp',
	'chart.cpp',
	'client_info_holder.cpp',
	'compressor.cpp',
	'compressor_preset.cpp',
	'compressor_ui.cpp',
	'convolver.cpp',
	'convolver_menu_impulses.cpp',
	'convolver_menu_combine.cpp',
	'convolver_preset.cpp',
	'convolver_ui.cpp',
	'convolver_ui_common.cpp',
	'crossfeed.cpp',
	'crossfeed_preset.cpp',
	'crossfeed_ui.cpp',
	'crystalizer.cpp',
	'crystalizer_preset.cpp',
	'crystalizer_ui.cpp',
	'deesser.cpp',
	'deesser_preset.cpp',
	'deesser_ui.cpp',
	'delay.cpp',
	'delay_preset.cpp',
	'delay_ui.cpp',
	'echo_canceller.cpp',
	'echo_canceller_preset.cpp',
	'echo_canceller_ui.cpp',
	'effects_base.cpp',
	'effects_box.cpp',
	'equalizer_band_box.cpp',
	'equalizer.cpp',
	'equalizer_preset.cpp',
	'equalizer_ui.cpp',
	'exciter.cpp',
	'exciter_preset.cpp',
	'exciter_ui.cpp',
	'filter.cpp',
	'filter_preset.cpp',
	'filter_ui.cpp',
	'fir_filter_bandpass.cpp',
	'fir_filter_base.cpp',
	'fir_filter_lowpass.cpp',
	'fir_filter_highpass.cpp',
	'gate.cpp',
	'gate_preset.cpp',
	'gate_ui.cpp',
	'limiter.cpp',
	'limiter_preset.cpp',
	'limiter_ui.cpp',
	'loudness.cpp',
	'loudness_preset.cpp',
	'loudness_ui.cpp',
	'lv2_wrapper.cpp',
	'maximizer.cpp',
	'maximizer_preset.cpp',
	'maximizer_ui.cpp',
	'module_info_holder.cpp',
	'multiband_compressor.cpp',
	'multiband_compressor_band_box.cpp',
	'multiband_compressor_preset.cpp',
	'multiband_compressor_ui.cpp',
	'multiband_gate.cpp',
	'multiband_gate_band_box.cpp',
	'multiband_gate_preset.cpp',
	'multiband_gate_ui.cpp',
	'node_info_holder.cpp',
	'output_level.cpp',
	'pipe_manager.cpp',
	'pipe_manager_box.cpp',
	'pitch.cpp',
	'pitch_preset.cpp',
	'pitch_ui.cpp',
	'plugin_base.cpp',
	'plugin_preset_base.cpp',
	'plugins_box.cpp',
	'plugins_menu.cpp',
	'preferences_general.cpp',
	'preferences_spectrum.cpp',
	'preferences_window.cpp',
	'presets_autoloading_holder.cpp',
	'presets_menu.cpp',
	'presets_manager.cpp',
	'reverb.cpp',
	'reverb_preset.cpp',
	'reverb_ui.cpp',
	'resampler.cpp',
	'rnnoise.cpp',
	'rnnoise_preset.cpp',
	'rnnoise_ui.cpp',
	'spectrum.cpp',
	'speex.cpp',
	'speex_preset.cpp',
	'speex_ui.cpp',
	'stereo_tools.cpp',
	'stereo_tools_preset.cpp',
	'stereo_tools_ui.cpp',
	'stream_output_effects.cpp',
	'stream_input_effects.cpp',
	'tags_plugin_name.cpp',
	'test_signals.cpp',
	'ui_helpers.cpp',
	'util.cpp',
	gresources
]

if get_option('enable-libportal') and libportal.found()
	easyeffects_sources += 'libportal.cpp'
endif

cxx = meson.get_compiler('cpp')

zita_convolver = cxx.find_library('zita-convolver', required: true)

tbb = cxx.find_library('tbb', required: true)

rnnoise = dependency('rnnoise', required: false)

speex = dependency('speex', required: false)

if rnnoise.found()
	add_global_arguments('-DRNNOISE_AVAILABLE=1', language : 'cpp')
else
	message('The RNNoise library was not found. The calls to its functions will be disabled')
endif

if speex.found()
	add_global_arguments('-DSPEEX_AVAILABLE=1', language : 'cpp')
else
	message('The Speex library was not found. The calls to its functions will be disabled')
endif

easyeffects_deps = [
	dependency('libpipewire-0.3', version: '>=0.3.41', include_type: 'system'),
	dependency('glib-2.0', version: '>=2.56', include_type: 'system'),
	dependency('gtk4', version: '>=4.2.1', include_type: 'system'),
	dependency('libadwaita-1', version: '>=1.2.0', include_type: 'system'),
	dependency('sigc++-3.0', version: '>=3.0.6', include_type: 'system'),
	dependency('lilv-0', version: '>=0.22', include_type: 'system'),
	dependency('lv2', version: '>=1.18.2', include_type: 'system'),
	dependency('libbs2b', include_type: 'system'),
	dependency('sndfile', include_type: 'system'),
	dependency('fftw3f', include_type: 'system'),
	dependency('fftw3', include_type: 'system'),
	dependency('libebur128',version: '>=1.2.0', include_type: 'system'),
	dependency('samplerate', include_type: 'system'),
	dependency('rubberband', include_type: 'system'),
	dependency('speexdsp', include_type: 'system'),
	dependency('nlohmann_json', include_type: 'system'),
	dependency('fmt', version: '>=8.0.0', include_type: 'system'),
	dependency('gsl', include_type: 'system'),
	dependency('threads'),
	tbb,
	zita_convolver,
	rnnoise,
	libportal,
	config_h
]

executable(
	meson.project_name(),
	easyeffects_sources,
	include_directories : [include_dir,config_h_dir],
	dependencies : easyeffects_deps,
	install: true
)
