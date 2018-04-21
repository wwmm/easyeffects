# -*- coding: utf-8 -*-

from gi.repository import Gio, GLib


class LimiterPresets():

    def __init__(self, config):
        self.config = config  # configParser

        self.settings_sinkinputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.limiter')

        self.settings_sourceoutputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs.limiter')

    def add_section(self, settings, section):
        enabled = settings.get_value('state')
        input_gain = settings.get_value('input-gain')
        limit = settings.get_value('limit')
        lookahead = settings.get_value('lookahead')
        release = settings.get_value('release')
        oversampling = settings.get_value('oversampling')
        asc = settings.get_value('asc')
        asc_level = settings.get_value('asc-level')

        self.config[section] = {'enabled': str(enabled),
                                'input_gain': str(input_gain),
                                'limit': str(limit),
                                'lookahead': str(lookahead),
                                'release': str(release),
                                'oversampling': str(oversampling),
                                'asc': str(asc),
                                'asc_level': str(asc_level)}

    def add_autovolume_section(self, settings, section):
        enabled = settings.get_value('autovolume-state')
        window = settings.get_value('autovolume-window')
        target = settings.get_value('autovolume-target')
        tolerance = settings.get_value('autovolume-tolerance')
        threshold = settings.get_value('autovolume-threshold')

        self.config[section] = {'enabled': str(enabled),
                                'window': str(window),
                                'target': str(target),
                                'tolerance': str(tolerance),
                                'threshold': str(threshold)}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)

        # reading autovolume state directly from preset because gsettings
        # writing is asynchronous. IT may not have been updated when we read it

        autovolume_state = None

        if 'apps' in section:
            autovolume_state = self.config.getboolean('apps_autovolume',
                                                      'enabled',
                                                      fallback=False)
        else:
            autovolume_state = self.config.getboolean('mic_autovolume',
                                                      'enabled',
                                                      fallback=False)

        if autovolume_state is False:
            input_gain = self.config.getfloat(section, 'input_gain',
                                              fallback=0.0)
            limit = self.config.getfloat(section, 'limit', fallback=0.0)
            lookahead = self.config.getfloat(section, 'lookahead', fallback=5)
            release = self.config.getfloat(section, 'release', fallback=50)
            oversampling = self.config.getint(section, 'oversampling',
                                              fallback=1)
            asc = self.config.getboolean(section, 'asc', fallback=False)
            asc_level = self.config.getfloat(section, 'asc_level',
                                             fallback=0.5)

            settings.set_value('state', GLib.Variant('b', enabled))
            settings.set_value('input-gain', GLib.Variant('d', input_gain))
            settings.set_value('limit', GLib.Variant('d', limit))
            settings.set_value('lookahead', GLib.Variant('d', lookahead))
            settings.set_value('release', GLib.Variant('d', release))
            settings.set_value('oversampling', GLib.Variant('i', oversampling))
            settings.set_value('asc', GLib.Variant('b', asc))
            settings.set_value('asc-level', GLib.Variant('d', asc_level))

        settings.set_value('state', GLib.Variant('b', enabled))

    def load_autovolume_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        window = self.config.getfloat(section, 'window', fallback=1000.0)
        target = self.config.getint(section, 'target', fallback=-12)
        tolerance = self.config.getint(section, 'tolerance', fallback=1)
        threshold = self.config.getint(section, 'threshold', fallback=-50)

        settings.set_value('autovolume-state', GLib.Variant('b', enabled))
        settings.set_value('autovolume-window', GLib.Variant('d', window))
        settings.set_value('autovolume-target', GLib.Variant('i', target))
        settings.set_value('autovolume-tolerance',
                           GLib.Variant('i', tolerance))
        settings.set_value('autovolume-threshold',
                           GLib.Variant('i', threshold))

    def save(self):
        self.add_autovolume_section(self.settings_sinkinputs,
                                    'apps_autovolume')
        self.add_section(self.settings_sinkinputs, 'apps_limiter')

        self.add_autovolume_section(self.settings_sourceoutputs,
                                    'mic_autovolume')
        self.add_section(self.settings_sourceoutputs, 'mic_limiter')

    def load(self):
        self.load_autovolume_section(self.settings_sinkinputs,
                                     'apps_autovolume')
        self.load_section(self.settings_sinkinputs, 'apps_limiter')

        self.load_autovolume_section(self.settings_sourceoutputs,
                                     'mic_autovolume')
        self.load_section(self.settings_sourceoutputs, 'mic_limiter')
