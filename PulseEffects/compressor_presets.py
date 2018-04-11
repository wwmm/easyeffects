# -*- coding: utf-8 -*-

from gi.repository import Gio, GLib


class CompressorPresets():

    def __init__(self, config):
        self.config = config  # configParser

        self.settings_sinkinputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.compressor')

        self.settings_sourceoutputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs.compressor')

    def add_section(self, settings, section):
        enabled = settings.get_value('state')
        use_peak = settings.get_value('use-peak')
        attack = settings.get_value('attack')
        release = settings.get_value('release')
        threshold = settings.get_value('threshold')
        ratio = settings.get_value('ratio')
        knee = settings.get_value('knee')
        makeup = settings.get_value('makeup')

        self.config[section] = {'enabled': str(enabled),
                                'use_peak': str(use_peak),
                                'attack': str(attack),
                                'release': str(release),
                                'threshold': str(threshold),
                                'ratio': str(ratio),
                                'knee': str(knee),
                                'makeup': str(makeup)}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        use_peak = self.config.getboolean(section, 'use_peak', fallback=False)
        attack = self.config.getfloat(section, 'attack', fallback=101.1)
        release = self.config.getfloat(section, 'release', fallback=401.0)
        threshold = self.config.getfloat(section, 'threshold', fallback=0.0)
        ratio = self.config.getfloat(section, 'ratio', fallback=1.0)
        knee = self.config.getfloat(section, 'knee', fallback=3.0)
        makeup = self.config.getfloat(section, 'makeup', fallback=0.0)

        settings.set_value('state', GLib.Variant('b', enabled))
        settings.set_value('use-peak', GLib.Variant('b', use_peak))
        settings.set_value('attack', GLib.Variant('d', attack))
        settings.set_value('release', GLib.Variant('d', release))
        settings.set_value('threshold', GLib.Variant('d', threshold))
        settings.set_value('ratio', GLib.Variant('d', ratio))
        settings.set_value('knee', GLib.Variant('d', knee))
        settings.set_value('makeup', GLib.Variant('d', makeup))

    def save(self):
        self.add_section(self.settings_sinkinputs, 'apps_compressor')
        self.add_section(self.settings_sourceoutputs, 'mic_compressor')

    def load(self):
        self.load_section(self.settings_sinkinputs, 'apps_compressor')
        self.load_section(self.settings_sourceoutputs, 'mic_compressor')
