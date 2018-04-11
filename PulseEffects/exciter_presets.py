# -*- coding: utf-8 -*-

from gi.repository import Gio, GLib


class ExciterPresets():

    def __init__(self, config):
        self.config = config  # configParser

        self.settings_sinkinputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.exciter')

    def add_section(self, settings, section):
        enabled = settings.get_value('state')
        input_gain = settings.get_value('input-gain')
        output_gain = settings.get_value('output-gain')
        amount = settings.get_value('amount')
        harmonics = settings.get_value('harmonics')
        scope = settings.get_value('scope')
        ceiling = settings.get_value('ceiling')
        blend = settings.get_value('blend')

        self.config[section] = {'enabled': str(enabled),
                                'input_gain': str(input_gain),
                                'output_gain': str(output_gain),
                                'amount': str(amount),
                                'harmonics': str(harmonics),
                                'scope': str(scope),
                                'ceiling': str(ceiling),
                                'blend': str(blend)}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        input_gain = self.config.getfloat(section, 'input_gain', fallback=0.0)
        output_gain = self.config.getfloat(section, 'output_gain',
                                           fallback=0.0)
        amount = self.config.getfloat(section, 'amount', fallback=1.0)
        harmonics = self.config.getfloat(section, 'harmonics', fallback=8.5)
        scope = self.config.getfloat(section, 'scope', fallback=7500.0)
        ceiling = self.config.getfloat(section, 'ceiling', fallback=16000.0)
        blend = self.config.getfloat(section, 'blend', fallback=0.0)

        settings.set_value('state', GLib.Variant('b', enabled))
        settings.set_value('input-gain', GLib.Variant('d', input_gain))
        settings.set_value('output-gain', GLib.Variant('d', output_gain))
        settings.set_value('amount', GLib.Variant('d', amount))
        settings.set_value('harmonics', GLib.Variant('d', harmonics))
        settings.set_value('scope', GLib.Variant('d', scope))
        settings.set_value('ceiling', GLib.Variant('d', ceiling))
        settings.set_value('blend', GLib.Variant('d', blend))

    def save(self):
        self.add_section(self.settings_sinkinputs, 'apps_exciter')

    def load(self):
        self.load_section(self.settings_sinkinputs, 'apps_exciter')
