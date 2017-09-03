# -*- coding: utf-8 -*-

import configparser

from gi.repository import Gio, GLib


class LoadPresets():

    def __init__(self, presets_path):
        self.config = configparser.ConfigParser()

        self.config.read(presets_path)

        self.settings = Gio.Settings('com.github.wwmm.pulseeffects')

    def load_sink_inputs_preset(self, settings):
        autovolume_state = settings.get_value('autovolume-state').unpack()

        if autovolume_state is False:
            limiter = dict(self.config['apps_limiter']).values()
            limiter = [float(v) for v in limiter]

            settings.set_value('limiter-user', GLib.Variant('ad', limiter))

        panorama_position = self.config.getfloat('apps_panorama', 'position',
                                                 fallback=0.0)

        compressor = dict(self.config['apps_compressor']).values()
        compressor = [float(v) for v in compressor]

        reverb = dict(self.config['apps_reverb']).values()
        reverb = [float(v) for v in reverb]

        highpass_cutoff = self.config.getint('apps_highpass',
                                             'cutoff',
                                             fallback=20)
        highpass_poles = self.config.getint('apps_highpass',
                                            'poles',
                                            fallback=4)
        lowpass_cutoff = self.config.getint('apps_lowpass',
                                            'cutoff',
                                            fallback=20000)
        lowpass_poles = self.config.getint('apps_lowpass',
                                           'poles',
                                           fallback=4)

        equalizer_input_gain = self.config.getfloat('apps_equalizer',
                                                    'input_gain',
                                                    fallback=0)
        equalizer_output_gain = self.config.getfloat('apps_equalizer',
                                                     'output_gain',
                                                     fallback=0)

        equalizer_band0 = self.config.getfloat('apps_equalizer', 'band0')
        equalizer_band1 = self.config.getfloat('apps_equalizer', 'band1')
        equalizer_band2 = self.config.getfloat('apps_equalizer', 'band2')
        equalizer_band3 = self.config.getfloat('apps_equalizer', 'band3')
        equalizer_band4 = self.config.getfloat('apps_equalizer', 'band4')
        equalizer_band5 = self.config.getfloat('apps_equalizer', 'band5')
        equalizer_band6 = self.config.getfloat('apps_equalizer', 'band6')
        equalizer_band7 = self.config.getfloat('apps_equalizer', 'band7')
        equalizer_band8 = self.config.getfloat('apps_equalizer', 'band8')
        equalizer_band9 = self.config.getfloat('apps_equalizer', 'band9')
        equalizer_band10 = self.config.getfloat('apps_equalizer', 'band10')
        equalizer_band11 = self.config.getfloat('apps_equalizer', 'band11')
        equalizer_band12 = self.config.getfloat('apps_equalizer', 'band12')
        equalizer_band13 = self.config.getfloat('apps_equalizer', 'band13')
        equalizer_band14 = self.config.getfloat('apps_equalizer', 'band14')

        eq_band0_freq = self.config.getfloat('apps_equalizer', 'band0_freq',
                                             fallback=26)
        eq_band1_freq = self.config.getfloat('apps_equalizer', 'band1_freq',
                                             fallback=41)
        eq_band2_freq = self.config.getfloat('apps_equalizer', 'band2_freq',
                                             fallback=65)
        eq_band3_freq = self.config.getfloat('apps_equalizer', 'band3_freq',
                                             fallback=103)
        eq_band4_freq = self.config.getfloat('apps_equalizer', 'band4_freq',
                                             fallback=163)
        eq_band5_freq = self.config.getfloat('apps_equalizer', 'band5_freq',
                                             fallback=259)
        eq_band6_freq = self.config.getfloat('apps_equalizer', 'band6_freq',
                                             fallback=410)
        eq_band7_freq = self.config.getfloat('apps_equalizer', 'band7_freq',
                                             fallback=649)
        eq_band8_freq = self.config.getfloat('apps_equalizer', 'band8_freq',
                                             fallback=1029)
        eq_band9_freq = self.config.getfloat('apps_equalizer', 'band9_freq',
                                             fallback=1631)
        eq_band10_freq = self.config.getfloat('apps_equalizer', 'band10_freq',
                                              fallback=2585)
        eq_band11_freq = self.config.getfloat('apps_equalizer', 'band11_freq',
                                              fallback=4097)
        eq_band12_freq = self.config.getfloat('apps_equalizer', 'band12_freq',
                                              fallback=6493)
        eq_band13_freq = self.config.getfloat('apps_equalizer', 'band13_freq',
                                              fallback=10291)
        eq_band14_freq = self.config.getfloat('apps_equalizer', 'band14_freq',
                                              fallback=16310)

        eq_band0_qfactor = self.config.getfloat('apps_equalizer',
                                                'band0_qfactor',
                                                fallback=2.21)
        eq_band1_qfactor = self.config.getfloat('apps_equalizer',
                                                'band1_qfactor',
                                                fallback=2.21)
        eq_band2_qfactor = self.config.getfloat('apps_equalizer',
                                                'band2_qfactor',
                                                fallback=2.21)
        eq_band3_qfactor = self.config.getfloat('apps_equalizer',
                                                'band3_qfactor',
                                                fallback=2.21)
        eq_band4_qfactor = self.config.getfloat('apps_equalizer',
                                                'band4_qfactor',
                                                fallback=2.21)
        eq_band5_qfactor = self.config.getfloat('apps_equalizer',
                                                'band5_qfactor',
                                                fallback=2.21)
        eq_band6_qfactor = self.config.getfloat('apps_equalizer',
                                                'band6_qfactor',
                                                fallback=2.21)
        eq_band7_qfactor = self.config.getfloat('apps_equalizer',
                                                'band7_qfactor',
                                                fallback=2.21)
        eq_band8_qfactor = self.config.getfloat('apps_equalizer',
                                                'band8_qfactor',
                                                fallback=2.21)
        eq_band9_qfactor = self.config.getfloat('apps_equalizer',
                                                'band9_qfactor',
                                                fallback=2.21)
        eq_band10_qfactor = self.config.getfloat('apps_equalizer',
                                                 'band10_qfactor',
                                                 fallback=2.21)
        eq_band11_qfactor = self.config.getfloat('apps_equalizer',
                                                 'band11_qfactor',
                                                 fallback=2.21)
        eq_band12_qfactor = self.config.getfloat('apps_equalizer',
                                                 'band12_qfactor',
                                                 fallback=2.21)
        eq_band13_qfactor = self.config.getfloat('apps_equalizer',
                                                 'band13_qfactor',
                                                 fallback=2.21)
        eq_band14_qfactor = self.config.getfloat('apps_equalizer',
                                                 'band14_qfactor',
                                                 fallback=2.21)

        equalizer_bands = [equalizer_band0, equalizer_band1,
                           equalizer_band2, equalizer_band3,
                           equalizer_band4, equalizer_band5,
                           equalizer_band6, equalizer_band7,
                           equalizer_band8, equalizer_band9,
                           equalizer_band10, equalizer_band11,
                           equalizer_band12, equalizer_band13,
                           equalizer_band14]

        eq_freqs = [eq_band0_freq, eq_band1_freq, eq_band2_freq, eq_band3_freq,
                    eq_band4_freq, eq_band5_freq, eq_band6_freq, eq_band7_freq,
                    eq_band8_freq, eq_band9_freq, eq_band10_freq,
                    eq_band11_freq, eq_band12_freq, eq_band13_freq,
                    eq_band14_freq]

        eq_qfactors = [eq_band0_qfactor, eq_band1_qfactor, eq_band2_qfactor,
                       eq_band3_qfactor, eq_band4_qfactor, eq_band5_qfactor,
                       eq_band6_qfactor, eq_band7_qfactor, eq_band8_qfactor,
                       eq_band9_qfactor, eq_band10_qfactor, eq_band11_qfactor,
                       eq_band12_qfactor, eq_band13_qfactor, eq_band14_qfactor]

        settings.set_value('panorama-position',
                           GLib.Variant('d', panorama_position))

        settings.set_value('compressor-user', GLib.Variant('ad', compressor))

        settings.set_value('reverb-user', GLib.Variant('ad', reverb))

        settings.set_value('highpass-cutoff',
                           GLib.Variant('i', highpass_cutoff))
        settings.set_value('highpass-poles',
                           GLib.Variant('i', highpass_poles))

        settings.set_value('lowpass-cutoff',
                           GLib.Variant('i', lowpass_cutoff))
        settings.set_value('lowpass-poles',
                           GLib.Variant('i', lowpass_poles))

        settings.set_value('equalizer-input-gain',
                           GLib.Variant('d', equalizer_input_gain))
        settings.set_value('equalizer-output-gain',
                           GLib.Variant('d', equalizer_output_gain))
        settings.set_value('equalizer-user',
                           GLib.Variant('ad', equalizer_bands))
        settings.set_value('equalizer-freqs',
                           GLib.Variant('ad', eq_freqs))
        settings.set_value('equalizer-qfactors',
                           GLib.Variant('ad', eq_qfactors))

    def load_source_outputs_preset(self, settings):
        limiter = dict(self.config['mic_limiter']).values()
        limiter = [float(v) for v in limiter]

        compressor = dict(self.config['mic_compressor']).values()
        compressor = [float(v) for v in compressor]

        reverb = dict(self.config['mic_reverb']).values()
        reverb = [float(v) for v in reverb]

        highpass_cutoff = self.config.getint('mic_equalizer',
                                             'cutoff',
                                             fallback=20)
        highpass_poles = self.config.getint('mic_equalizer',
                                            'poles',
                                            fallback=4)
        lowpass_cutoff = self.config.getint('mic_equalizer',
                                            'cutoff',
                                            fallback=20000)
        lowpass_poles = self.config.getint('mic_equalizer',
                                           'poles',
                                           fallback=4)

        equalizer_input_gain = self.config.getfloat('mic_equalizer',
                                                    'input_gain',
                                                    fallback=0)
        equalizer_output_gain = self.config.getfloat('mic_equalizer',
                                                     'output_gain',
                                                     fallback=0)

        equalizer_band0 = self.config.getfloat('mic_equalizer', 'band0')
        equalizer_band1 = self.config.getfloat('mic_equalizer', 'band1')
        equalizer_band2 = self.config.getfloat('mic_equalizer', 'band2')
        equalizer_band3 = self.config.getfloat('mic_equalizer', 'band3')
        equalizer_band4 = self.config.getfloat('mic_equalizer', 'band4')
        equalizer_band5 = self.config.getfloat('mic_equalizer', 'band5')
        equalizer_band6 = self.config.getfloat('mic_equalizer', 'band6')
        equalizer_band7 = self.config.getfloat('mic_equalizer', 'band7')
        equalizer_band8 = self.config.getfloat('mic_equalizer', 'band8')
        equalizer_band9 = self.config.getfloat('mic_equalizer', 'band9')
        equalizer_band10 = self.config.getfloat('mic_equalizer', 'band10')
        equalizer_band11 = self.config.getfloat('mic_equalizer', 'band11')
        equalizer_band12 = self.config.getfloat('mic_equalizer', 'band12')
        equalizer_band13 = self.config.getfloat('mic_equalizer', 'band13')
        equalizer_band14 = self.config.getfloat('mic_equalizer', 'band14')

        eq_band0_freq = self.config.getfloat('mic_equalizer', 'band0_freq',
                                             fallback=26)
        eq_band1_freq = self.config.getfloat('mic_equalizer', 'band1_freq',
                                             fallback=41)
        eq_band2_freq = self.config.getfloat('mic_equalizer', 'band2_freq',
                                             fallback=65)
        eq_band3_freq = self.config.getfloat('mic_equalizer', 'band3_freq',
                                             fallback=103)
        eq_band4_freq = self.config.getfloat('mic_equalizer', 'band4_freq',
                                             fallback=163)
        eq_band5_freq = self.config.getfloat('mic_equalizer', 'band5_freq',
                                             fallback=259)
        eq_band6_freq = self.config.getfloat('mic_equalizer', 'band6_freq',
                                             fallback=410)
        eq_band7_freq = self.config.getfloat('mic_equalizer', 'band7_freq',
                                             fallback=649)
        eq_band8_freq = self.config.getfloat('mic_equalizer', 'band8_freq',
                                             fallback=1029)
        eq_band9_freq = self.config.getfloat('mic_equalizer', 'band9_freq',
                                             fallback=1631)
        eq_band10_freq = self.config.getfloat('mic_equalizer', 'band10_freq',
                                              fallback=2585)
        eq_band11_freq = self.config.getfloat('mic_equalizer', 'band11_freq',
                                              fallback=4097)
        eq_band12_freq = self.config.getfloat('mic_equalizer', 'band12_freq',
                                              fallback=6493)
        eq_band13_freq = self.config.getfloat('mic_equalizer', 'band13_freq',
                                              fallback=10291)
        eq_band14_freq = self.config.getfloat('mic_equalizer', 'band14_freq',
                                              fallback=16310)

        eq_band0_qfactor = self.config.getfloat('mic_equalizer',
                                                'band0_qfactor',
                                                fallback=2.21)
        eq_band1_qfactor = self.config.getfloat('mic_equalizer',
                                                'band1_qfactor',
                                                fallback=2.21)
        eq_band2_qfactor = self.config.getfloat('mic_equalizer',
                                                'band2_qfactor',
                                                fallback=2.21)
        eq_band3_qfactor = self.config.getfloat('mic_equalizer',
                                                'band3_qfactor',
                                                fallback=2.21)
        eq_band4_qfactor = self.config.getfloat('mic_equalizer',
                                                'band4_qfactor',
                                                fallback=2.21)
        eq_band5_qfactor = self.config.getfloat('mic_equalizer',
                                                'band5_qfactor',
                                                fallback=2.21)
        eq_band6_qfactor = self.config.getfloat('mic_equalizer',
                                                'band6_qfactor',
                                                fallback=2.21)
        eq_band7_qfactor = self.config.getfloat('mic_equalizer',
                                                'band7_qfactor',
                                                fallback=2.21)
        eq_band8_qfactor = self.config.getfloat('mic_equalizer',
                                                'band8_qfactor',
                                                fallback=2.21)
        eq_band9_qfactor = self.config.getfloat('mic_equalizer',
                                                'band9_qfactor',
                                                fallback=2.21)
        eq_band10_qfactor = self.config.getfloat('mic_equalizer',
                                                 'band10_qfactor',
                                                 fallback=2.21)
        eq_band11_qfactor = self.config.getfloat('mic_equalizer',
                                                 'band11_qfactor',
                                                 fallback=2.21)
        eq_band12_qfactor = self.config.getfloat('mic_equalizer',
                                                 'band12_qfactor',
                                                 fallback=2.21)
        eq_band13_qfactor = self.config.getfloat('mic_equalizer',
                                                 'band13_qfactor',
                                                 fallback=2.21)
        eq_band14_qfactor = self.config.getfloat('mic_equalizer',
                                                 'band14_qfactor',
                                                 fallback=2.21)

        equalizer_bands = [equalizer_band0, equalizer_band1,
                           equalizer_band2, equalizer_band3,
                           equalizer_band4, equalizer_band5,
                           equalizer_band6, equalizer_band7,
                           equalizer_band8, equalizer_band9,
                           equalizer_band10, equalizer_band11,
                           equalizer_band12, equalizer_band13,
                           equalizer_band14]

        eq_freqs = [eq_band0_freq, eq_band1_freq, eq_band2_freq, eq_band3_freq,
                    eq_band4_freq, eq_band5_freq, eq_band6_freq, eq_band7_freq,
                    eq_band8_freq, eq_band9_freq, eq_band10_freq,
                    eq_band11_freq, eq_band12_freq, eq_band13_freq,
                    eq_band14_freq]

        eq_qfactors = [eq_band0_qfactor, eq_band1_qfactor, eq_band2_qfactor,
                       eq_band3_qfactor, eq_band4_qfactor, eq_band5_qfactor,
                       eq_band6_qfactor, eq_band7_qfactor, eq_band8_qfactor,
                       eq_band9_qfactor, eq_band10_qfactor, eq_band11_qfactor,
                       eq_band12_qfactor, eq_band13_qfactor, eq_band14_qfactor]

        settings.set_value('limiter-user', GLib.Variant('ad', limiter))

        settings.set_value('compressor-user', GLib.Variant('ad', compressor))

        settings.set_value('reverb-user', GLib.Variant('ad', reverb))

        settings.set_value('highpass-cutoff',
                           GLib.Variant('i', highpass_cutoff))
        settings.set_value('highpass-poles',
                           GLib.Variant('i', highpass_poles))

        settings.set_value('lowpass-cutoff',
                           GLib.Variant('i', lowpass_cutoff))
        settings.set_value('lowpass-poles',
                           GLib.Variant('i', lowpass_poles))

        settings.set_value('equalizer-input-gain',
                           GLib.Variant('d', equalizer_input_gain))
        settings.set_value('equalizer-output-gain',
                           GLib.Variant('d', equalizer_output_gain))
        settings.set_value('equalizer-user',
                           GLib.Variant('ad', equalizer_bands))
        settings.set_value('equalizer-freqs',
                           GLib.Variant('ad', eq_freqs))
        settings.set_value('equalizer-qfactors',
                           GLib.Variant('ad', eq_qfactors))
