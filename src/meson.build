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
	gresources
]

pulseeffects_deps = [
	# dependency('glib-2.0'),
	# dependency('gobject-2.0'),
	dependency('libpulse'),
	dependency('glibmm-2.4'),
	dependency('gtk+-3.0', version: '>= 3.20'),
	dependency('gtkmm-3.0',version: '>= 3.20')
]

executable(
	meson.project_name(),
	pulseeffects_sources,
	include_directories : include_dir,
	dependencies : pulseeffects_deps,
	install: true
)
