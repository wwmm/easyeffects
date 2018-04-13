# -*- coding: utf-8 -*-

import configparser

from PulseEffects.bass_enhancer_presets import BassEnhancerPresets
from PulseEffects.compressor_presets import CompressorPresets
from PulseEffects.crossfeed_presets import CrossfeedPresets
from PulseEffects.delay_presets import DelayPresets
from PulseEffects.deesser_presets import DeesserPresets
from PulseEffects.equalizer_presets import EqualizerPresets
from PulseEffects.exciter_presets import ExciterPresets
from PulseEffects.gate_presets import GatePresets
from PulseEffects.highpass_presets import HighpassPresets
from PulseEffects.limiter_presets import LimiterPresets
from PulseEffects.lowpass_presets import LowpassPresets
from PulseEffects.maximizer_presets import MaximizerPresets
from PulseEffects.output_limiter_presets import OutputLimiterPresets
from PulseEffects.panorama_presets import PanoramaPresets
from PulseEffects.pitch_presets import PitchPresets
from PulseEffects.reverb_presets import ReverbPresets
from PulseEffects.stereo_enhancer_presets import StereoEnhancerPresets
from PulseEffects.stereo_spread_presets import StereoSpreadPresets


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
        self.stereo_enhancer_presets = StereoEnhancerPresets(self.config)
        self.stereo_spread_presets = StereoSpreadPresets(self.config)
        self.crossfeed_presets = CrossfeedPresets(self.config)
        self.maximizer_presets = MaximizerPresets(self.config)
        self.output_limiter_presets = OutputLimiterPresets(self.config)
        self.pitch_presets = PitchPresets(self.config)
        self.gate_presets = GatePresets(self.config)
        self.deesser_presets = DeesserPresets(self.config)

    def save(self, path):
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
        self.stereo_enhancer_presets.save()
        self.stereo_spread_presets.save()
        self.crossfeed_presets.save()
        self.maximizer_presets.save()
        self.output_limiter_presets.save()
        self.pitch_presets.save()
        self.gate_presets.save()
        self.deesser_presets.save()

        output_file = open(path, 'w')

        self.config.write(output_file)

        output_file.close()
