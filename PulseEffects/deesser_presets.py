# -*- coding: utf-8 -*-

from gi.repository import Gio, GLib


class DeesserPresets():

    def __init__(self, config):
        self.config = config  # configParser

        self.settings_sourceoutputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs.deesser')

    def add_section(self, settings, section):
        enabled = settings.get_value('state')
        detection = settings.get_value('detection-rms')
        mode = settings.get_value('mode-wide')
        threshold = settings.get_value('threshold')
        ratio = settings.get_value('ratio')
        makeup = settings.get_value('makeup')
        laxity = settings.get_value('laxity')
        f1 = settings.get_value('f1')
        f1_level = settings.get_value('f1-level')
        f2 = settings.get_value('f2')
        f2_level = settings.get_value('f2-level')
        f2_q = settings.get_value('f2-q')

        self.config[section] = {'enabled': str(enabled),
                                'detection_type_rms': str(detection),
                                'mode_type_wide': str(mode),
                                'threshold': str(threshold),
                                'ratio': str(ratio),
                                'makeup': str(makeup),
                                'laxity': str(laxity),
                                'f1': str(f1),
                                'f1_level': str(f1_level),
                                'f2': str(f2),
                                'f2_level': str(f2_level),
                                'f2_q': str(f2_q)}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        detection = self.config.getboolean(section, 'detection_type_rms',
                                           fallback=True)
        mode = self.config.getboolean(section, 'mode_type_wide', fallback=True)
        threshold = self.config.getfloat(section, 'threshold', fallback=-18.0)
        ratio = self.config.getfloat(section, 'ratio', fallback=3.0)
        makeup = self.config.getfloat(section, 'makeup', fallback=0.0)
        laxity = self.config.getint(section, 'laxity', fallback=15)
        f1 = self.config.getfloat(section, 'f1', fallback=6000.0)
        f1_level = self.config.getfloat(section, 'f1_level', fallback=0.0)
        f2 = self.config.getfloat(section, 'f2', fallback=4500.0)
        f2_level = self.config.getfloat(section, 'f2_level', fallback=12.0)
        f2_q = self.config.getfloat(section, 'f2_q', fallback=1.0)

        settings.set_value('state', GLib.Variant('b', enabled))
        settings.set_value('detection-rms', GLib.Variant('b', detection))
        settings.set_value('mode-wide', GLib.Variant('b', mode))
        settings.set_value('threshold', GLib.Variant('d', threshold))
        settings.set_value('ratio', GLib.Variant('d', ratio))
        settings.set_value('makeup', GLib.Variant('d', makeup))
        settings.set_value('laxity', GLib.Variant('i', laxity))
        settings.set_value('f1', GLib.Variant('d', f1))
        settings.set_value('f1-level', GLib.Variant('d', f1_level))
        settings.set_value('f2', GLib.Variant('d', f2))
        settings.set_value('f2-level', GLib.Variant('d', f2_level))
        settings.set_value('f2-q', GLib.Variant('d', f2_q))

    def save(self):
        self.add_section(self.settings_sourceoutputs, 'mic_deesser')

    def load(self):
        self.load_section(self.settings_sourceoutputs, 'mic_deesser')
