plugin_sources = [
	'gstpecrystalizer.cpp',
]

plugin_deps = [
	dependency('gstreamer-1.0'),
	dependency('gstreamer-base-1.0'),
	dependency('gstreamer-controller-1.0'),
	dependency('gstreamer-audio-1.0')
]

plugins_install_dir = '@0@/gstreamer-1.0'.format(get_option('libdir'))

library(
	'gstpecrystalizer',
	plugin_sources,
	include_directories : include_dir,
	dependencies : plugin_deps,
	install: true,
	install_dir : plugins_install_dir
)
