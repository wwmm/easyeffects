conf_pulseeffects = configuration_data()
conf_pulseeffects.set('LOCALEDIR', localedir)
conf_pulseeffects.set('PROJECT_NAME', meson.project_name())

pulseeffects_vala = configure_file(input: 'pulseeffects.vala.in',
							 output: 'pulseeffects.vala',
							 configuration: conf_pulseeffects,
							 install_dir: bindir)


pulseeffects_sources = [
	pulseeffects_vala,
	'application.vala',
	'application_window.vala',
	'pulse_manager.vala',
	'parse_app_info.vala',
	gresources
]

pulseeffects_deps = [
	dependency('glib-2.0'),
	dependency('gobject-2.0'),
	dependency('libpulse'),
	dependency('gtk+-3.0', version: '>= 3.20')
]

executable(
	meson.project_name(),
	pulseeffects_sources,
	dependencies : pulseeffects_deps,
	install: true
)
