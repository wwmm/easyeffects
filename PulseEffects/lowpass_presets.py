# -*- coding: utf-8 -*-

from gi.repository import Gio, GLib


class LowpassPresets():

    def __init__(self, config):
        self.config = config  # configParser

        self.settings_sinkinputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.lowpass')

        self.settings_sourceoutputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs.lowpass')

    def add_section(self, settings, section):
        enabled = settings.get_value('state')
        cutoff = settings.get_value('cutoff')
        poles = settings.get_value('poles')

        self.config[section] = {'enabled': str(enabled), 'cutoff': str(cutoff),
                                'poles': str(poles)}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        cutoff = self.config.getint(section, 'cutoff', fallback=20000)
        poles = self.config.getint(section, 'poles', fallback=4)

        settings.set_value('state', GLib.Variant('b', enabled))
        settings.set_value('cutoff', GLib.Variant('i', cutoff))
        settings.set_value('poles', GLib.Variant('i', poles))

    def save(self):
        self.add_section(self.settings_sinkinputs, 'apps_lowpass')
        self.add_section(self.settings_sourceoutputs, 'mic_lowpass')

    def load(self):
        self.load_section(self.settings_sinkinputs, 'apps_lowpass')
        self.load_section(self.settings_sourceoutputs, 'mic_lowpass')
