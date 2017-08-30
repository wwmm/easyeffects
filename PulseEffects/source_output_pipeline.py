# -*- coding: utf-8 -*-

import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst
from PulseEffects.pipeline_base import PipelineBase


Gst.init(None)


class SourceOutputPipeline(PipelineBase):

    def __init__(self, sampling_rate):
        PipelineBase.__init__(self, sampling_rate)

        self.effects_bin.append(self.limiter_bin, self.on_filter_added, None)
        self.effects_bin.append(self.compressor_bin, self.on_filter_added,
                                None)
        self.effects_bin.append(self.reverb_bin, self.on_filter_added, None)
        self.effects_bin.append(self.highpass_bin, self.on_filter_added, None)
        self.effects_bin.append(self.lowpass_bin, self.on_filter_added, None)
        self.effects_bin.append(self.equalizer_bin, self.on_filter_added, None)
        self.effects_bin.append(self.spectrum, self.on_filter_added, None)
