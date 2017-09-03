# -*- coding: utf-8 -*-

import configparser


class SavePresets():

    def __init__(self, presets_path):
        self.config = configparser.ConfigParser()

        self.output_file = open(presets_path, 'w')

    def save_sink_inputs_presets(self, settings):
        limiter = settings.get_value('limiter-user')

        self.config['apps_limiter'] = {'input gain': str(limiter[0]),
                                       'limit': str(limiter[1]),
                                       'release time': str(limiter[2])}

        panorama_position = settings.get_value('panorama-position')

        self.config['apps_panorama'] = {'position':
                                        str(panorama_position)}

        compressor = settings.get_value('compressor-user')

        self.config['apps_compressor'] = {'rms-peak': str(compressor[0]),
                                          'attack': str(compressor[1]),
                                          'release': str(compressor[2]),
                                          'threshold': str(compressor[3]),
                                          'ratio': str(compressor[4]),
                                          'knee': str(compressor[5]),
                                          'makeup': str(compressor[6])}

        reverb = settings.get_value('reverb-user')

        self.config['apps_reverb'] = {'room size': str(reverb[0]),
                                      'damping': str(reverb[1]),
                                      'width': str(reverb[2]),
                                      'level': str(reverb[3])}

        highpass_cutoff = settings.get_value('highpass-cutoff')
        highpass_poles = settings.get_value('highpass-poles')

        self.config['apps_highpass'] = {'cutoff': str(highpass_cutoff),
                                        'poles': str(highpass_poles)}

        lowpass_cutoff = settings.get_value('lowpass-cutoff')
        lowpass_poles = settings.get_value('lowpass-poles')

        self.config['apps_lowpass'] = {'cutoff': str(lowpass_cutoff),
                                       'poles': str(lowpass_poles)}

        equalizer_input_gain = settings.get_value(
            'equalizer-input-gain')
        equalizer_output_gain = settings.get_value(
            'equalizer-output-gain')
        equalizer = settings.get_value('equalizer-user')
        eq_freqs = settings.get_value('equalizer-freqs')
        eq_qfactors = settings.get_value('equalizer-qfactors')

        self.config['apps_equalizer'] = {'input_gain':
                                         str(equalizer_input_gain),
                                         'output_gain':
                                         str(equalizer_output_gain),
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

    def save_source_outputs_presets(self, settings):
        limiter = settings.get_value('limiter-user')

        self.config['mic_limiter'] = {'input gain': str(limiter[0]),
                                      'limit': str(limiter[1]),
                                      'release time': str(limiter[2])}

        compressor = settings.get_value('compressor-user')

        self.config['mic_compressor'] = {'rms-peak': str(compressor[0]),
                                         'attack': str(compressor[1]),
                                         'release': str(compressor[2]),
                                         'threshold': str(compressor[3]),
                                         'ratio': str(compressor[4]),
                                         'knee': str(compressor[5]),
                                         'makeup': str(compressor[6])}

        reverb = settings.get_value('reverb-user')

        self.config['mic_reverb'] = {'room size': str(reverb[0]),
                                     'damping': str(reverb[1]),
                                     'width': str(reverb[2]),
                                     'level': str(reverb[3])}

        highpass_cutoff = settings.get_value('highpass-cutoff')
        highpass_poles = settings.get_value('highpass-poles')

        self.config['mic_highpass'] = {'cutoff': str(highpass_cutoff),
                                       'poles': str(highpass_poles)}

        lowpass_cutoff = settings.get_value('lowpass-cutoff')
        lowpass_poles = settings.get_value('lowpass-poles')

        self.config['mic_lowpass'] = {'cutoff': str(lowpass_cutoff),
                                      'poles': str(lowpass_poles)}

        equalizer_input_gain = settings.get_value(
            'equalizer-input-gain')
        equalizer_output_gain = settings.get_value(
            'equalizer-output-gain')
        equalizer = settings.get_value('equalizer-user')
        eq_freqs = settings.get_value('equalizer-freqs')
        eq_qfactors = settings.get_value('equalizer-qfactors')

        self.config['mic_equalizer'] = {'input_gain':
                                        str(equalizer_input_gain),
                                        'output_gain':
                                        str(equalizer_output_gain),
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
                                        'band10_qfactor': str(eq_qfactors[10]),
                                        'band11_qfactor': str(eq_qfactors[11]),
                                        'band12_qfactor': str(eq_qfactors[12]),
                                        'band13_qfactor': str(eq_qfactors[13]),
                                        'band14_qfactor': str(eq_qfactors[14])}

    def write_config(self):
        self.config.write(self.output_file)
