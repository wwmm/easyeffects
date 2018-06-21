project(
	'pulseeffects',
	'c',
	'cpp',
	default_options : ['cpp_std=c++17'],
	version: '4.0.9',
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

subdir('data')
subdir('po')
subdir('src')

meson.add_install_script('meson_post_install.py')
