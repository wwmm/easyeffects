plugin_sources = [
	'gstpeconvolver.cpp',
	'zita-convolver.cc'
]

plugin_deps = [
	dependency('gstreamer-1.0'),
	dependency('gstreamer-base-1.0'),
	dependency('gstreamer-controller-1.0'),
	dependency('gstreamer-audio-1.0'),
	dependency('sndfile'),
	dependency('samplerate'),
	dependency('fftw3f'),
	dependency('threads')
]

plugins_install_dir = '@0@/gstreamer-1.0'.format(get_option('libdir'))

zita_cpp_args = '-D_POSIX_PTHREAD_SEMANTICS -DENABLE_VECTOR_MODE '
zita_cpp_args += '-fPIC -Wall -ffast-math -funroll-loops -O3'

library(
	'gstpeconvolver',
	plugin_sources,
	include_directories : include_dir,
	dependencies : plugin_deps,
	install: true,
	install_dir : plugins_install_dir,
	cpp_args: zita_cpp_args
)
