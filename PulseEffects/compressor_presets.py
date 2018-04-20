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
        detection = settings.get_value('detection-rms')
        stereo_link = settings.get_value('stereo-link-average')
        mix = settings.get_value('mix')
        attack = settings.get_value('attack')
        release = settings.get_value('release')
        threshold = settings.get_value('threshold')
        ratio = settings.get_value('ratio')
        knee = settings.get_value('knee')
        makeup = settings.get_value('makeup')

        self.config[section] = {'enabled': str(enabled),
                                'detection_type_rms': str(detection),
                                'stereo_link_type_average': str(stereo_link),
                                'mix': str(mix),
                                'attack': str(attack),
                                'release': str(release),
                                'threshold': str(threshold),
                                'ratio': str(ratio),
                                'knee': str(knee),
                                'makeup': str(makeup)}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        detection = self.config.getboolean(section, 'detection_type_rms',
                                           fallback=True)
        stereo_link = self.config.getboolean(section,
                                             'stereo_link_type_average',
                                             fallback=True)
        mix = self.config.getfloat(section, 'mix', fallback=0.0)
        attack = self.config.getfloat(section, 'attack', fallback=20.0)
        release = self.config.getfloat(section, 'release', fallback=250.0)
        threshold = self.config.getfloat(section, 'threshold', fallback=-18.0)
        ratio = self.config.getfloat(section, 'ratio', fallback=2.0)
        knee = self.config.getfloat(section, 'knee', fallback=9.0)
        makeup = self.config.getfloat(section, 'makeup', fallback=0.0)

        settings.set_value('state', GLib.Variant('b', enabled))
        settings.set_value('detection-rms', GLib.Variant('b', detection))
        settings.set_value('stereo-link-average',
                           GLib.Variant('b', stereo_link))
        settings.set_value('mix', GLib.Variant('d', mix))
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
