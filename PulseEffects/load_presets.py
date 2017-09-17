# -*- coding: utf-8 -*-

import configparser

from gi.repository import GLib


class LoadPresets():

    def __init__(self, presets_path):
        self.config = configparser.ConfigParser()
        self.config.read(presets_path)

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
        self.band0_g = self.config.getfloat(section, 'band0')
        self.band1_g = self.config.getfloat(section, 'band1')
        self.band2_g = self.config.getfloat(section, 'band2')
        self.band3_g = self.config.getfloat(section, 'band3')
        self.band4_g = self.config.getfloat(section, 'band4')
        self.band5_g = self.config.getfloat(section, 'band5')
        self.band6_g = self.config.getfloat(section, 'band6')
        self.band7_g = self.config.getfloat(section, 'band7')
        self.band8_g = self.config.getfloat(section, 'band8')
        self.band9_g = self.config.getfloat(section, 'band9')
        self.band10_g = self.config.getfloat(section, 'band10')
        self.band11_g = self.config.getfloat(section, 'band11')
        self.band12_g = self.config.getfloat(section, 'band12')
        self.band13_g = self.config.getfloat(section, 'band13')
        self.band14_g = self.config.getfloat(section, 'band14')

        self.band0_f = self.config.getfloat(section, 'band0_freq', fallback=26)
        self.band1_f = self.config.getfloat(section, 'band1_freq', fallback=41)
        self.band2_f = self.config.getfloat(section, 'band2_freq', fallback=65)
        self.band3_f = self.config.getfloat(section, 'band3_freq',
                                            fallback=103)
        self.band4_f = self.config.getfloat(section, 'band4_freq',
                                            fallback=163)
        self.band5_f = self.config.getfloat(section, 'band5_freq',
                                            fallback=259)
        self.band6_f = self.config.getfloat(section, 'band6_freq',
                                            fallback=410)
        self.band7_f = self.config.getfloat(section, 'band7_freq',
                                            fallback=649)
        self.band8_f = self.config.getfloat(section, 'band8_freq',
                                            fallback=1029)
        self.band9_f = self.config.getfloat(section, 'band9_freq',
                                            fallback=1631)
        self.band10_f = self.config.getfloat(section, 'band10_freq',
                                             fallback=2585)
        self.band11_f = self.config.getfloat(section, 'band11_freq',
                                             fallback=4097)
        self.band12_f = self.config.getfloat(section, 'band12_freq',
                                             fallback=6493)
        self.band13_f = self.config.getfloat(section, 'band13_freq',
                                             fallback=10291)
        self.band14_f = self.config.getfloat(section, 'band14_freq',
                                             fallback=16310)

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

        self.band0_t = self.config.getfloat(section, 'band0_type', fallback=1)
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
                                             fallback=2)

        settings.set_value('equalizer-state', GLib.Variant('b', enabled))
        settings.set_value('equalizer-input-gain',
                           GLib.Variant('d', input_gain))
        settings.set_value('equalizer-output-gain',
                           GLib.Variant('d', output_gain))

        for n in range(15):
            g = GLib.Variant('d', getattr(self, 'band' + str(n) + '_g'))
            f = GLib.Variant('d', getattr(self, 'band' + str(n) + '_f'))
            q = GLib.Variant('d', getattr(self, 'band' + str(n) + '_q'))
            t = GLib.Variant('i', getattr(self, 'band' + str(n) + '_t'))

            settings.set_value('equalizer-band' + str(n) + '-gain', g)
            settings.set_value('equalizer-band' + str(n) + '-frequency', f)
            settings.set_value('equalizer-band' + str(n) + '-quality', q)
            settings.set_value('equalizer-band' + str(n) + '-type', t)

    def load_sink_inputs_presets(self, settings):
        # order is important
        self.load_limiter_presets(settings, 'apps_limiter')
        self.load_autovolume_presets(settings, 'apps_autovolume')
        self.load_panorama_presets(settings, 'apps_panorama')
        self.load_compressor_presets(settings, 'apps_compressor')
        self.load_reverb_presets(settings, 'apps_reverb')
        self.load_highpass_presets(settings, 'apps_highpass')
        self.load_lowpass_presets(settings, 'apps_lowpass')
        self.load_equalizer_presets(settings, 'apps_equalizer')

    def load_source_outputs_presets(self, settings):
        # order is important
        self.load_limiter_presets(settings, 'mic_limiter')
        self.load_autovolume_presets(settings, 'mic_autovolume')
        self.load_compressor_presets(settings, 'mic_compressor')
        self.load_reverb_presets(settings, 'mic_reverb')
        self.load_highpass_presets(settings, 'mic_highpass')
        self.load_lowpass_presets(settings, 'mic_lowpass')
        self.load_equalizer_presets(settings, 'mic_equalizer')
