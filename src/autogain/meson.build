plugin_sources = [
	'gstpeautogain.cpp',
]

plugin_deps = [
	dependency('gstreamer-1.0'),
	dependency('gstreamer-base-1.0'),
	dependency('gstreamer-controller-1.0'),
	dependency('gstreamer-audio-1.0'),
	dependency('libebur128',version: '>=1.2.0')
]

plugins_install_dir = '@0@/gstreamer-1.0'.format(get_option('libdir'))

library(
	'gstpeautogain',
	plugin_sources,
	include_directories : config_h_dir,
	dependencies : plugin_deps,
	install: true,
	install_dir : plugins_install_dir,
	cpp_args: plugins_cxx_args
)
