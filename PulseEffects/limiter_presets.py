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
        release_time = settings.get_value('release-time')

        self.config[section] = {'enabled': str(enabled),
                                'input gain': str(input_gain),
                                'limit': str(limit),
                                'release time': str(release_time)}

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
        autovolume_state = settings.get_value('autovolume-state').unpack()

        if autovolume_state is False:
            input_gain = self.config.getfloat(section, 'input gain',
                                              fallback=0.0)
            limit = self.config.getfloat(section, 'limit', fallback=0.0)
            release_time = self.config.getfloat(section, 'release time',
                                                fallback=0.5)

            settings.set_value('input-gain', GLib.Variant('d', input_gain))
            settings.set_value('limit', GLib.Variant('d', limit))
            settings.set_value('release-time', GLib.Variant('d', release_time))

        settings.set_value('state', GLib.Variant('b', enabled))

    def load_autovolume_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        window = self.config.getfloat(section, 'window', fallback=1.0)
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
        self.add_section(self.settings_sinkinputs, 'apps_limiter')
        self.add_autovolume_section(self.settings_sinkinputs,
                                    'apps_autovolume')
        self.add_section(self.settings_sourceoutputs, 'mic_limiter')
        self.add_autovolume_section(self.settings_sourceoutputs,
                                    'mic_autovolume')

    def load(self):
        self.load_section(self.settings_sinkinputs, 'apps_limiter')
        self.load_autovolume_section(self.settings_sinkinputs,
                                     'apps_autovolume')
        self.load_section(self.settings_sourceoutputs, 'mic_limiter')
        self.load_autovolume_section(self.settings_sourceoutputs,
                                     'mic_autovolume')
