conf_pulseeffects = configuration_data()
conf_pulseeffects.set('LOCALEDIR', localedir)
conf_pulseeffects.set('PROJECT_NAME', meson.project_name())

pulseeffects_cpp = configure_file(input: 'pulseeffects.cpp.in',
							 output: 'pulseeffects.cpp',
							 configuration: conf_pulseeffects,
							 install_dir: bindir)


pulseeffects_sources = [
	pulseeffects_cpp,
	'application.cpp',
	'application_ui.cpp',
	'pulse_manager.cpp',
	'effects_base_ui.cpp',
	'app_info_ui.cpp',
	'sink_input_effects_ui.cpp',
	'sink_input_effects.cpp',
	'source_output_effects_ui.cpp',
	'source_output_effects.cpp',
	'pipeline_base.cpp',
	'util.cpp',
	gresources
]

pulseeffects_deps = [
	dependency('libpulse'),
	dependency('glibmm-2.4'),
	dependency('gtk+-3.0', version: '>= 3.20'),
	dependency('gtkmm-3.0',version: '>= 3.20'),
	dependency('gstreamer-1.0', version: '>= 1.12'),
	dependency('gstreamer-plugins-bad-1.0', version: '>= 1.12'),
	dependency('gstreamer-webrtc-1.0', required: false),
	dependency('lilv-0', version: '>= 0.22', required: false),
	dependency('libbs2b', required: false)
]

executable(
	meson.project_name(),
	pulseeffects_sources,
	include_directories : include_dir,
	dependencies : pulseeffects_deps,
	install: true
)
