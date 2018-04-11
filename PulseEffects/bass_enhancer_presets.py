# -*- coding: utf-8 -*-

from gi.repository import Gio, GLib


class BassEnhancerPresets():

    def __init__(self, config):
        self.config = config  # configParser

        self.settings_sinkinputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.bassenhancer')

    def add_section(self, settings, section):
        enabled = settings.get_value('state')
        input_gain = settings.get_value('input-gain')
        output_gain = settings.get_value('output-gain')
        amount = settings.get_value('amount')
        harmonics = settings.get_value('harmonics')
        scope = settings.get_value('scope')
        floor = settings.get_value('floor')
        blend = settings.get_value('blend')

        self.config[section] = {'enabled': str(enabled),
                                'input_gain': str(input_gain),
                                'output_gain': str(output_gain),
                                'amount': str(amount),
                                'harmonics': str(harmonics),
                                'scope': str(scope),
                                'floor': str(floor),
                                'blend': str(blend)}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        input_gain = self.config.getfloat(section, 'input_gain', fallback=0.0)
        output_gain = self.config.getfloat(section, 'output_gain',
                                           fallback=0.0)
        amount = self.config.getfloat(section, 'amount', fallback=1.0)
        harmonics = self.config.getfloat(section, 'harmonics', fallback=8.5)
        scope = self.config.getfloat(section, 'scope', fallback=100.0)
        floor = self.config.getfloat(section, 'floor', fallback=20.0)
        blend = self.config.getfloat(section, 'blend', fallback=0.0)

        settings.set_value('state', GLib.Variant('b', enabled))
        settings.set_value('input-gain', GLib.Variant('d', input_gain))
        settings.set_value('output-gain', GLib.Variant('d', output_gain))
        settings.set_value('amount', GLib.Variant('d', amount))
        settings.set_value('harmonics', GLib.Variant('d', harmonics))
        settings.set_value('scope', GLib.Variant('d', scope))
        settings.set_value('floor', GLib.Variant('d', floor))
        settings.set_value('blend', GLib.Variant('d', blend))

    def save(self):
        self.add_section(self.settings_sinkinputs, 'apps_bass_enhancer')

    def load(self):
        self.load_section(self.settings_sinkinputs, 'apps_bass_enhancer')
