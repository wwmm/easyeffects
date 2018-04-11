# -*- coding: utf-8 -*-

from gi.repository import Gio, GLib


class PanoramaPresets():

    def __init__(self, config):
        self.config = config  # configParser

        self.settings_sinkinputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.panorama')

    def add_section(self, settings, section):
        enabled = settings.get_value('state')
        position = settings.get_value('position')

        self.config[section] = {'enabled': str(enabled),
                                'position': str(position)}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        position = self.config.getfloat(section, 'position', fallback=0.0)

        settings.set_value('state', GLib.Variant('b', enabled))
        settings.set_value('position', GLib.Variant('d', position))

        settings.set_value('state', GLib.Variant('b', enabled))

    def save(self):
        self.add_section(self.settings_sinkinputs, 'apps_panorama')

    def load(self):
        self.load_section(self.settings_sinkinputs, 'apps_panorama')
