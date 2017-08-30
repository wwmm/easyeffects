# -*- coding: utf-8 -*-

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
from gi.repository import GObject, Gst, GstInsertBin
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

        self.build_panorama_bin()

        self.pipeline.add(self.audio_src)
        self.pipeline.add(self.source_caps)
        self.pipeline.add(self.limiter_bin)
        self.pipeline.add(self.autovolume_level)
        self.pipeline.add(self.panorama_bin)
        self.pipeline.add(self.compressor_bin)
        self.pipeline.add(self.reverb_bin)
        self.pipeline.add(self.highpass_bin)
        self.pipeline.add(self.lowpass_bin)
        self.pipeline.add(self.equalizer_bin)
        self.pipeline.add(self.spectrum)
        self.pipeline.add(self.audio_sink)

        self.audio_src.link(self.source_caps)
        self.source_caps.link(self.limiter_bin)
        self.limiter_bin.link(self.autovolume_level)
        self.autovolume_level.link(self.panorama_bin)
        self.panorama_bin.link(self.compressor_bin)
        self.compressor_bin.link(self.reverb_bin)
        self.reverb_bin.link(self.highpass_bin)
        self.highpass_bin.link(self.lowpass_bin)
        self.lowpass_bin.link(self.equalizer_bin)
        self.equalizer_bin.link(self.spectrum)
        self.spectrum.link(self.audio_sink)

    def build_panorama_bin(self):
        self.panorama = Gst.ElementFactory.make('audiopanorama', None)

        panorama_input_level = Gst.ElementFactory.make(
            'level', 'panorama_input_level')
        panorama_output_level = Gst.ElementFactory.make(
            'level', 'panorama_output_level')

        self.panorama.set_property('method', 'psychoacoustic')

        self.panorama_bin = GstInsertBin.InsertBin.new('panorama_bin')
        self.panorama_bin.append(self.panorama, self.on_filter_added, None)
        self.panorama_bin.append(panorama_input_level, self.on_filter_added,
                                 None)
        self.panorama_bin.append(panorama_output_level, self.on_filter_added,
                                 None)
