# -*- coding: utf-8 -*-

from gi.repository import Gio, GLib


class MaximizerPresets():

    def __init__(self, config):
        self.config = config  # configParser

        self.settings_sinkinputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.maximizer')

    def add_section(self, settings, section):
        enabled = settings.get_value('state')
        release = settings.get_value('release')
        ceiling = settings.get_value('ceiling')
        threshold = settings.get_value('threshold')

        self.config[section] = {'enabled': str(enabled),
                                'release': str(release),
                                'ceiling': str(ceiling),
                                'threshold': str(threshold)}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        release = self.config.getfloat(section, 'release', fallback=3.16)
        ceiling = self.config.getfloat(section, 'ceiling', fallback=0.0)
        threshold = self.config.getfloat(section, 'threshold', fallback=0.0)

        settings.set_value('state', GLib.Variant('b', enabled))
        settings.set_value('release', GLib.Variant('d', release))
        settings.set_value('ceiling', GLib.Variant('d', ceiling))
        settings.set_value('threshold', GLib.Variant('d', threshold))

    def save(self):
        self.add_section(self.settings_sinkinputs, 'apps_maximizer')

    def load(self):
        self.load_section(self.settings_sinkinputs, 'apps_maximizer')
