plugins_install_dir = '@0@/gstreamer-1.0'.format(get_option('libdir'))

plugin_sources = [
	'gstpeconvolver.cpp'
]

plugin_deps = [
	dependency('gstreamer-1.0'),
	dependency('gstreamer-base-1.0'),
	dependency('gstreamer-controller-1.0'),
	dependency('gstreamer-audio-1.0'),
]

library(
	'gstpeconvolver',
	plugin_sources,
	include_directories : include_dir,
	dependencies : plugin_deps,
	install: true
)
