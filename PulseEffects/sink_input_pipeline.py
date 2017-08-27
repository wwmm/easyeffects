# -*- coding: utf-8 -*-

import gi
gi.require_version('Gst', '1.0')
from gi.repository import GObject, Gst
from PulseEffects.pipeline_base import PipelineBase


Gst.init(None)


class SinkInputPipeline(PipelineBase):

    __gsignals__ = {
        'new_autovolume': (GObject.SIGNAL_RUN_FIRST, None,
                           (float,))
    }

    def __init__(self, sampling_rate):
        PipelineBase.__init__(self, sampling_rate)

        self.build_pipeline()

    def build_pipeline(self):
        self.autovolume_level = Gst.ElementFactory.make('level', 'autovolume')

        self.panorama = Gst.ElementFactory.make('audiopanorama', None)

        self.panorama_output_level = Gst.ElementFactory.make(
            'level', 'panorama_output_level')

        self.panorama.set_property('method', 'psychoacoustic')

        self.pipeline.add(self.audio_src)
        self.pipeline.add(self.source_caps)
        self.pipeline.add(self.limiter_input_level)
        self.pipeline.add(self.limiter)
        self.pipeline.add(self.limiter_output_level)
        self.pipeline.add(self.autovolume_level)
        self.pipeline.add(self.panorama)
        self.pipeline.add(self.panorama_output_level)
        self.pipeline.add(self.compressor)
        self.pipeline.add(self.compressor_output_level)
        self.pipeline.add(self.freeverb)
        self.pipeline.add(self.reverb_output_level)
        self.pipeline.add(self.highpass_bin)
        self.pipeline.add(self.lowpass_bin)
        self.pipeline.add(self.equalizer_bin)
        self.pipeline.add(self.spectrum)
        self.pipeline.add(self.output_limiter)
        self.pipeline.add(self.audio_sink)

        self.audio_src.link(self.source_caps)
        self.source_caps.link(self.limiter_input_level)
        self.limiter_input_level.link(self.limiter)
        self.limiter.link(self.limiter_output_level)
        self.limiter_output_level.link(self.autovolume_level)
        self.autovolume_level.link(self.panorama)
        self.panorama.link(self.panorama_output_level)
        self.panorama_output_level.link(self.compressor)
        self.compressor.link(self.compressor_output_level)
        self.compressor_output_level.link(self.freeverb)
        self.freeverb.link(self.reverb_output_level)
        self.reverb_output_level.link(self.highpass_bin)
        self.highpass_bin.link(self.lowpass_bin)
        self.lowpass_bin.link(self.equalizer_bin)
        self.equalizer_bin.link(self.spectrum)
        self.spectrum.link(self.output_limiter)
        self.output_limiter.link(self.audio_sink)
