dep_rnnoise = dependency('rnnoise', required: false)

if dep_rnnoise.found()

plugin_sources = [
	'gstpernnoise.cpp',
	'../util.cpp'
]

plugin_deps = [
	dependency('gstreamer-1.0'),
	dependency('gstreamer-base-1.0'),
	dependency('gstreamer-controller-1.0'),
	dependency('gstreamer-audio-1.0'),
	dep_rnnoise,
	dependency('threads')
]

plugins_install_dir = '@0@/gstreamer-1.0'.format(get_option('libdir'))

library(
	'gstpernnoise',
	plugin_sources,
	include_directories : [include_dir,config_h_dir],
	dependencies : plugin_deps,
	install: true,
	install_dir : plugins_install_dir,
  	cpp_args: plugins_cxx_args
)

else

message('could not find the RNNoise library')

endif
