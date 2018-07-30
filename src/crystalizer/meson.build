plugin_sources = [
	'gstpecrystalizer.cpp',
]

plugin_deps = [
	dependency('gstreamer-1.0'),
	dependency('gstreamer-base-1.0'),
	dependency('gstreamer-controller-1.0'),
	dependency('gstreamer-audio-1.0')
]

library(
	'gstpecrystalizer',
	plugin_sources,
	include_directories : config_h_dir,
	dependencies : plugin_deps,
	install: true,
	install_dir : plugins_install_dir,
	cpp_args: plugins_cxx_args
)
