cxx = meson.get_compiler('cpp')

cxx_args = '-Wl,-Bsymbolic-functions'

# Symbol visibility
cxx_args += '-fvisibility=hidden'

# Disable strict aliasing
cxx_args += '-fno-strict-aliasing'

plugin_sources = [
	'gstpeebur.cpp',
]

plugin_deps = [
	dependency('gstreamer-1.0'),
	dependency('gstreamer-base-1.0'),
	dependency('gstreamer-controller-1.0'),
	dependency('gstreamer-audio-1.0'),
	dependency('libebur128')
]

plugins_install_dir = '@0@/gstreamer-1.0'.format(get_option('libdir'))

library(
	'gstpeebur',
	plugin_sources,
	include_directories : include_dir,
	dependencies : plugin_deps,
	install: true,
	install_dir : plugins_install_dir,
	c_args: cxx_args
)
