# -*- coding: utf-8 -*-

from gi.repository import Gio, GLib


class StereoSpreadPresets():

    def __init__(self, config):
        self.config = config  # configParser

        self.settings_sinkinputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.stereospread')

    def add_section(self, settings, section):
        enabled = settings.get_value('state')
        input_gain = settings.get_value('input-gain')
        output_gain = settings.get_value('output-gain')

        amount0 = settings.get_value('amount0')
        amount1 = settings.get_value('amount1')
        amount2 = settings.get_value('amount2')
        amount3 = settings.get_value('amount3')
        filters = settings.get_value('filters')
        mono = settings.get_value('mono')

        self.config[section] = {'enabled': str(enabled),
                                'input_gain': str(input_gain),
                                'output_gain': str(output_gain),
                                'amount0': str(amount0),
                                'amount1': str(amount1),
                                'amount2': str(amount2),
                                'amount3': str(amount3),
                                'filters': str(filters),
                                'mono': str(mono)}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        input_gain = self.config.getfloat(section, 'input_gain', fallback=0.0)
        output_gain = self.config.getfloat(section, 'output_gain',
                                           fallback=0.0)

        amount0 = self.config.getfloat(section, 'amount0', fallback=0.0)
        amount1 = self.config.getfloat(section, 'amount1', fallback=0.0)
        amount2 = self.config.getfloat(section, 'amount2', fallback=0.0)
        amount3 = self.config.getfloat(section, 'amount3', fallback=0.0)
        filters = self.config.getint(section, 'filters', fallback=2)
        mono = self.config.getboolean(section, 'mono', fallback=False)

        settings.set_value('state', GLib.Variant('b', enabled))
        settings.set_value('input-gain', GLib.Variant('d', input_gain))
        settings.set_value('output-gain', GLib.Variant('d', output_gain))

        settings.set_value('amount0', GLib.Variant('d', amount0))
        settings.set_value('amount1', GLib.Variant('d', amount1))
        settings.set_value('amount2', GLib.Variant('d', amount2))
        settings.set_value('amount3', GLib.Variant('d', amount3))
        settings.set_value('filters', GLib.Variant('i', filters))
        settings.set_value('mono', GLib.Variant('b', mono))

    def save(self):
        self.add_section(self.settings_sinkinputs, 'apps_stereo_spread')

    def load(self):
        self.load_section(self.settings_sinkinputs, 'apps_stereo_spread')
