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

        self.autovolume_level = Gst.ElementFactory.make('level', 'autovolume')

        self.build_panorama_bin()

        # self.effects_bin.append(self.limiter_bin, self.on_filter_added, None)
        # self.effects_bin.append(self.autovolume_level, self.on_filter_added,
        #                         None)
        # self.effects_bin.append(self.panorama_bin, self.on_filter_added, None)
        # self.effects_bin.append(self.compressor_bin, self.on_filter_added,
        #                         None)
        # self.effects_bin.append(self.reverb_bin, self.on_filter_added, None)
        # self.effects_bin.append(self.highpass_bin, self.on_filter_added, None)
        # self.effects_bin.append(self.lowpass_bin, self.on_filter_added, None)
        # self.effects_bin.append(self.equalizer_bin, self.on_filter_added, None)
        # self.effects_bin.append(self.spectrum, self.on_filter_added, None)

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
