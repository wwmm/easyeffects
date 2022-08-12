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

status = []

if get_option('devel')
  status += [ 'Using development build mode with .Devel appended to the application ID' ]
  app_id_suffix = '.Devel'
  name_suffix = ' (Devel)'
else
  status += [ 'Using stable build mode with the standard application ID' ]
  app_id_suffix = ''
  name_suffix = ''
endif

app_id = 'com.github.wwmm.easyeffects@0@'.format(app_id_suffix)

# Configuration for application itself at compile time, such as debug info like the commit that was built
conf = configuration_data()

conf.set10('IS_DEVEL_BUILD', get_option('devel'))
conf.set_quoted('APP_NAME', 'EasyEffects' + name_suffix)

conf.set_quoted('GETTEXT_PACKAGE', meson.project_name())
conf.set_quoted('LOCALE_DIR', localedir)
conf.set_quoted('VERSION', meson.project_version())

conf.set('COMMIT_DESC', '"@VCS_TAG@"')

config_h = declare_dependency(
  sources: vcs_tag(
    command: ['git', 'rev-parse', '--short', 'HEAD'],
    fallback: 'Could find commit of build',
    input: configure_file(
      output: 'config.h.in',
      configuration: conf
    ),
    output: 'config.h'
  )
)

subdir('data')
subdir('po')
subdir('help')
subdir('src')

meson.add_install_script('meson_post_install.py')

message('\n  '.join(status))
