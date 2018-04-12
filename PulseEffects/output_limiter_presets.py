# -*- coding: utf-8 -*-

from gi.repository import Gio, GLib


class OutputLimiterPresets():

    def __init__(self, config):
        self.config = config  # configParser

        self.settings_sinkinputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.outputlimiter')

    def add_section(self, settings, section):
        enabled = settings.get_value('state')
        input_gain = settings.get_value('input-gain')
        limit = settings.get_value('limit')
        release_time = settings.get_value('release-time')

        self.config[section] = {'enabled': str(enabled),
                                'input gain': str(input_gain),
                                'limit': str(limit),
                                'release time': str(release_time)}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        input_gain = self.config.getfloat(section, 'input gain', fallback=0.0)
        limit = self.config.getfloat(section, 'limit', fallback=0.0)
        release_time = self.config.getfloat(section, 'release time',
                                            fallback=0.5)

        settings.set_value('input-gain', GLib.Variant('d', input_gain))
        settings.set_value('limit', GLib.Variant('d', limit))
        settings.set_value('release-time', GLib.Variant('d', release_time))
        settings.set_value('state', GLib.Variant('b', enabled))

    def save(self):
        self.add_section(self.settings_sinkinputs, 'apps_output_limiter')

    def load(self):
        self.load_section(self.settings_sinkinputs, 'apps_output_limiter')
