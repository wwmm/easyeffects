# -*- coding: utf-8 -*-

import configparser

from PulseEffects.bass_enhancer_presets import BassEnhancerPresets
from PulseEffects.compressor_presets import CompressorPresets
from PulseEffects.delay_presets import DelayPresets
from PulseEffects.equalizer_presets import EqualizerPresets
from PulseEffects.exciter_presets import ExciterPresets
from PulseEffects.highpass_presets import HighpassPresets
from PulseEffects.limiter_presets import LimiterPresets
from PulseEffects.lowpass_presets import LowpassPresets
from PulseEffects.panorama_presets import PanoramaPresets
from PulseEffects.reverb_presets import ReverbPresets


class SavePresets():

    def __init__(self):
        self.config = configparser.ConfigParser()

        self.limiter_presets = LimiterPresets(self.config)
        self.panorama_presets = PanoramaPresets(self.config)
        self.compressor_presets = CompressorPresets(self.config)
        self.reverb_presets = ReverbPresets(self.config)
        self.highpass_presets = HighpassPresets(self.config)
        self.lowpass_presets = LowpassPresets(self.config)
        self.equalizer_presets = EqualizerPresets(self.config)
        self.exciter_presets = ExciterPresets(self.config)
        self.bass_enhancer_presets = BassEnhancerPresets(self.config)
        self.delay_presets = DelayPresets(self.config)

    def set_output_path(self, path):
        self.output_file = open(path, 'w')

    def save_stereo_enhancer_preset(self, settings, section):
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
        middle_source = settings.get_value('stereo-enhancer-middle-source')

        side_gain = settings.get_value('stereo-enhancer-side-gain')

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

    def save_stereo_spread_preset(self, settings, section):
        enabled = settings.get_value('stereo-spread-state')
        input_gain = settings.get_value('stereo-spread-input-gain')
        output_gain = settings.get_value('stereo-spread-output-gain')

        amount0 = settings.get_value('stereo-spread-amount0')
        amount1 = settings.get_value('stereo-spread-amount1')
        amount2 = settings.get_value('stereo-spread-amount2')
        amount3 = settings.get_value('stereo-spread-amount3')
        filters = settings.get_value('stereo-spread-filters')
        mono = settings.get_value('stereo-spread-mono')

        self.config[section] = {'enabled': str(enabled),
                                'input_gain': str(input_gain),
                                'output_gain': str(output_gain),
                                'amount0': str(amount0),
                                'amount1': str(amount1),
                                'amount2': str(amount2),
                                'amount3': str(amount3),
                                'filters': str(filters),
                                'mono': str(mono)}

    def save_crossfeed_preset(self, settings, section):
        enabled = settings.get_value('crossfeed-state')
        fcut = settings.get_value('crossfeed-fcut')
        feed = settings.get_value('crossfeed-feed')

        self.config[section] = {'enabled': str(enabled),
                                'fcut': str(fcut),
                                'feed': str(feed)}

    def save_maximizer_preset(self, settings, section):
        enabled = settings.get_value('maximizer-state')
        release = settings.get_value('maximizer-release')
        ceiling = settings.get_value('maximizer-ceiling')
        threshold = settings.get_value('maximizer-threshold')

        self.config[section] = {'enabled': str(enabled),
                                'release': str(release),
                                'ceiling': str(ceiling),
                                'threshold': str(threshold)}

    def save_output_limiter_preset(self, settings, section):
        enabled = settings.get_value('output-limiter-state')
        input_gain = settings.get_value('output-limiter-input-gain')
        limit = settings.get_value('output-limiter-limit')
        release_time = settings.get_value('output-limiter-release-time')

        self.config[section] = {'enabled': str(enabled),
                                'input gain': str(input_gain),
                                'limit': str(limit),
                                'release time': str(release_time)}

    def save_pitch_preset(self, settings, section):
        enabled = settings.get_value('pitch-state')
        cents = settings.get_value('pitch-cents')
        semitones = settings.get_value('pitch-semitones')
        octaves = settings.get_value('pitch-octaves')
        crispness = settings.get_value('pitch-crispness')
        faster = settings.get_value('pitch-faster')
        preserve_formant = settings.get_value('pitch-preserve-formant')

        self.config[section] = {'enabled': str(enabled),
                                'cents': str(cents),
                                'semitones': str(semitones),
                                'octaves': str(octaves),
                                'crispness': str(crispness),
                                'faster': str(faster),
                                'preserve_formant': str(preserve_formant)}

    def save_gate_preset(self, settings, section):
        enabled = settings.get_value('gate-state')
        detection = settings.get_value('gate-detection-rms')
        stereo_link = settings.get_value('gate-stereo-link-average')
        range = settings.get_value('gate-range')
        attack = settings.get_value('gate-attack')
        release = settings.get_value('gate-release')
        threshold = settings.get_value('gate-threshold')
        ratio = settings.get_value('gate-ratio')
        knee = settings.get_value('gate-knee')
        makeup = settings.get_value('gate-makeup')

        self.config[section] = {'enabled': str(enabled),
                                'detection_type_rms': str(detection),
                                'stereo_link_type_average': str(stereo_link),
                                'range': str(range),
                                'attack': str(attack),
                                'release': str(release),
                                'threshold': str(threshold),
                                'ratio': str(ratio),
                                'knee': str(knee),
                                'makeup': str(makeup)}

    def save_deesser_preset(self, settings, section):
        enabled = settings.get_value('deesser-state')
        detection = settings.get_value('deesser-detection-rms')
        mode = settings.get_value('deesser-mode-wide')
        threshold = settings.get_value('deesser-threshold')
        ratio = settings.get_value('deesser-ratio')
        makeup = settings.get_value('deesser-makeup')
        laxity = settings.get_value('deesser-laxity')
        f1 = settings.get_value('deesser-f1')
        f1_level = settings.get_value('deesser-f1-level')
        f2 = settings.get_value('deesser-f2')
        f2_level = settings.get_value('deesser-f2-level')
        f2_q = settings.get_value('deesser-f2-q')

        self.config[section] = {'enabled': str(enabled),
                                'detection_type_rms': str(detection),
                                'mode_type_wide': str(mode),
                                'threshold': str(threshold),
                                'ratio': str(ratio),
                                'makeup': str(makeup),
                                'laxity': str(laxity),
                                'f1': str(f1),
                                'f1_level': str(f1_level),
                                'f2': str(f2),
                                'f2_level': str(f2_level),
                                'f2_q': str(f2_q)}

    def save_sink_inputs_preset(self, settings):
        self.save_stereo_enhancer_preset(settings, 'apps_stereo_enhancer')
        self.save_stereo_spread_preset(settings, 'apps_stereo_spread')
        self.save_crossfeed_preset(settings, 'apps_crossfeed')
        self.save_maximizer_preset(settings, 'apps_maximizer')
        self.save_output_limiter_preset(settings, 'apps_output_limiter')

    def save_source_outputs_preset(self, settings):
        self.save_pitch_preset(settings, 'mic_pitch')
        self.save_gate_preset(settings, 'mic_gate')
        self.save_deesser_preset(settings, 'mic_deesser')

    def write_config(self):
        self.config.write(self.output_file)
        self.output_file.close()

    def save(self):
        self.limiter_presets.save()
        self.panorama_presets.save()
        self.compressor_presets.save()
        self.reverb_presets.save()
        self.highpass_presets.save()
        self.lowpass_presets.save()
        self.equalizer_presets.save()
        self.exciter_presets.save()
        self.bass_enhancer_presets.save()
        self.delay_presets.save()
