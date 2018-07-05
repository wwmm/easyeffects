zita_deps = [
	dependency('fftw3f'),
	dependency('threads')
]

zita_cpp_args = '-D_POSIX_PTHREAD_SEMANTICS -DENABLE_VECTOR_MODE '
zita_cpp_args += '-fPIC -Wall -ffast-math -funroll-loops -O3'

zita = static_library(
	'zitaconcolver',
	'zita-convolver.cc',
	dependencies : zita_deps,
	cpp_args: zita_cpp_args
)

plugin_sources = [
	'gstpeconvolver.cpp'
]

plugin_deps = [
	dependency('gstreamer-1.0'),
	dependency('gstreamer-base-1.0'),
	dependency('gstreamer-controller-1.0'),
	dependency('gstreamer-audio-1.0'),
	dependency('sndfile'),
	dependency('samplerate')
]

plugins_install_dir = '@0@/gstreamer-1.0'.format(get_option('libdir'))

library(
	'gstpeconvolver',
	plugin_sources,
	include_directories : include_dir,
	dependencies : plugin_deps,
	install: true,
	install_dir : plugins_install_dir,
	link_with: zita
)
