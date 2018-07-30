cxx = meson.get_compiler('cpp')

zita_convolver = cxx.find_library('zita-convolver', required: false)

if cxx.compiles(
'''
#include <zita-convolver.h>
#if ZITA_CONVOLVER_MAJOR_VERSION != 3 && ZITA_CONVOLVER_MAJOR_VERSION != 4
    #error "This programs requires zita-convolver 3 or 4"
#endif
''', dependencies: [zita_convolver])

plugin_sources = [
	'gstpeconvolver.cpp',
	'../util.cpp'
]

plugin_deps = [
	dependency('gstreamer-1.0'),
	dependency('gstreamer-base-1.0'),
	dependency('gstreamer-controller-1.0'),
	dependency('gstreamer-audio-1.0'),
	dependency('sndfile'),
	dependency('samplerate'),
	dependency('threads'),
	zita_convolver
]

plugins_install_dir = '@0@/gstreamer-1.0'.format(get_option('libdir'))

library(
	'gstpeconvolver',
	plugin_sources,
	include_directories : [include_dir,config_h_dir],
	dependencies : plugin_deps,
	install: true,
	install_dir : plugins_install_dir,
  cpp_args: plugins_cxx_args
)

else
	message('Missing dependency zita-convolver = 3.x.x or zita-convolver = 4.x.x')
	message('Convolver plugin will not be built')
endif
