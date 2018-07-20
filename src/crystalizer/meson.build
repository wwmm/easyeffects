cxx = meson.get_compiler('cpp')

cxx_args = ''

if cxx.has_link_argument('-Wl,-Bsymbolic-functions')
  cxx_args += '-Wl,-Bsymbolic-functions'
endif

# Symbol visibility
if cxx.has_argument('-fvisibility=hidden')
	cxx_args += '-fvisibility=hidden'
endif

# Disable strict aliasing
if cxx.has_argument('-fno-strict-aliasing')
  cxx_args += '-fno-strict-aliasing'
endif

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
	install_dir : plugins_install_dir,
	c_args: cxx_args
)
