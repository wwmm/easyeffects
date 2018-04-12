# -*- coding: utf-8 -*-

from gi.repository import Gio, GLib


class StereoEnhancerPresets():

    def __init__(self, config):
        self.config = config  # configParser

        self.settings_sinkinputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.stereoenhancer')

    def add_section(self, settings, section):
        enabled = settings.get_value('state')
        input_gain = settings.get_value('input-gain')
        output_gain = settings.get_value('output-gain')

        left_invert_phase = settings.get_value('left-invert-phase')
        left_balance = settings.get_value('left-balance')
        left_delay = settings.get_value('left-delay')
        left_gain = settings.get_value('left-gain')

        right_invert_phase = settings.get_value('right-invert-phase')
        right_balance = settings.get_value('right-balance')
        right_delay = settings.get_value('right-delay')
        right_gain = settings.get_value('right-gain')

        middle_invert_phase = settings.get_value('middle-invert-phase')
        middle_source = settings.get_value('middle-source')

        side_gain = settings.get_value('side-gain')

        self.config[section] = {'enabled': str(enabled),
                                'input_gain': str(input_gain),
                                'output_gain': str(output_gain),
                                'left_invert_phase': str(left_invert_phase),
                                'left_balance': str(left_balance),
                                'left_delay': str(left_delay),
                                'left_gain': str(left_gain),
                                'right_invert_phase': str(right_invert_phase),
                                'right_balance': str(right_balance),
                                'right_delay': str(right_delay),
                                'right_gain': str(right_gain),
                                'middle_invert_phase':
                                str(middle_invert_phase),
                                'middle_source': str(middle_source),
                                'side_gain': str(side_gain)}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        input_gain = self.config.getfloat(section, 'input_gain', fallback=0.0)
        output_gain = self.config.getfloat(section, 'output_gain',
                                           fallback=0.0)

        left_invert_phase = self.config.getboolean(section,
                                                   'left_invert_phase',
                                                   fallback=False)
        left_balance = self.config.getfloat(section, 'left_balance',
                                            fallback=-1.0)
        left_delay = self.config.getfloat(section, 'left_delay', fallback=2.05)
        left_gain = self.config.getfloat(section, 'left_gain', fallback=0.0)

        right_invert_phase = self.config.getboolean(section,
                                                    'right_invert_phase',
                                                    fallback=False)
        right_balance = self.config.getfloat(section, 'right_balance',
                                             fallback=-1.0)
        right_delay = self.config.getfloat(section, 'right_delay',
                                           fallback=2.05)
        right_gain = self.config.getfloat(section, 'right_gain', fallback=0.0)

        middle_invert_phase = self.config.getboolean(section,
                                                     'middle_invert_phase',
                                                     fallback=False)
        middle_source = self.config.getint(section, 'middle_source',
                                           fallback=2)

        side_gain = self.config.getfloat(section, 'side_gain', fallback=0.0)

        settings.set_value('state', GLib.Variant('b', enabled))
        settings.set_value('input-gain', GLib.Variant('d', input_gain))
        settings.set_value('output-gain', GLib.Variant('d', output_gain))

        settings.set_value('left-invert-phase',
                           GLib.Variant('b', left_invert_phase))
        settings.set_value('left-balance', GLib.Variant('d', left_balance))
        settings.set_value('left-delay', GLib.Variant('d', left_delay))
        settings.set_value('left-gain', GLib.Variant('d', left_gain))

        settings.set_value('right-invert-phase',
                           GLib.Variant('b', right_invert_phase))
        settings.set_value('right-balance', GLib.Variant('d', right_balance))
        settings.set_value('right-delay', GLib.Variant('d', right_delay))
        settings.set_value('right-gain', GLib.Variant('d', right_gain))

        settings.set_value('middle-invert-phase',
                           GLib.Variant('b', middle_invert_phase))
        settings.set_value('middle-source', GLib.Variant('i', middle_source))

        settings.set_value('side-gain', GLib.Variant('d', side_gain))

    def save(self):
        self.add_section(self.settings_sinkinputs, 'apps_stereo_enhancer')

    def load(self):
        self.load_section(self.settings_sinkinputs, 'apps_stereo_enhancer')
