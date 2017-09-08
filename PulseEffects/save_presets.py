# -*- coding: utf-8 -*-

import configparser


class SavePresets():

    def __init__(self, presets_path):
        self.config = configparser.ConfigParser()

        self.output_file = open(presets_path, 'w')

    def save_limiter_presets(self, settings, section):
        enabled = settings.get_value('limiter-state')
        limiter = settings.get_value('limiter-user')

        self.config[section] = {'enabled': str(enabled),
                                'input gain': str(limiter[0]),
                                'limit': str(limiter[1]),
                                'release time': str(limiter[2])}

    def save_panorama_presets(self, settings, section):
        enabled = settings.get_value('panorama-state')
        position = settings.get_value('panorama-position')

        self.config[section] = {'enabled': str(enabled),
                                'position': str(position)}

    def save_compressor_presets(self, settings, section):
        enabled = settings.get_value('compressor-state')
        use_peak = settings.get_value('compressor-use-peak')
        attack = settings.get_value('compressor-attack')
        release = settings.get_value('compressor-release')
        threshold = settings.get_value('compressor-threshold')
        ratio = settings.get_value('compressor-ratio')
        knee = settings.get_value('compressor-knee')
        makeup = settings.get_value('compressor-makeup')

        self.config[section] = {'enabled': str(enabled),
                                'use_peak': str(use_peak),
                                'attack': str(attack),
                                'release': str(release),
                                'threshold': str(threshold),
                                'ratio': str(ratio),
                                'knee': str(knee),
                                'makeup': str(makeup)}

    def save_reverb_presets(self, settings, section):
        enabled = settings.get_value('reverb-state')
        room_size = settings.get_value('reverb-room-size')
        damping = settings.get_value('reverb-damping')
        width = settings.get_value('reverb-width')
        level = settings.get_value('reverb-level')

        self.config[section] = {'enabled': str(enabled),
                                'room size': str(room_size),
                                'damping': str(damping),
                                'width': str(width),
                                'level': str(level)}

    def save_highpass_presets(self, settings, section):
        enabled = settings.get_value('highpass-state')
        cutoff = settings.get_value('highpass-cutoff')
        poles = settings.get_value('highpass-poles')

        self.config[section] = {'enabled': str(enabled), 'cutoff': str(cutoff),
                                'poles': str(poles)}

    def save_lowpass_presets(self, settings, section):
        enabled = settings.get_value('lowpass-state')
        cutoff = settings.get_value('lowpass-cutoff')
        poles = settings.get_value('lowpass-poles')

        self.config[section] = {'enabled': str(enabled), 'cutoff': str(cutoff),
                                'poles': str(poles)}

    def save_equalizer_presets(self, settings, section):
        enabled = settings.get_value('equalizer-state')
        input_gain = settings.get_value('equalizer-input-gain')
        output_gain = settings.get_value('equalizer-output-gain')
        equalizer = settings.get_value('equalizer-user')
        eq_freqs = settings.get_value('equalizer-freqs')
        eq_qfactors = settings.get_value('equalizer-qfactors')

        self.config[section] = {'enabled': str(enabled),
                                'input_gain': str(input_gain),
                                'output_gain': str(output_gain),
                                'band0': str(equalizer[0]),
                                'band1': str(equalizer[1]),
                                'band2': str(equalizer[2]),
                                'band3': str(equalizer[3]),
                                'band4': str(equalizer[4]),
                                'band5': str(equalizer[5]),
                                'band6': str(equalizer[6]),
                                'band7': str(equalizer[7]),
                                'band8': str(equalizer[8]),
                                'band9': str(equalizer[9]),
                                'band10': str(equalizer[10]),
                                'band11': str(equalizer[11]),
                                'band12': str(equalizer[12]),
                                'band13': str(equalizer[13]),
                                'band14': str(equalizer[14]),
                                'band0_freq': str(eq_freqs[0]),
                                'band1_freq': str(eq_freqs[1]),
                                'band2_freq': str(eq_freqs[2]),
                                'band3_freq': str(eq_freqs[3]),
                                'band4_freq': str(eq_freqs[4]),
                                'band5_freq': str(eq_freqs[5]),
                                'band6_freq': str(eq_freqs[6]),
                                'band7_freq': str(eq_freqs[7]),
                                'band8_freq': str(eq_freqs[8]),
                                'band9_freq': str(eq_freqs[9]),
                                'band10_freq': str(eq_freqs[10]),
                                'band11_freq': str(eq_freqs[11]),
                                'band12_freq': str(eq_freqs[12]),
                                'band13_freq': str(eq_freqs[13]),
                                'band14_freq': str(eq_freqs[14]),
                                'band0_qfactor': str(eq_qfactors[0]),
                                'band1_qfactor': str(eq_qfactors[1]),
                                'band2_qfactor': str(eq_qfactors[2]),
                                'band3_qfactor': str(eq_qfactors[3]),
                                'band4_qfactor': str(eq_qfactors[4]),
                                'band5_qfactor': str(eq_qfactors[5]),
                                'band6_qfactor': str(eq_qfactors[6]),
                                'band7_qfactor': str(eq_qfactors[7]),
                                'band8_qfactor': str(eq_qfactors[8]),
                                'band9_qfactor': str(eq_qfactors[9]),
                                'band10_qfactor':
                                str(eq_qfactors[10]),
                                'band11_qfactor':
                                str(eq_qfactors[11]),
                                'band12_qfactor':
                                str(eq_qfactors[12]),
                                'band13_qfactor':
                                str(eq_qfactors[13]),
                                'band14_qfactor':
                                str(eq_qfactors[14])}

    def save_sink_inputs_presets(self, settings):
        self.save_limiter_presets(settings, 'apps_limiter')
        self.save_panorama_presets(settings, 'apps_panorama')
        self.save_compressor_presets(settings, 'apps_compressor')
        self.save_reverb_presets(settings, 'apps_reverb')
        self.save_highpass_presets(settings, 'apps_highpass')
        self.save_lowpass_presets(settings, 'apps_lowpass')
        self.save_equalizer_presets(settings, 'apps_equalizer')

    def save_source_outputs_presets(self, settings):
        self.save_limiter_presets(settings, 'mic_limiter')
        self.save_compressor_presets(settings, 'mic_compressor')
        self.save_reverb_presets(settings, 'mic_reverb')
        self.save_highpass_presets(settings, 'mic_highpass')
        self.save_lowpass_presets(settings, 'mic_lowpass')
        self.save_equalizer_presets(settings, 'mic_equalizer')

    def write_config(self):
        self.config.write(self.output_file)
