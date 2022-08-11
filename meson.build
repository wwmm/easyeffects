project(
	'easyeffects',
	'c',
	'cpp',
	default_options : ['cpp_std=c++20','buildtype=debugoptimized'],
	version: '6.2.8',
	meson_version: '>= 0.57.0'
)

# see https://github.com/wwmm/easyeffects/pull/1739 for more info
cxx = meson.get_compiler('cpp')
assert(cxx.get_id() == 'gcc', 'This project only supports the gcc compiler due to usage of c++20 features.')

add_global_arguments('-DG_LOG_DOMAIN="easyeffects"', language : 'c')
add_global_arguments('-DG_LOG_DOMAIN="easyeffects"', language : 'cpp')
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

# Configuration
conf = configuration_data()

conf.set_quoted('GETTEXT_PACKAGE', meson.project_name())
conf.set_quoted('LOCALE_DIR', localedir)
conf.set_quoted('VERSION', meson.project_version())

configure_file(output: 'config.h', configuration: conf)

subdir('data')
subdir('po')
subdir('help')
subdir('src')

meson.add_install_script('meson_post_install.py')
