# -*- coding: utf-8 -*-

from gi.repository import Gio, GLib


class CrossfeedPresets():

    def __init__(self, config):
        self.config = config  # configParser

        self.settings_sinkinputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.crossfeed')

    def add_section(self, settings, section):
        enabled = settings.get_value('state')
        fcut = settings.get_value('fcut')
        feed = settings.get_value('feed')

        self.config[section] = {'enabled': str(enabled),
                                'fcut': str(fcut),
                                'feed': str(feed)}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        fcut = self.config.getint(section, 'fcut', fallback=700)
        feed = self.config.getfloat(section, 'feed', fallback=4.5)

        settings.set_value('state', GLib.Variant('b', enabled))
        settings.set_value('fcut', GLib.Variant('i', fcut))
        settings.set_value('feed', GLib.Variant('d', feed))

    def save(self):
        self.add_section(self.settings_sinkinputs, 'apps_crossfeed')

    def load(self):
        self.load_section(self.settings_sinkinputs, 'apps_crossfeed')
