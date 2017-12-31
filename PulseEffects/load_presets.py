# -*- coding: utf-8 -*-

import configparser

from gi.repository import GLib


class LoadPresets():

    def __init__(self):
        self.config = configparser.ConfigParser()

    def set_config_path(self, path):
        self.config.clear()
        self.config.read(path)

    def load_limiter_presets(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        autovolume_state = settings.get_value('autovolume-state').unpack()

        if autovolume_state is False:
            input_gain = self.config.getfloat(section, 'input gain',
                                              fallback=0.0)
            limit = self.config.getfloat(section, 'limit', fallback=0.0)
            release_time = self.config.getfloat(section, 'release time',
                                                fallback=0.5)

            settings.set_value('limiter-input-gain',
                               GLib.Variant('d', input_gain))
            settings.set_value('limiter-limit', GLib.Variant('d', limit))
            settings.set_value('limiter-release-time',
                               GLib.Variant('d', release_time))

        settings.set_value('limiter-state', GLib.Variant('b', enabled))

    def load_autovolume_presets(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        window = self.config.getfloat(section, 'window', fallback=1.0)
        target = self.config.getint(section, 'target', fallback=-12.0)
        tolerance = self.config.getint(section, 'tolerance', fallback=1.0)
        threshold = self.config.getint(section, 'threshold', fallback=-50.0)

        settings.set_value('autovolume-state', GLib.Variant('b', enabled))
        settings.set_value('autovolume-window', GLib.Variant('d', window))
        settings.set_value('autovolume-target', GLib.Variant('i', target))
        settings.set_value('autovolume-tolerance',
                           GLib.Variant('i', tolerance))
        settings.set_value('autovolume-threshold',
                           GLib.Variant('i', threshold))

    def load_panorama_presets(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        position = self.config.getfloat(section, 'position', fallback=0.0)

        settings.set_value('panorama-state', GLib.Variant('b', enabled))
        settings.set_value('panorama-position', GLib.Variant('d', position))

    def load_compressor_presets(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        use_peak = self.config.getboolean(section, 'use_peak', fallback=False)
        attack = self.config.getfloat(section, 'attack', fallback=101.1)
        release = self.config.getfloat(section, 'release', fallback=401.0)
        threshold = self.config.getfloat(section, 'threshold', fallback=0.0)
        ratio = self.config.getfloat(section, 'ratio', fallback=1.0)
        knee = self.config.getfloat(section, 'knee', fallback=3.0)
        makeup = self.config.getfloat(section, 'makeup', fallback=0.0)

        settings.set_value('compressor-state', GLib.Variant('b', enabled))
        settings.set_value('compressor-use-peak', GLib.Variant('b', use_peak))
        settings.set_value('compressor-attack', GLib.Variant('d', attack))
        settings.set_value('compressor-release', GLib.Variant('d', release))
        settings.set_value('compressor-threshold',
                           GLib.Variant('d', threshold))
        settings.set_value('compressor-ratio', GLib.Variant('d', ratio))
        settings.set_value('compressor-knee', GLib.Variant('d', knee))
        settings.set_value('compressor-makeup', GLib.Variant('d', makeup))

    def load_reverb_presets(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        room_size = self.config.getfloat(section, 'room size', fallback=0.5)
        damping = self.config.getfloat(section, 'damping', fallback=0.2)
        width = self.config.getfloat(section, 'width', fallback=1.0)
        level = self.config.getfloat(section, 'level', fallback=0.5)

        settings.set_value('reverb-state', GLib.Variant('b', enabled))
        settings.set_value('reverb-room-size', GLib.Variant('d', room_size))
        settings.set_value('reverb-damping', GLib.Variant('d', damping))
        settings.set_value('reverb-width', GLib.Variant('d', width))
        settings.set_value('reverb-level', GLib.Variant('d', level))

    def load_highpass_presets(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        cutoff = self.config.getint(section, 'cutoff', fallback=20)
        poles = self.config.getint(section, 'poles', fallback=4)

        settings.set_value('highpass-state', GLib.Variant('b', enabled))
        settings.set_value('highpass-cutoff', GLib.Variant('i', cutoff))
        settings.set_value('highpass-poles', GLib.Variant('i', poles))

    def load_lowpass_presets(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        cutoff = self.config.getint(section, 'cutoff', fallback=20000)
        poles = self.config.getint(section, 'poles', fallback=4)

        settings.set_value('lowpass-state', GLib.Variant('b', enabled))
        settings.set_value('lowpass-cutoff', GLib.Variant('i', cutoff))
        settings.set_value('lowpass-poles', GLib.Variant('i', poles))

    def load_equalizer_presets(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        input_gain = self.config.getfloat(section, 'input_gain', fallback=0)
        output_gain = self.config.getfloat(section, 'output_gain', fallback=0)

        # it would make more sense to call these keys bandn_g but this would
        # break people presets...
        self.band0_g = self.config.getfloat(section, 'band0', fallback=0)
        self.band1_g = self.config.getfloat(section, 'band1', fallback=0)
        self.band2_g = self.config.getfloat(section, 'band2', fallback=0)
        self.band3_g = self.config.getfloat(section, 'band3', fallback=0)
        self.band4_g = self.config.getfloat(section, 'band4', fallback=0)
        self.band5_g = self.config.getfloat(section, 'band5', fallback=0)
        self.band6_g = self.config.getfloat(section, 'band6', fallback=0)
        self.band7_g = self.config.getfloat(section, 'band7', fallback=0)
        self.band8_g = self.config.getfloat(section, 'band8', fallback=0)
        self.band9_g = self.config.getfloat(section, 'band9', fallback=0)
        self.band10_g = self.config.getfloat(section, 'band10', fallback=0)
        self.band11_g = self.config.getfloat(section, 'band11', fallback=0)
        self.band12_g = self.config.getfloat(section, 'band12', fallback=0)
        self.band13_g = self.config.getfloat(section, 'band13', fallback=0)
        self.band14_g = self.config.getfloat(section, 'band14', fallback=0)
        self.band15_g = self.config.getfloat(section, 'band15', fallback=0)
        self.band16_g = self.config.getfloat(section, 'band16', fallback=0)
        self.band17_g = self.config.getfloat(section, 'band17', fallback=0)
        self.band18_g = self.config.getfloat(section, 'band18', fallback=0)
        self.band19_g = self.config.getfloat(section, 'band19', fallback=0)
        self.band20_g = self.config.getfloat(section, 'band20', fallback=0)
        self.band21_g = self.config.getfloat(section, 'band21', fallback=0)
        self.band22_g = self.config.getfloat(section, 'band22', fallback=0)
        self.band23_g = self.config.getfloat(section, 'band23', fallback=0)
        self.band24_g = self.config.getfloat(section, 'band24', fallback=0)
        self.band25_g = self.config.getfloat(section, 'band25', fallback=0)
        self.band26_g = self.config.getfloat(section, 'band26', fallback=0)
        self.band27_g = self.config.getfloat(section, 'band27', fallback=0)
        self.band28_g = self.config.getfloat(section, 'band28', fallback=0)
        self.band29_g = self.config.getfloat(section, 'band29', fallback=0)

        self.band0_f = self.config.getfloat(section, 'band0_freq',
                                            fallback=22.59)
        self.band1_f = self.config.getfloat(section, 'band1_freq',
                                            fallback=28.44)
        self.band2_f = self.config.getfloat(section, 'band2_freq',
                                            fallback=35.8)
        self.band3_f = self.config.getfloat(section, 'band3_freq',
                                            fallback=45.07)
        self.band4_f = self.config.getfloat(section, 'band4_freq',
                                            fallback=56.74)
        self.band5_f = self.config.getfloat(section, 'band5_freq',
                                            fallback=71.43)
        self.band6_f = self.config.getfloat(section, 'band6_freq',
                                            fallback=89.93)
        self.band7_f = self.config.getfloat(section, 'band7_freq',
                                            fallback=113.21)
        self.band8_f = self.config.getfloat(section, 'band8_freq',
                                            fallback=142.53)
        self.band9_f = self.config.getfloat(section, 'band9_freq',
                                            fallback=179.43)
        self.band10_f = self.config.getfloat(section, 'band10_freq',
                                             fallback=225.89)
        self.band11_f = self.config.getfloat(section, 'band11_freq',
                                             fallback=284.38)
        self.band12_f = self.config.getfloat(section, 'band12_freq',
                                             fallback=358.02)
        self.band13_f = self.config.getfloat(section, 'band13_freq',
                                             fallback=450.72)
        self.band14_f = self.config.getfloat(section, 'band14_freq',
                                             fallback=567.42)
        self.band15_f = self.config.getfloat(section, 'band15_freq',
                                             fallback=714.34)
        self.band16_f = self.config.getfloat(section, 'band16_freq',
                                             fallback=899.29)
        self.band17_f = self.config.getfloat(section, 'band17_freq',
                                             fallback=1132.15)
        self.band18_f = self.config.getfloat(section, 'band18_freq',
                                             fallback=1425.29)
        self.band19_f = self.config.getfloat(section, 'band19_freq',
                                             fallback=1794.33)
        self.band20_f = self.config.getfloat(section, 'band20_freq',
                                             fallback=2258.93)
        self.band21_f = self.config.getfloat(section, 'band21_freq',
                                             fallback=2843.82)
        self.band22_f = self.config.getfloat(section, 'band22_freq',
                                             fallback=3580.16)
        self.band23_f = self.config.getfloat(section, 'band23_freq',
                                             fallback=4507.15)
        self.band24_f = self.config.getfloat(section, 'band24_freq',
                                             fallback=5674.16)
        self.band25_f = self.config.getfloat(section, 'band25_freq',
                                             fallback=7143.35)
        self.band26_f = self.config.getfloat(section, 'band26_freq',
                                             fallback=8992.94)
        self.band27_f = self.config.getfloat(section, 'band27_freq',
                                             fallback=11321.45)
        self.band28_f = self.config.getfloat(section, 'band28_freq',
                                             fallback=14252.86)
        self.band29_f = self.config.getfloat(section, 'band29_freq',
                                             fallback=17943.28)

        self.band0_q = self.config.getfloat(section, 'band0_qfactor',
                                            fallback=2.21)
        self.band1_q = self.config.getfloat(section, 'band1_qfactor',
                                            fallback=2.21)
        self.band2_q = self.config.getfloat(section, 'band2_qfactor',
                                            fallback=2.21)
        self.band3_q = self.config.getfloat(section, 'band3_qfactor',
                                            fallback=2.21)
        self.band4_q = self.config.getfloat(section, 'band4_qfactor',
                                            fallback=2.21)
        self.band5_q = self.config.getfloat(section, 'band5_qfactor',
                                            fallback=2.21)
        self.band6_q = self.config.getfloat(section, 'band6_qfactor',
                                            fallback=2.21)
        self.band7_q = self.config.getfloat(section, 'band7_qfactor',
                                            fallback=2.21)
        self.band8_q = self.config.getfloat(section, 'band8_qfactor',
                                            fallback=2.21)
        self.band9_q = self.config.getfloat(section, 'band9_qfactor',
                                            fallback=2.21)
        self.band10_q = self.config.getfloat(section, 'band10_qfactor',
                                             fallback=2.21)
        self.band11_q = self.config.getfloat(section, 'band11_qfactor',
                                             fallback=2.21)
        self.band12_q = self.config.getfloat(section, 'band12_qfactor',
                                             fallback=2.21)
        self.band13_q = self.config.getfloat(section, 'band13_qfactor',
                                             fallback=2.21)
        self.band14_q = self.config.getfloat(section, 'band14_qfactor',
                                             fallback=2.21)
        self.band15_q = self.config.getfloat(section, 'band15_qfactor',
                                             fallback=2.21)
        self.band16_q = self.config.getfloat(section, 'band16_qfactor',
                                             fallback=2.21)
        self.band17_q = self.config.getfloat(section, 'band17_qfactor',
                                             fallback=2.21)
        self.band18_q = self.config.getfloat(section, 'band18_qfactor',
                                             fallback=2.21)
        self.band19_q = self.config.getfloat(section, 'band19_qfactor',
                                             fallback=2.21)
        self.band20_q = self.config.getfloat(section, 'band20_qfactor',
                                             fallback=2.21)
        self.band21_q = self.config.getfloat(section, 'band21_qfactor',
                                             fallback=2.21)
        self.band22_q = self.config.getfloat(section, 'band22_qfactor',
                                             fallback=2.21)
        self.band23_q = self.config.getfloat(section, 'band23_qfactor',
                                             fallback=2.21)
        self.band24_q = self.config.getfloat(section, 'band24_qfactor',
                                             fallback=2.21)
        self.band25_q = self.config.getfloat(section, 'band25_qfactor',
                                             fallback=2.21)
        self.band26_q = self.config.getfloat(section, 'band26_qfactor',
                                             fallback=2.21)
        self.band27_q = self.config.getfloat(section, 'band27_qfactor',
                                             fallback=2.21)
        self.band28_q = self.config.getfloat(section, 'band28_qfactor',
                                             fallback=2.21)
        self.band29_q = self.config.getfloat(section, 'band29_qfactor',
                                             fallback=2.21)

        self.band0_t = self.config.getfloat(section, 'band0_type', fallback=0)
        self.band1_t = self.config.getfloat(section, 'band1_type', fallback=0)
        self.band2_t = self.config.getfloat(section, 'band2_type', fallback=0)
        self.band3_t = self.config.getfloat(section, 'band3_type', fallback=0)
        self.band4_t = self.config.getfloat(section, 'band4_type', fallback=0)
        self.band5_t = self.config.getfloat(section, 'band5_type', fallback=0)
        self.band6_t = self.config.getfloat(section, 'band6_type', fallback=0)
        self.band7_t = self.config.getfloat(section, 'band7_type', fallback=0)
        self.band8_t = self.config.getfloat(section, 'band8_type', fallback=0)
        self.band9_t = self.config.getfloat(section, 'band9_type', fallback=0)
        self.band10_t = self.config.getfloat(section, 'band10_type',
                                             fallback=0)
        self.band11_t = self.config.getfloat(section, 'band11_type',
                                             fallback=0)
        self.band12_t = self.config.getfloat(section, 'band12_type',
                                             fallback=0)
        self.band13_t = self.config.getfloat(section, 'band13_type',
                                             fallback=0)
        self.band14_t = self.config.getfloat(section, 'band14_type',
                                             fallback=0)
        self.band15_t = self.config.getfloat(section, 'band15_type',
                                             fallback=0)
        self.band16_t = self.config.getfloat(section, 'band16_type',
                                             fallback=0)
        self.band17_t = self.config.getfloat(section, 'band17_type',
                                             fallback=0)
        self.band18_t = self.config.getfloat(section, 'band18_type',
                                             fallback=0)
        self.band19_t = self.config.getfloat(section, 'band19_type',
                                             fallback=0)
        self.band20_t = self.config.getfloat(section, 'band20_type',
                                             fallback=0)
        self.band21_t = self.config.getfloat(section, 'band21_type',
                                             fallback=0)
        self.band22_t = self.config.getfloat(section, 'band22_type',
                                             fallback=0)
        self.band23_t = self.config.getfloat(section, 'band23_type',
                                             fallback=0)
        self.band24_t = self.config.getfloat(section, 'band24_type',
                                             fallback=0)
        self.band25_t = self.config.getfloat(section, 'band25_type',
                                             fallback=0)
        self.band26_t = self.config.getfloat(section, 'band26_type',
                                             fallback=0)
        self.band27_t = self.config.getfloat(section, 'band27_type',
                                             fallback=0)
        self.band28_t = self.config.getfloat(section, 'band28_type',
                                             fallback=0)
        self.band29_t = self.config.getfloat(section, 'band29_type',
                                             fallback=0)

        settings.set_value('equalizer-state', GLib.Variant('b', enabled))
        settings.set_value('equalizer-input-gain',
                           GLib.Variant('d', input_gain))
        settings.set_value('equalizer-output-gain',
                           GLib.Variant('d', output_gain))

        for n in range(30):
            g = GLib.Variant('d', getattr(self, 'band' + str(n) + '_g'))
            f = GLib.Variant('d', getattr(self, 'band' + str(n) + '_f'))
            q = GLib.Variant('d', getattr(self, 'band' + str(n) + '_q'))
            t = GLib.Variant('i', getattr(self, 'band' + str(n) + '_t'))

            settings.set_value('equalizer-band' + str(n) + '-gain', g)
            settings.set_value('equalizer-band' + str(n) + '-frequency', f)
            settings.set_value('equalizer-band' + str(n) + '-quality', q)
            settings.set_value('equalizer-band' + str(n) + '-type', t)

    def load_exciter_presets(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        input_gain = self.config.getfloat(section, 'input_gain', fallback=0.0)
        output_gain = self.config.getfloat(section, 'output_gain',
                                           fallback=0.0)
        amount = self.config.getfloat(section, 'amount', fallback=1.0)
        harmonics = self.config.getfloat(section, 'harmonics', fallback=8.5)
        scope = self.config.getfloat(section, 'scope', fallback=7500.0)
        ceiling = self.config.getfloat(section, 'ceiling', fallback=16000.0)
        blend = self.config.getfloat(section, 'blend', fallback=0.0)

        settings.set_value('exciter-state', GLib.Variant('b', enabled))
        settings.set_value('exciter-input-gain',
                           GLib.Variant('d', input_gain))
        settings.set_value('exciter-output-gain',
                           GLib.Variant('d', output_gain))
        settings.set_value('exciter-amount', GLib.Variant('d', amount))
        settings.set_value('exciter-harmonics',
                           GLib.Variant('d', harmonics))
        settings.set_value('exciter-scope', GLib.Variant('d', scope))
        settings.set_value('exciter-ceiling', GLib.Variant('d', ceiling))
        settings.set_value('exciter-blend', GLib.Variant('d', blend))

    def load_bass_enhancer_presets(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        input_gain = self.config.getfloat(section, 'input_gain', fallback=0.0)
        output_gain = self.config.getfloat(section, 'output_gain',
                                           fallback=0.0)
        amount = self.config.getfloat(section, 'amount', fallback=1.0)
        harmonics = self.config.getfloat(section, 'harmonics', fallback=8.5)
        scope = self.config.getfloat(section, 'scope', fallback=100.0)
        floor = self.config.getfloat(section, 'floor', fallback=20.0)
        blend = self.config.getfloat(section, 'blend', fallback=0.0)

        settings.set_value('bass-enhancer-state', GLib.Variant('b', enabled))
        settings.set_value('bass-enhancer-input-gain',
                           GLib.Variant('d', input_gain))
        settings.set_value('bass-enhancer-output-gain',
                           GLib.Variant('d', output_gain))
        settings.set_value('bass-enhancer-amount', GLib.Variant('d', amount))
        settings.set_value('bass-enhancer-harmonics',
                           GLib.Variant('d', harmonics))
        settings.set_value('bass-enhancer-scope', GLib.Variant('d', scope))
        settings.set_value('bass-enhancer-floor', GLib.Variant('d', floor))
        settings.set_value('bass-enhancer-blend', GLib.Variant('d', blend))

    def load_stereo_enhancer_presets(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        input_gain = self.config.getfloat(section, 'input_gain', fallback=0.0)
        output_gain = self.config.getfloat(section, 'output_gain',
                                           fallback=0.0)

        left_invert_phase = self.config.getboolean(section,
                                                   'left_invert_phase',
                                                   fallback=False)
        left_balance = self.config.getfloat(section, 'left_balance',
                                            fallback=-1.0)
        left_delay = self.config.getfloat(section, 'left_delay',
                                          fallback=2.05)
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

        settings.set_value('stereo-enhancer-state', GLib.Variant('b', enabled))
        settings.set_value('stereo-enhancer-input-gain',
                           GLib.Variant('d', input_gain))
        settings.set_value('stereo-enhancer-output-gain',
                           GLib.Variant('d', output_gain))

        settings.set_value('stereo-enhancer-left-invert-phase',
                           GLib.Variant('b', left_invert_phase))
        settings.set_value('stereo-enhancer-left-balance',
                           GLib.Variant('d', left_balance))
        settings.set_value('stereo-enhancer-left-delay',
                           GLib.Variant('d', left_delay))
        settings.set_value('stereo-enhancer-left-gain',
                           GLib.Variant('d', left_gain))

        settings.set_value('stereo-enhancer-right-invert-phase',
                           GLib.Variant('b', right_invert_phase))
        settings.set_value('stereo-enhancer-right-balance',
                           GLib.Variant('d', right_balance))
        settings.set_value('stereo-enhancer-right-delay',
                           GLib.Variant('d', right_delay))
        settings.set_value('stereo-enhancer-right-gain',
                           GLib.Variant('d', right_gain))

        settings.set_value('stereo-enhancer-middle-invert-phase',
                           GLib.Variant('b', middle_invert_phase))
        settings.set_value('stereo-enhancer-middle-source',
                           GLib.Variant('i', middle_source))

        settings.set_value('stereo-enhancer-side-gain',
                           GLib.Variant('d', side_gain))

    def load_stereo_spread_presets(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        input_gain = self.config.getfloat(section, 'input_gain', fallback=0.0)
        output_gain = self.config.getfloat(section, 'output_gain',
                                           fallback=0.0)

        amount0 = self.config.getfloat(section, 'amount0', fallback=0.0)
        amount1 = self.config.getfloat(section, 'amount1', fallback=0.0)
        amount2 = self.config.getfloat(section, 'amount2', fallback=0.0)
        amount3 = self.config.getfloat(section, 'amount3', fallback=0.0)
        filters = self.config.getint(section, 'filters', fallback=2.0)
        mono = self.config.getboolean(section, 'mono', fallback=False)

        settings.set_value('stereo-spread-state', GLib.Variant('b', enabled))
        settings.set_value('stereo-spread-input-gain',
                           GLib.Variant('d', input_gain))
        settings.set_value('stereo-spread-output-gain',
                           GLib.Variant('d', output_gain))

        settings.set_value('stereo-spread-amount0',
                           GLib.Variant('d', amount0))
        settings.set_value('stereo-spread-amount1',
                           GLib.Variant('d', amount1))
        settings.set_value('stereo-spread-amount2',
                           GLib.Variant('d', amount2))
        settings.set_value('stereo-spread-amount3',
                           GLib.Variant('d', amount3))
        settings.set_value('stereo-spread-filters',
                           GLib.Variant('i', filters))
        settings.set_value('stereo-spread-mono',
                           GLib.Variant('b', mono))

    def load_maximizer_presets(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        release = self.config.getfloat(section, 'release', fallback=3.16)
        ceiling = self.config.getfloat(section, 'ceiling', fallback=0.0)
        threshold = self.config.getfloat(section, 'threshold', fallback=0.0)

        settings.set_value('maximizer-state', GLib.Variant('b', enabled))
        settings.set_value('maximizer-release', GLib.Variant('d', release))
        settings.set_value('maximizer-ceiling', GLib.Variant('d', ceiling))
        settings.set_value('maximizer-threshold', GLib.Variant('d', threshold))

    def load_output_limiter_presets(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        input_gain = self.config.getfloat(section, 'input gain', fallback=0.0)
        limit = self.config.getfloat(section, 'limit', fallback=0.0)
        release_time = self.config.getfloat(section, 'release time',
                                            fallback=0.5)

        settings.set_value('output-limiter-input-gain',
                           GLib.Variant('d', input_gain))
        settings.set_value('output-limiter-limit', GLib.Variant('d', limit))
        settings.set_value('output-limiter-release-time',
                           GLib.Variant('d', release_time))
        settings.set_value('output-limiter-state', GLib.Variant('b', enabled))

    def load_sink_inputs_presets(self, settings):
        self.load_limiter_presets(settings, 'apps_limiter')
        self.load_autovolume_presets(settings, 'apps_autovolume')
        self.load_panorama_presets(settings, 'apps_panorama')
        self.load_compressor_presets(settings, 'apps_compressor')
        self.load_reverb_presets(settings, 'apps_reverb')
        self.load_highpass_presets(settings, 'apps_highpass')
        self.load_lowpass_presets(settings, 'apps_lowpass')
        self.load_equalizer_presets(settings, 'apps_equalizer')
        self.load_exciter_presets(settings, 'apps_exciter')
        self.load_bass_enhancer_presets(settings, 'apps_bass_enhancer')
        self.load_stereo_enhancer_presets(settings, 'apps_stereo_enhancer')
        self.load_stereo_spread_presets(settings, 'apps_stereo_spread')
        self.load_maximizer_presets(settings, 'apps_maximizer')
        self.load_output_limiter_presets(settings, 'apps_output_limiter')

    def load_source_outputs_presets(self, settings):
        self.load_limiter_presets(settings, 'mic_limiter')
        self.load_autovolume_presets(settings, 'mic_autovolume')
        self.load_compressor_presets(settings, 'mic_compressor')
        self.load_reverb_presets(settings, 'mic_reverb')
        self.load_highpass_presets(settings, 'mic_highpass')
        self.load_lowpass_presets(settings, 'mic_lowpass')
        self.load_equalizer_presets(settings, 'mic_equalizer')
