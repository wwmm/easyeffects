# -*- coding: utf-8 -*-

import configparser

from gi.repository import GLib
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


class LoadPresets():

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

    def set_config_path(self, path):
        self.config.clear()
        self.config.read(path)

    def load_stereo_enhancer_preset(self, settings, section):
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

    def load_stereo_spread_preset(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        input_gain = self.config.getfloat(section, 'input_gain', fallback=0.0)
        output_gain = self.config.getfloat(section, 'output_gain',
                                           fallback=0.0)

        amount0 = self.config.getfloat(section, 'amount0', fallback=0.0)
        amount1 = self.config.getfloat(section, 'amount1', fallback=0.0)
        amount2 = self.config.getfloat(section, 'amount2', fallback=0.0)
        amount3 = self.config.getfloat(section, 'amount3', fallback=0.0)
        filters = self.config.getint(section, 'filters', fallback=2)
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

    def load_crossfeed_preset(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        fcut = self.config.getint(section, 'fcut', fallback=700)
        feed = self.config.getfloat(section, 'feed', fallback=4.5)

        settings.set_value('crossfeed-state', GLib.Variant('b', enabled))
        settings.set_value('crossfeed-fcut', GLib.Variant('i', fcut))
        settings.set_value('crossfeed-feed', GLib.Variant('d', feed))

    def load_maximizer_preset(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        release = self.config.getfloat(section, 'release', fallback=3.16)
        ceiling = self.config.getfloat(section, 'ceiling', fallback=0.0)
        threshold = self.config.getfloat(section, 'threshold', fallback=0.0)

        settings.set_value('maximizer-state', GLib.Variant('b', enabled))
        settings.set_value('maximizer-release', GLib.Variant('d', release))
        settings.set_value('maximizer-ceiling', GLib.Variant('d', ceiling))
        settings.set_value('maximizer-threshold', GLib.Variant('d', threshold))

    def load_output_limiter_preset(self, settings, section):
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

    def load_pitch_preset(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        cents = self.config.getfloat(section, 'cents', fallback=0)
        semitones = self.config.getint(section, 'semitones', fallback=0)
        octaves = self.config.getint(section, 'octaves', fallback=0)
        crispness = self.config.getint(section, 'crispness', fallback=3)
        faster = self.config.getboolean(section, 'faster', fallback=False)
        preserve_formant = self.config.getboolean(section, 'preserve_formant',
                                                  fallback=False)

        settings.set_value('pitch-state', GLib.Variant('b', enabled))
        settings.set_value('pitch-cents', GLib.Variant('d', cents))
        settings.set_value('pitch-semitones', GLib.Variant('i', semitones))
        settings.set_value('pitch-octaves', GLib.Variant('i', octaves))
        settings.set_value('pitch-crispness', GLib.Variant('i', crispness))
        settings.set_value('pitch-faster', GLib.Variant('b', faster))
        settings.set_value('pitch-preserve-formant',
                           GLib.Variant('b', preserve_formant))

    def load_gate_preset(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        detection = self.config.getboolean(section, 'detection_type_rms',
                                           fallback=True)
        stereo_link = self.config.getboolean(section,
                                             'stereo_link_type_average',
                                             fallback=True)
        attack = self.config.getfloat(section, 'attack', fallback=20.0)
        release = self.config.getfloat(section, 'release', fallback=250.0)
        threshold = self.config.getfloat(section, 'threshold', fallback=-18.0)
        ratio = self.config.getfloat(section, 'ratio', fallback=2.0)
        knee = self.config.getfloat(section, 'knee', fallback=9.0)
        makeup = self.config.getfloat(section, 'makeup', fallback=0.0)

        settings.set_value('gate-state', GLib.Variant('b', enabled))
        settings.set_value('gate-detection-rms', GLib.Variant('b', detection))
        settings.set_value('gate-stereo-link-average',
                           GLib.Variant('b', stereo_link))
        settings.set_value('gate-attack', GLib.Variant('d', attack))
        settings.set_value('gate-release', GLib.Variant('d', release))
        settings.set_value('gate-threshold',
                           GLib.Variant('d', threshold))
        settings.set_value('gate-ratio', GLib.Variant('d', ratio))
        settings.set_value('gate-knee', GLib.Variant('d', knee))
        settings.set_value('gate-makeup', GLib.Variant('d', makeup))

    def load_deesser_preset(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        detection = self.config.getboolean(section, 'detection_type_rms',
                                           fallback=True)
        mode = self.config.getboolean(section, 'mode_type_wide', fallback=True)
        threshold = self.config.getfloat(section, 'threshold', fallback=-18.0)
        ratio = self.config.getfloat(section, 'ratio', fallback=3.0)
        makeup = self.config.getfloat(section, 'makeup', fallback=0.0)
        laxity = self.config.getint(section, 'laxity', fallback=15)
        f1 = self.config.getfloat(section, 'f1', fallback=6000.0)
        f1_level = self.config.getfloat(section, 'f1_level', fallback=0.0)
        f2 = self.config.getfloat(section, 'f2', fallback=4500.0)
        f2_level = self.config.getfloat(section, 'f2_level', fallback=12.0)
        f2_q = self.config.getfloat(section, 'f2_q', fallback=1.0)

        settings.set_value('deesser-state', GLib.Variant('b', enabled))
        settings.set_value('deesser-detection-rms',
                           GLib.Variant('b', detection))
        settings.set_value('deesser-mode-wide', GLib.Variant('b', mode))
        settings.set_value('deesser-threshold', GLib.Variant('d', threshold))
        settings.set_value('deesser-ratio', GLib.Variant('d', ratio))
        settings.set_value('deesser-makeup', GLib.Variant('d', makeup))
        settings.set_value('deesser-laxity', GLib.Variant('i', laxity))
        settings.set_value('deesser-f1', GLib.Variant('d', f1))
        settings.set_value('deesser-f1-level', GLib.Variant('d', f1_level))
        settings.set_value('deesser-f2', GLib.Variant('d', f2))
        settings.set_value('deesser-f2-level', GLib.Variant('d', f2_level))
        settings.set_value('deesser-f2-q', GLib.Variant('d', f2_q))

    def load_sink_inputs_preset(self, settings):
        self.load_stereo_enhancer_preset(settings, 'apps_stereo_enhancer')
        self.load_stereo_spread_preset(settings, 'apps_stereo_spread')
        self.load_crossfeed_preset(settings, 'apps_crossfeed')
        self.load_maximizer_preset(settings, 'apps_maximizer')
        self.load_output_limiter_preset(settings, 'apps_output_limiter')

    def load_source_outputs_preset(self, settings):
        self.load_pitch_preset(settings, 'mic_pitch')
        self.load_gate_preset(settings, 'mic_gate')
        self.load_deesser_preset(settings, 'mic_deesser')

    def load(self):
        self.limiter_presets.load()
        self.panorama_presets.load()
        self.compressor_presets.load()
        self.reverb_presets.load()
        self.highpass_presets.load()
        self.lowpass_presets.load()
        self.equalizer_presets.load()
        self.exciter_presets.load()
        self.bass_enhancer_presets.load()
        self.delay_presets.load()
