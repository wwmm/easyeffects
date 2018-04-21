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

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        input_gain = self.config.getfloat(section, 'input_gain', fallback=0.0)
        limit = self.config.getfloat(section, 'limit', fallback=0.0)
        lookahead = self.config.getfloat(section, 'lookahead', fallback=5)
        release = self.config.getfloat(section, 'release', fallback=50)
        oversampling = self.config.getint(section, 'oversampling',
                                          fallback=1)
        asc = self.config.getboolean(section, 'asc', fallback=False)
        asc_level = self.config.getfloat(section, 'asc_level', fallback=0.5)

        settings.set_value('state', GLib.Variant('b', enabled))
        settings.set_value('input-gain', GLib.Variant('d', input_gain))
        settings.set_value('limit', GLib.Variant('d', limit))
        settings.set_value('lookahead', GLib.Variant('d', lookahead))
        settings.set_value('release', GLib.Variant('d', release))
        settings.set_value('oversampling', GLib.Variant('i', oversampling))
        settings.set_value('asc', GLib.Variant('b', asc))
        settings.set_value('asc-level', GLib.Variant('d', asc_level))

    def save(self):
        self.add_section(self.settings_sinkinputs, 'apps_output_limiter')

    def load(self):
        self.load_section(self.settings_sinkinputs, 'apps_output_limiter')
