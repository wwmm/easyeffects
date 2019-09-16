project(
	'pulseeffects',
	'c',
	'cpp',
	default_options : ['cpp_std=c++17','buildtype=debugoptimized'],
	version: '4.6.8',
	meson_version: '>= 0.40.0'
)

add_global_arguments('-DG_LOG_DOMAIN="pulseeffects"', language : 'c')
add_global_arguments('-DG_LOG_DOMAIN="pulseeffects"', language : 'cpp')
add_global_arguments('-DGETTEXT_PACKAGE="@0@"'.format(meson.project_name()),
	language:'c')

gnome_mod = import('gnome')
i18n_mod = import('i18n')

prefix = get_option('prefix')
bindir = join_paths(prefix, get_option('bindir'))
datadir = join_paths(prefix, get_option('datadir'))
localedir = join_paths(prefix, get_option('localedir'))
include_dir = include_directories('include')
config_h_dir = include_directories('.')
plugins_install_dir = '@0@/gstreamer-1.0'.format(get_option('libdir'))

# Configuration
conf = configuration_data()

conf.set_quoted('GETTEXT_PACKAGE', meson.project_name())
conf.set_quoted('LOCALE_DIR', localedir)
conf.set_quoted('VERSION', meson.project_version())
conf.set_quoted('PACKAGE', 'PulseEffects')
conf.set_quoted('PLUGINS_INSTALL_DIR', join_paths(prefix, plugins_install_dir))

configure_file(output: 'config.h', configuration: conf)

plugins_cxx_args = [
	'-Wl,-Bsymbolic-functions',
	'-fvisibility=hidden',
	'-fno-strict-aliasing'
]

subdir('data')
subdir('po')
subdir('help')
subdir('src')

meson.add_install_script('meson_post_install.py')
