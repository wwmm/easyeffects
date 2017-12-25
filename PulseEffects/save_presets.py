# -*- coding: utf-8 -*-

import configparser


class SavePresets():

    def __init__(self):
        self.config = configparser.ConfigParser()

    def set_output_path(self, path):
        self.output_file = open(path, 'w')

    def save_limiter_presets(self, settings, section):
        enabled = settings.get_value('limiter-state')
        input_gain = settings.get_value('limiter-input-gain')
        limit = settings.get_value('limiter-limit')
        release_time = settings.get_value('limiter-release-time')

        self.config[section] = {'enabled': str(enabled),
                                'input gain': str(input_gain),
                                'limit': str(limit),
                                'release time': str(release_time)}

    def save_autovolume_presets(self, settings, section):
        enabled = settings.get_value('autovolume-state')
        window = settings.get_value('autovolume-window')
        target = settings.get_value('autovolume-target')
        tolerance = settings.get_value('autovolume-tolerance')
        threshold = settings.get_value('autovolume-threshold')

        self.config[section] = {'enabled': str(enabled),
                                'window': str(window),
                                'target': str(target),
                                'tolerance': str(tolerance),
                                'threshold': str(threshold)}

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

        gain, frequencies, qualities, types = [], [], [], []

        for n in range(30):
            gain.append(
                settings.get_value('equalizer-band' + str(n) + '-gain'))

            frequencies.append(
                settings.get_value('equalizer-band' + str(n) + '-frequency'))

            qualities.append(
                settings.get_value('equalizer-band' + str(n) + '-quality'))

            types.append(
                settings.get_value('equalizer-band' + str(n) + '-type'))

        self.config[section] = {'enabled': str(enabled),
                                'input_gain': str(input_gain),
                                'output_gain': str(output_gain),
                                'band0': str(gain[0]),
                                'band1': str(gain[1]),
                                'band2': str(gain[2]),
                                'band3': str(gain[3]),
                                'band4': str(gain[4]),
                                'band5': str(gain[5]),
                                'band6': str(gain[6]),
                                'band7': str(gain[7]),
                                'band8': str(gain[8]),
                                'band9': str(gain[9]),
                                'band10': str(gain[10]),
                                'band11': str(gain[11]),
                                'band12': str(gain[12]),
                                'band13': str(gain[13]),
                                'band14': str(gain[14]),
                                'band15': str(gain[15]),
                                'band16': str(gain[16]),
                                'band17': str(gain[17]),
                                'band18': str(gain[18]),
                                'band19': str(gain[19]),
                                'band20': str(gain[20]),
                                'band21': str(gain[21]),
                                'band22': str(gain[22]),
                                'band23': str(gain[23]),
                                'band24': str(gain[24]),
                                'band25': str(gain[25]),
                                'band26': str(gain[26]),
                                'band27': str(gain[27]),
                                'band28': str(gain[28]),
                                'band29': str(gain[29]),
                                'band0_freq': str(frequencies[0]),
                                'band1_freq': str(frequencies[1]),
                                'band2_freq': str(frequencies[2]),
                                'band3_freq': str(frequencies[3]),
                                'band4_freq': str(frequencies[4]),
                                'band5_freq': str(frequencies[5]),
                                'band6_freq': str(frequencies[6]),
                                'band7_freq': str(frequencies[7]),
                                'band8_freq': str(frequencies[8]),
                                'band9_freq': str(frequencies[9]),
                                'band10_freq': str(frequencies[10]),
                                'band11_freq': str(frequencies[11]),
                                'band12_freq': str(frequencies[12]),
                                'band13_freq': str(frequencies[13]),
                                'band14_freq': str(frequencies[14]),
                                'band15_freq': str(frequencies[15]),
                                'band16_freq': str(frequencies[16]),
                                'band17_freq': str(frequencies[17]),
                                'band18_freq': str(frequencies[18]),
                                'band19_freq': str(frequencies[19]),
                                'band20_freq': str(frequencies[20]),
                                'band21_freq': str(frequencies[21]),
                                'band22_freq': str(frequencies[22]),
                                'band23_freq': str(frequencies[23]),
                                'band24_freq': str(frequencies[24]),
                                'band25_freq': str(frequencies[25]),
                                'band26_freq': str(frequencies[26]),
                                'band27_freq': str(frequencies[27]),
                                'band28_freq': str(frequencies[28]),
                                'band29_freq': str(frequencies[29]),
                                'band0_qfactor': str(qualities[0]),
                                'band1_qfactor': str(qualities[1]),
                                'band2_qfactor': str(qualities[2]),
                                'band3_qfactor': str(qualities[3]),
                                'band4_qfactor': str(qualities[4]),
                                'band5_qfactor': str(qualities[5]),
                                'band6_qfactor': str(qualities[6]),
                                'band7_qfactor': str(qualities[7]),
                                'band8_qfactor': str(qualities[8]),
                                'band9_qfactor': str(qualities[9]),
                                'band10_qfactor': str(qualities[10]),
                                'band11_qfactor': str(qualities[11]),
                                'band12_qfactor': str(qualities[12]),
                                'band13_qfactor': str(qualities[13]),
                                'band14_qfactor': str(qualities[14]),
                                'band15_qfactor': str(qualities[15]),
                                'band16_qfactor': str(qualities[16]),
                                'band17_qfactor': str(qualities[17]),
                                'band18_qfactor': str(qualities[18]),
                                'band19_qfactor': str(qualities[19]),
                                'band20_qfactor': str(qualities[20]),
                                'band21_qfactor': str(qualities[21]),
                                'band22_qfactor': str(qualities[22]),
                                'band23_qfactor': str(qualities[23]),
                                'band24_qfactor': str(qualities[24]),
                                'band25_qfactor': str(qualities[25]),
                                'band26_qfactor': str(qualities[26]),
                                'band27_qfactor': str(qualities[27]),
                                'band28_qfactor': str(qualities[28]),
                                'band29_qfactor': str(qualities[29]),
                                'band0_type': str(types[0]),
                                'band1_type': str(types[1]),
                                'band2_type': str(types[2]),
                                'band3_type': str(types[3]),
                                'band4_type': str(types[4]),
                                'band5_type': str(types[5]),
                                'band6_type': str(types[6]),
                                'band7_type': str(types[7]),
                                'band8_type': str(types[8]),
                                'band9_type': str(types[9]),
                                'band10_type': str(types[10]),
                                'band11_type': str(types[11]),
                                'band12_type': str(types[12]),
                                'band13_type': str(types[13]),
                                'band14_type': str(types[14]),
                                'band15_type': str(types[15]),
                                'band16_type': str(types[16]),
                                'band17_type': str(types[17]),
                                'band18_type': str(types[18]),
                                'band19_type': str(types[19]),
                                'band20_type': str(types[20]),
                                'band21_type': str(types[21]),
                                'band22_type': str(types[22]),
                                'band23_type': str(types[23]),
                                'band24_type': str(types[24]),
                                'band25_type': str(types[25]),
                                'band26_type': str(types[26]),
                                'band27_type': str(types[27]),
                                'band28_type': str(types[28]),
                                'band29_type': str(types[29])}

    def save_exciter_presets(self, settings, section):
        enabled = settings.get_value('exciter-state')
        input_gain = settings.get_value('exciter-input-gain')
        output_gain = settings.get_value('exciter-output-gain')
        amount = settings.get_value('exciter-amount')
        harmonics = settings.get_value('exciter-harmonics')
        scope = settings.get_value('exciter-scope')
        ceiling = settings.get_value('exciter-ceiling')
        blend = settings.get_value('exciter-blend')

        self.config[section] = {'enabled': str(enabled),
                                'input_gain': str(input_gain),
                                'output_gain': str(output_gain),
                                'amount': str(amount),
                                'harmonics': str(harmonics),
                                'scope': str(scope),
                                'floor': str(ceiling),
                                'blend': str(blend)}

    def save_bass_enhancer_presets(self, settings, section):
        enabled = settings.get_value('bass-enhancer-state')
        input_gain = settings.get_value('bass-enhancer-input-gain')
        output_gain = settings.get_value('bass-enhancer-output-gain')
        amount = settings.get_value('bass-enhancer-amount')
        harmonics = settings.get_value('bass-enhancer-harmonics')
        scope = settings.get_value('bass-enhancer-scope')
        floor = settings.get_value('bass-enhancer-floor')
        blend = settings.get_value('bass-enhancer-blend')

        self.config[section] = {'enabled': str(enabled),
                                'input_gain': str(input_gain),
                                'output_gain': str(output_gain),
                                'amount': str(amount),
                                'harmonics': str(harmonics),
                                'scope': str(scope),
                                'floor': str(floor),
                                'blend': str(blend)}

    def save_stereo_enhancer_presets(self, settings, section):
        enabled = settings.get_value('stereo-enhancer-state')
        input_gain = settings.get_value('stereo-enhancer-input-gain')
        output_gain = settings.get_value('stereo-enhancer-output-gain')

        left_invert_phase = settings.get_value(
            'stereo-enhancer-left-invert-phase')
        left_balance = settings.get_value('stereo-enhancer-left-balance')
        left_delay = settings.get_value('stereo-enhancer-left-delay')
        left_gain = settings.get_value('stereo-enhancer-left-gain')

        right_invert_phase = settings.get_value(
            'stereo-enhancer-right-invert-phase')
        right_balance = settings.get_value('stereo-enhancer-right-balance')
        right_delay = settings.get_value('stereo-enhancer-right-delay')
        right_gain = settings.get_value('stereo-enhancer-right-gain')

        middle_invert_phase = settings.get_value(
            'stereo-enhancer-middle-invert-phase')
        middle_gain = settings.get_value('stereo-enhancer-middle-gain')
        middle_source = settings.get_value('stereo-enhancer-middle-source')

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
                                'middle_gain': str(middle_gain),
                                'middle_source': str(middle_source)}

    def save_maximizer_presets(self, settings, section):
        enabled = settings.get_value('maximizer-state')
        release = settings.get_value('maximizer-release')
        ceiling = settings.get_value('maximizer-ceiling')
        threshold = settings.get_value('maximizer-threshold')

        self.config[section] = {'enabled': str(enabled),
                                'release': str(release),
                                'ceiling': str(ceiling),
                                'threshold': str(threshold)}

    def save_output_limiter_presets(self, settings, section):
        enabled = settings.get_value('output-limiter-state')
        input_gain = settings.get_value('output-limiter-input-gain')
        limit = settings.get_value('output-limiter-limit')
        release_time = settings.get_value('output-limiter-release-time')

        self.config[section] = {'enabled': str(enabled),
                                'input gain': str(input_gain),
                                'limit': str(limit),
                                'release time': str(release_time)}

    def save_sink_inputs_presets(self, settings):
        self.save_limiter_presets(settings, 'apps_limiter')
        self.save_autovolume_presets(settings, 'apps_autovolume')
        self.save_panorama_presets(settings, 'apps_panorama')
        self.save_compressor_presets(settings, 'apps_compressor')
        self.save_reverb_presets(settings, 'apps_reverb')
        self.save_highpass_presets(settings, 'apps_highpass')
        self.save_lowpass_presets(settings, 'apps_lowpass')
        self.save_equalizer_presets(settings, 'apps_equalizer')
        self.save_exciter_presets(settings, 'apps_exciter')
        self.save_bass_enhancer_presets(settings, 'apps_bass_enhancer')
        self.save_stereo_enhancer_presets(settings, 'apps_stereo_enhancer')
        self.save_maximizer_presets(settings, 'apps_maximizer')
        self.save_output_limiter_presets(settings, 'apps_output_limiter')

    def save_source_outputs_presets(self, settings):
        self.save_limiter_presets(settings, 'mic_limiter')
        self.save_autovolume_presets(settings, 'mic_autovolume')
        self.save_compressor_presets(settings, 'mic_compressor')
        self.save_reverb_presets(settings, 'mic_reverb')
        self.save_highpass_presets(settings, 'mic_highpass')
        self.save_lowpass_presets(settings, 'mic_lowpass')
        self.save_equalizer_presets(settings, 'mic_equalizer')

    def write_config(self):
        self.config.write(self.output_file)
        self.output_file.close()
