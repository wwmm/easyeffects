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

        band0 = self.config.getfloat(section, 'band0')
        band1 = self.config.getfloat(section, 'band1')
        band2 = self.config.getfloat(section, 'band2')
        band3 = self.config.getfloat(section, 'band3')
        band4 = self.config.getfloat(section, 'band4')
        band5 = self.config.getfloat(section, 'band5')
        band6 = self.config.getfloat(section, 'band6')
        band7 = self.config.getfloat(section, 'band7')
        band8 = self.config.getfloat(section, 'band8')
        band9 = self.config.getfloat(section, 'band9')
        band10 = self.config.getfloat(section, 'band10')
        band11 = self.config.getfloat(section, 'band11')
        band12 = self.config.getfloat(section, 'band12')
        band13 = self.config.getfloat(section, 'band13')
        band14 = self.config.getfloat(section, 'band14')

        band0_freq = self.config.getfloat(section, 'band0_freq', fallback=26)
        band1_freq = self.config.getfloat(section, 'band1_freq', fallback=41)
        band2_freq = self.config.getfloat(section, 'band2_freq', fallback=65)
        band3_freq = self.config.getfloat(section, 'band3_freq', fallback=103)
        band4_freq = self.config.getfloat(section, 'band4_freq', fallback=163)
        band5_freq = self.config.getfloat(section, 'band5_freq', fallback=259)
        band6_freq = self.config.getfloat(section, 'band6_freq', fallback=410)
        band7_freq = self.config.getfloat(section, 'band7_freq', fallback=649)
        band8_freq = self.config.getfloat(section, 'band8_freq', fallback=1029)
        band9_freq = self.config.getfloat(section, 'band9_freq', fallback=1631)
        band10_freq = self.config.getfloat(section, 'band10_freq',
                                           fallback=2585)
        band11_freq = self.config.getfloat(section, 'band11_freq',
                                           fallback=4097)
        band12_freq = self.config.getfloat(section, 'band12_freq',
                                           fallback=6493)
        band13_freq = self.config.getfloat(section, 'band13_freq',
                                           fallback=10291)
        band14_freq = self.config.getfloat(section, 'band14_freq',
                                           fallback=16310)

        band0_qfactor = self.config.getfloat(section, 'band0_qfactor',
                                             fallback=2.21)
        band1_qfactor = self.config.getfloat(section, 'band1_qfactor',
                                             fallback=2.21)
        band2_qfactor = self.config.getfloat(section, 'band2_qfactor',
                                             fallback=2.21)
        band3_qfactor = self.config.getfloat(section, 'band3_qfactor',
                                             fallback=2.21)
        band4_qfactor = self.config.getfloat(section, 'band4_qfactor',
                                             fallback=2.21)
        band5_qfactor = self.config.getfloat(section, 'band5_qfactor',
                                             fallback=2.21)
        band6_qfactor = self.config.getfloat(section, 'band6_qfactor',
                                             fallback=2.21)
        band7_qfactor = self.config.getfloat(section, 'band7_qfactor',
                                             fallback=2.21)
        band8_qfactor = self.config.getfloat(section, 'band8_qfactor',
                                             fallback=2.21)
        band9_qfactor = self.config.getfloat(section, 'band9_qfactor',
                                             fallback=2.21)
        band10_qfactor = self.config.getfloat(section, 'band10_qfactor',
                                              fallback=2.21)
        band11_qfactor = self.config.getfloat(section, 'band11_qfactor',
                                              fallback=2.21)
        band12_qfactor = self.config.getfloat(section, 'band12_qfactor',
                                              fallback=2.21)
        band13_qfactor = self.config.getfloat(section, 'band13_qfactor',
                                              fallback=2.21)
        band14_qfactor = self.config.getfloat(section, 'band14_qfactor',
                                              fallback=2.21)

        equalizer_bands = [band0, band1, band2, band3, band4, band5, band6,
                           band7, band8, band9, band10, band11, band12, band13,
                           band14]

        eq_freqs = [band0_freq, band1_freq, band2_freq, band3_freq, band4_freq,
                    band5_freq, band6_freq, band7_freq, band8_freq, band9_freq,
                    band10_freq, band11_freq, band12_freq, band13_freq,
                    band14_freq]

        eq_qfactors = [band0_qfactor, band1_qfactor, band2_qfactor,
                       band3_qfactor, band4_qfactor, band5_qfactor,
                       band6_qfactor, band7_qfactor, band8_qfactor,
                       band9_qfactor, band10_qfactor, band11_qfactor,
                       band12_qfactor, band13_qfactor, band14_qfactor]

        settings.set_value('equalizer-state', GLib.Variant('b', enabled))
        settings.set_value('equalizer-input-gain',
                           GLib.Variant('d', input_gain))
        settings.set_value('equalizer-output-gain',
                           GLib.Variant('d', output_gain))
        settings.set_value('equalizer-user',
                           GLib.Variant('ad', equalizer_bands))
        settings.set_value('equalizer-freqs',
                           GLib.Variant('ad', eq_freqs))
        settings.set_value('equalizer-qfactors',
                           GLib.Variant('ad', eq_qfactors))

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
