# -*- coding: utf-8 -*-

import os

import gi
import numpy as np
gi.require_version('Gst', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gst, Gtk
from scipy.interpolate import CubicSpline

from PulseEffectsTest.pipeline_base import PipelineBase


class TestSignals(PipelineBase):

    def __init__(self, rate):
        PipelineBase.__init__(self, rate)

        self.module_path = os.path.dirname(__file__)

        self.calc_spectrum_freqs()
        self.build_pipeline()
        self.init_ui()

    def build_pipeline(self):
        self.audio_src1 = Gst.ElementFactory.make('audiotestsrc', None)
        self.audio_src2 = Gst.ElementFactory.make('audiotestsrc', None)
        src1_caps = Gst.ElementFactory.make("capsfilter", None)
        src2_caps = Gst.ElementFactory.make("capsfilter", None)
        self.bandpass = Gst.ElementFactory.make('audiochebband', None)
        mixer = Gst.ElementFactory.make('audiomixer', None)
        self.spectrum = Gst.ElementFactory.make('spectrum', 'spectrum')
        self.audio_sink = Gst.ElementFactory.make('pulsesink', None)

        self.audio_sink.set_property('device', 'PulseEffects_apps')
        self.audio_sink.set_property('volume', 1.0)
        self.audio_sink.set_property('mute', False)

        pa_props_str = 'props,application.name=PulseEffectsTest'
        pa_props = Gst.Structure.new_from_string(pa_props_str)

        self.audio_sink.set_property('stream-properties', pa_props)

        self.audio_src1.set_property('wave', 'sine')
        self.audio_src2.set_property('wave', 'pink-noise')

        self.bandpass.set_property('mode', 'band-pass')
        self.bandpass.set_property('type', 1)
        self.bandpass.set_property('poles', 4)
        self.bandpass.set_property('ripple', 0)

        self.spectrum.set_property('bands', self.spectrum_nbands)
        self.spectrum.set_property('threshold', self.spectrum_threshold)
        self.spectrum.set_property('interval', int(1 * 1000000000))

        caps = ['audio/x-raw', 'format=F32LE',
                'rate=' + str(self.rate), 'channels=2']

        src_caps = Gst.Caps.from_string(",".join(caps))

        src1_caps.set_property("caps", src_caps)
        src2_caps.set_property("caps", src_caps)

        mixer.set_property('caps', src_caps)

        self.pipeline.add(self.audio_src1)
        self.pipeline.add(self.audio_src2)
        self.pipeline.add(src1_caps)
        self.pipeline.add(src2_caps)
        self.pipeline.add(self.bandpass)
        self.pipeline.add(mixer)
        self.pipeline.add(self.spectrum)
        self.pipeline.add(self.audio_sink)

        self.audio_src1.link(src1_caps)
        src1_caps.link(mixer)
        mixer.link(self.spectrum)
        self.spectrum.link(self.audio_sink)

        self.audio_src2.link(src2_caps)
        src2_caps.link(self.bandpass)
        self.bandpass.link(mixer)

    def init_ui(self):
        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/test_signals.glade')

        self.builder.connect_signals(self)

        self.ui_window = self.builder.get_object('window')

        self.wave1_switch = self.builder.get_object('wave1_switch')
        self.wave1_volume = self.builder.get_object('wave1_volume')
        self.wave1_freq = self.builder.get_object('wave1_freq')
        self.wave2_switch = self.builder.get_object('wave2_switch')
        self.wave2_volume = self.builder.get_object('wave2_volume')
        self.wave2_band8 = self.builder.get_object('wave2_band8')

        self.wave1_volume.set_value(0)
        self.wave1_freq.set_value(1000)
        self.set_wave1_freq(1000)
        self.set_wave1_volume(0)
        self.wave1_volume.set_sensitive(False)

        self.wave2_volume.set_value(0)
        self.set_wave2_volume(0)
        # equal loudness signal default frequency is 1 kHz (band 8)
        self.wave2_band8.set_active(True)
        self.wave2_volume.set_sensitive(False)

    def on_message_element(self, bus, msg):
        plugin = msg.src.get_name()

        if plugin == 'spectrum':
            magnitudes = msg.get_structure().get_value('magnitude')

            cs = CubicSpline(self.spectrum_freqs,
                             magnitudes[:self.spectrum_nfreqs])

            magnitudes = cs(self.spectrum_x_axis)

            max_mag = np.amax(magnitudes)
            min_mag = np.amin(magnitudes)

            if max_mag > min_mag:
                magnitudes = (magnitudes - min_mag) / (max_mag - min_mag)

                self.emit('new_spectrum', min_mag, max_mag, magnitudes)

        return True

    def set_wave1_volume(self, value):
        self.audio_src1.set_property('volume', value)

    def set_wave1_freq(self, value):
        self.audio_src1.set_property('freq', value)

    def set_wave1_type(self, value):
        self.audio_src1.set_property('wave', value)

    def set_wave2_freq(self, lower, upper, poles):
        current_lower = self.bandpass.get_property('lower-frequency')

        if upper < current_lower:
            self.bandpass.set_property('lower-frequency', lower)
            self.bandpass.set_property('upper-frequency', upper)
        else:
            self.bandpass.set_property('upper-frequency', upper)
            self.bandpass.set_property('lower-frequency', lower)

        self.bandpass.set_property('poles', poles)

    def set_wave2_volume(self, value):
        self.audio_src2.set_property('volume', value)

    def on_wave1_switch_state_set(self, obj, state):
        if state:
            self.wave1_volume.set_sensitive(True)
            self.wave1_volume.set_value(0.5)
            self.set_state('playing')
        else:
            self.wave1_volume.set_sensitive(False)
            self.wave1_volume.set_value(0)

            if not self.wave2_switch.get_state():
                self.set_state('paused')

    def on_wave1_volume_value_changed(self, obj):
        self.set_wave1_volume(obj.get_value())

    def on_wave1_type_toggled(self, obj):
        if obj.get_active():
            obj_id = Gtk.Buildable.get_name(obj)

            if obj_id == 'wave_sine':
                self.set_wave1_type('sine')
                self.wave1_freq.set_sensitive(True)
            elif obj_id == 'wave_square':
                self.set_wave1_type('square')
                self.wave1_freq.set_sensitive(True)
            elif obj_id == 'wave_saw':
                self.set_wave1_type('saw')
                self.wave1_freq.set_sensitive(True)
            elif obj_id == 'wave_triangle':
                self.set_wave1_type('triangle')
                self.wave1_freq.set_sensitive(True)
            elif obj_id == 'wave_silence':
                self.set_wave1_type('silence')
                self.wave1_freq.set_sensitive(False)
            elif obj_id == 'wave_white_noise':
                self.set_wave1_type('white-noise')
                self.wave1_freq.set_sensitive(False)
            elif obj_id == 'wave_pink_noise':
                self.set_wave1_type('pink-noise')
                self.wave1_freq.set_sensitive(False)
            elif obj_id == 'wave_sine_table':
                self.set_wave1_type('sine-table')
                self.wave1_freq.set_sensitive(True)
            elif obj_id == 'wave_ticks':
                self.set_wave1_type('ticks')
                self.wave1_freq.set_sensitive(True)
            elif obj_id == 'wave_gaussian_noise':
                self.set_wave1_type('gaussian-noise')
                self.wave1_freq.set_sensitive(False)
            elif obj_id == 'wave_red_noise':
                self.set_wave1_type('red-noise')
                self.wave1_freq.set_sensitive(False)
            elif obj_id == 'wave_blue_noise':
                self.set_wave1_type('blue-noise')
                self.wave1_freq.set_sensitive(False)
            elif obj_id == 'wave_violet_noise':
                self.set_wave1_type('violet-noise')
                self.wave1_freq.set_sensitive(False)

    def on_wave1_freq_value_changed(self, obj):
        self.set_wave1_freq(obj.get_value())

    def on_wave2_switch_state_set(self, obj, state):
        if state:
            self.wave2_volume.set_sensitive(True)
            self.wave2_volume.set_value(0.5)
            self.set_state('playing')
        else:
            self.wave2_volume.set_sensitive(False)
            self.wave2_volume.set_value(0)

            if not self.wave1_switch.get_state():
                self.set_state('paused')

    def on_wave2_volume_value_changed(self, obj):
        self.set_wave2_volume(obj.get_value())

    def on_wave2_freq_toggled(self, obj):
        if obj.get_active():
            obj_id = Gtk.Buildable.get_name(obj)

            if obj_id == 'wave2_band0':  # 26 Hz
                self.set_wave2_freq(20, 41, 4)
            elif obj_id == 'wave2_band1':  # 41 Hz
                self.set_wave2_freq(26, 65, 4)
            elif obj_id == 'wave2_band2':  # 65 Hz
                self.set_wave2_freq(41, 103, 4)
            elif obj_id == 'wave2_band3':  # 103 Hz
                self.set_wave2_freq(65, 163, 4)
            elif obj_id == 'wave2_band4':  # 163 Hz
                self.set_wave2_freq(103, 259, 4)
            elif obj_id == 'wave2_band5':  # 259 Hz
                self.set_wave2_freq(163, 410, 8)
            elif obj_id == 'wave2_band6':  # 410 Hz
                self.set_wave2_freq(259, 649, 8)
            elif obj_id == 'wave2_band7':  # 649 Hz
                self.set_wave2_freq(410, 1029, 8)
            elif obj_id == 'wave2_band8':  # 1029 Hz
                self.set_wave2_freq(649, 1631, 12)
            elif obj_id == 'wave2_band9':  # 1631 Hz
                self.set_wave2_freq(1029, 2585, 12)
            elif obj_id == 'wave2_band10':  # 2585 Hz
                self.set_wave2_freq(1631, 4097, 16)
            elif obj_id == 'wave2_band11':  # 4097 Hz
                self.set_wave2_freq(2585, 6493, 24)
            elif obj_id == 'wave2_band12':  # 6493 Hz
                self.set_wave2_freq(4097, 10291, 32)
            elif obj_id == 'wave2_band13':  # 10291 Hz
                self.set_wave2_freq(6493, 16310, 32)
            elif obj_id == 'wave2_band14':  # 16310 Hz
                self.set_wave2_freq(10291, 20000, 32)
