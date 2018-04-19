# -*- coding: utf-8 -*-

import logging
import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GObject, Gst, GstInsertBin, Gtk

Gst.init(None)


class Webrtc():

    def __init__(self):
        self.settings = None
        self.module_path = os.path.dirname(__file__)

        self.log = logging.getLogger('PulseEffects')

        if Gst.ElementFactory.make('webrtcdsp'):
            self.is_installed = True
        else:
            self.is_installed = False

            self.log.warn('Webrtc plugin was not found. Disabling it!')

        self.build_bin()

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_bin(self):
        self.webrtc = Gst.ElementFactory.make('webrtcdsp', None)
        self.in_level = Gst.ElementFactory.make('level', 'webrtc_input_level')
        self.out_level = Gst.ElementFactory.make('level',
                                                 'webrtc_output_level')

        self.bin = GstInsertBin.InsertBin.new('webrtc_bin')

        if self.is_installed:
            # probe setup

            self.probe_bin = Gst.Bin.new()

            self.probe = Gst.ElementFactory.make('webrtcechoprobe', None)
            self.probe_src = Gst.ElementFactory.make('pulsesrc', None)
            probe_queue = Gst.ElementFactory.make('queue', None)
            probe_sink = Gst.ElementFactory.make('fakesink', None)
            probe_convert = Gst.ElementFactory.make('audioconvert', None)
            probe_resample = Gst.ElementFactory.make('audioresample', None)
            probe_caps = Gst.ElementFactory.make('capsfilter', None)

            pa_props_str = 'props,application.name=PulseEffectsWebrtcProbe'
            pa_props = Gst.Structure.new_from_string(pa_props_str)

            self.probe_src.set_property('stream-properties', pa_props)
            self.probe_src.set_property('buffer-time', 10000)
            # self.probe_src.set_property('latency-time', 10000)

            caps_in = Gst.Caps.from_string('audio/x-raw,' + 'format=S16LE,' +
                                           'rate=48000,' + 'channels=2')

            probe_caps.set_property('caps', caps_in)

            probe_queue.set_property('silent', True)

            self.probe_bin.add(self.probe_src)
            self.probe_bin.add(probe_queue)
            self.probe_bin.add(probe_convert)
            self.probe_bin.add(probe_resample)
            self.probe_bin.add(probe_caps)
            self.probe_bin.add(self.probe)
            self.probe_bin.add(probe_sink)

            self.probe_src.link(probe_queue)
            probe_queue.link(probe_convert)
            probe_convert.link(probe_resample)
            probe_resample.link(probe_caps)
            probe_caps.link(self.probe)
            self.probe.link(probe_sink)

            # dsp setup

            dsp_convert_in = Gst.ElementFactory.make('audioconvert', None)
            dsp_resample_in = Gst.ElementFactory.make('audioresample', None)
            dsp_convert_out = Gst.ElementFactory.make('audioconvert', None)
            dsp_resample_out = Gst.ElementFactory.make('audioresample', None)
            dsp_caps_in = Gst.ElementFactory.make('capsfilter', None)

            dsp_caps_in.set_property('caps', caps_in)

            self.bin.append(self.in_level, self.on_filter_added, None)
            self.bin.append(dsp_convert_in, self.on_filter_added, None)
            self.bin.append(dsp_resample_in, self.on_filter_added, None)
            self.bin.append(dsp_caps_in, self.on_filter_added, None)
            self.bin.append(self.webrtc, self.on_filter_added, None)
            self.bin.append(dsp_convert_out, self.on_filter_added, None)
            self.bin.append(dsp_resample_out, self.on_filter_added, None)
            self.bin.append(self.out_level, self.on_filter_added, None)

    def post_messages(self, state):
        self.in_level.set_property('post-messages', state)
        self.out_level.set_property('post-messages', state)

    def set_probe_src_device(self, name):
        self.probe_src.set_property('device', name)

    def init_ui(self):
        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/webrtc.glade')
        self.builder.connect_signals(self)

        self.ui_window = self.builder.get_object('window')
        self.ui_controls = self.builder.get_object('controls')
        self.ui_listbox_control = self.builder.get_object('listbox_control')

        self.ui_enable = self.builder.get_object('enable')
        self.ui_img_state = self.builder.get_object('img_state')

        self.ui_echo_cancel = self.builder.get_object('echo_cancel')
        self.ui_echo_suppression_level_low = self.builder.get_object(
            'echo_suppression_level_low')
        self.ui_echo_suppression_level_moderate = self.builder.get_object(
            'echo_suppression_level_moderate')
        self.ui_echo_suppression_level_high = self.builder.get_object(
            'echo_suppression_level_high')

        self.ui_noise_suppression = self.builder.get_object(
            'noise_suppression')
        self.ui_noise_suppression_level_low = self.builder.get_object(
            'noise_suppression_level_low')
        self.ui_noise_suppression_level_moderate = self.builder.get_object(
            'noise_suppression_level_moderate')
        self.ui_noise_suppression_level_high = self.builder.get_object(
            'noise_suppression_level_high')
        self.ui_noise_suppression_level_very_high = self.builder.get_object(
            'noise_suppression_level_very_high')

        self.ui_gain_control = self.builder.get_object('gain_control')
        self.ui_gain_control_mode_adaptive = self.builder.get_object(
            'gain_control_mode_adaptive')
        self.ui_gain_control_mode_fixed = self.builder.get_object(
            'gain_control_mode_fixed')

        self.ui_voice_detection = self.builder.get_object('voice_detection')
        self.ui_voice_detection_frame_size = self.builder.get_object(
            'voice_detection_frame_size')
        self.ui_voice_detection_likehood_very_low = self.builder.get_object(
            'voice_detection_likehood_very_low')
        self.ui_voice_detection_likehood_low = self.builder.get_object(
            'voice_detection_likehood_low')
        self.ui_voice_detection_likehood_moderate = self.builder.get_object(
            'voice_detection_likehood_moderate')
        self.ui_voice_detection_likehood_high = self.builder.get_object(
            'voice_detection_likehood_high')

        self.ui_input_level_left = self.builder.get_object('input_level_left')
        self.ui_input_level_right = self.builder.get_object(
            'input_level_right')
        self.ui_output_level_left = self.builder.get_object(
            'output_level_left')
        self.ui_output_level_right = self.builder.get_object(
            'output_level_right')

        self.ui_input_level_left_label = self.builder.get_object(
            'input_level_left_label')
        self.ui_input_level_right_label = self.builder.get_object(
            'input_level_right_label')
        self.ui_output_level_left_label = self.builder.get_object(
            'output_level_left_label')
        self.ui_output_level_right_label = self.builder.get_object(
            'output_level_right_label')

    def bind(self):
        # binding ui widgets to gstreamer plugins

        flag = Gio.SettingsBindFlags.DEFAULT

        self.settings.bind('state', self.ui_enable, 'active', flag)
        self.settings.bind('state', self.ui_img_state, 'visible', flag)
        self.settings.bind('state', self.ui_controls, 'sensitive',
                           Gio.SettingsBindFlags.GET)

        self.settings.bind('echo-cancel', self.ui_echo_cancel, 'active', flag)
        self.settings.bind('echo-suppression-level-low',
                           self.ui_echo_suppression_level_low, 'active', flag)
        self.settings.bind('echo-suppression-level-moderate',
                           self.ui_echo_suppression_level_moderate, 'active',
                           flag)
        self.settings.bind('echo-suppression-level-high',
                           self.ui_echo_suppression_level_high, 'active', flag)

        self.settings.bind('noise-suppression', self.ui_noise_suppression,
                           'active', flag)
        self.settings.bind('noise-suppression-level-low',
                           self.ui_noise_suppression_level_low, 'active', flag)
        self.settings.bind('noise-suppression-level-moderate',
                           self.ui_noise_suppression_level_moderate, 'active',
                           flag)
        self.settings.bind('noise-suppression-level-high',
                           self.ui_noise_suppression_level_high, 'active',
                           flag)
        self.settings.bind('noise-suppression-level-very-high',
                           self.ui_noise_suppression_level_very_high, 'active',
                           flag)

        self.settings.bind('gain-control', self.ui_gain_control, 'active',
                           flag)
        self.settings.bind('gain-control-mode-adaptive',
                           self.ui_gain_control_mode_adaptive, 'active', flag)
        self.settings.bind('gain-control-mode-fixed',
                           self.ui_gain_control_mode_fixed, 'active', flag)

        self.settings.bind('voice-detection', self.ui_voice_detection,
                           'active', flag)
        self.settings.bind('voice-detection-frame-size-ms',
                           self.ui_voice_detection_frame_size, 'value', flag)
        self.settings.bind('voice-detection-likelihood-very-low',
                           self.ui_voice_detection_likehood_very_low, 'active',
                           flag)
        self.settings.bind('voice-detection-likelihood-low',
                           self.ui_voice_detection_likehood_low, 'active',
                           flag)
        self.settings.bind('voice-detection-likelihood-moderate',
                           self.ui_voice_detection_likehood_moderate, 'active',
                           flag)
        self.settings.bind('voice-detection-likelihood-high',
                           self.ui_voice_detection_likehood_high, 'active',
                           flag)

        # binding ui widgets to gstreamer plugins

        flag = GObject.BindingFlags.BIDIRECTIONAL | \
            GObject.BindingFlags.SYNC_CREATE

        self.ui_echo_cancel.bind_property('active', self.webrtc, 'echo-cancel',
                                          flag)
        self.ui_noise_suppression.bind_property('active', self.webrtc,
                                                'noise-suppression', flag)
        self.ui_gain_control.bind_property('active', self.webrtc,
                                           'gain-control', flag)
        self.ui_voice_detection.bind_property('active', self.webrtc,
                                              'voice-detection', flag)
        self.ui_voice_detection_frame_size.bind_property
        ('value', self.webrtc, 'voice-detection-frame-size-ms', flag)

    def on_new_echo_suppression_level(self, obj):
        if obj.get_active():
            label = obj.get_name()

            if label == 'echo_suppression_level_low':
                self.webrtc.set_property('echo-suppression-level', 'low')
            elif label == 'echo_suppression_level_moderate':
                self.webrtc.set_property('echo-suppression-level', 'moderate')
            elif label == 'echo_suppression_level_high':
                self.webrtc.set_property('echo-suppression-level', 'high')

    def on_new_noise_suppression_level(self, obj):
        if obj.get_active():
            label = obj.get_name()

            if label == 'noise_suppression_level_low':
                self.webrtc.set_property('noise-suppression-level', 'low')
            elif label == 'noise_suppression_level_moderate':
                self.webrtc.set_property('noise-suppression-level', 'moderate')
            elif label == 'noise_suppression_level_high':
                self.webrtc.set_property('noise-suppression-level', 'high')
            elif label == 'noise_suppression_level_very_high':
                self.webrtc.set_property('noise-suppression-level',
                                         'very-high')

    def on_new_gain_control_mode(self, obj):
        if obj.get_active():
            label = obj.get_name()

            if label == 'gain_control_mode_adaptive':
                self.webrtc.set_property('gain-control-mode',
                                         'adaptive-digital')
            elif label == 'gain_control_mode_fixed':
                self.webrtc.set_property('gain-control-mode', 'fixed-digital')

    def on_new_voice_detection_likehood(self, obj):
        if obj.get_active():
            label = obj.get_name()

            if label == 'voice_detection_likehood_very_low':
                self.webrtc.set_property('voice-detection-likelihood',
                                         'very-low')
            elif label == 'voice_detection_likehood_low':
                self.webrtc.set_property('voice-detection-likelihood',
                                         'low')
            elif label == 'voice_detection_likehood_moderate':
                self.webrtc.set_property('voice-detection-likelihood',
                                         'moderate')
            elif label == 'voice_detection_likehood_high':
                self.webrtc.set_property('voice-detection-likelihood',
                                         'high')

    def ui_update_level(self, widgets, peak):
        left, right = peak[0], peak[1]

        widget_level_left = widgets[0]
        widget_level_right = widgets[1]
        widget_level_left_label = widgets[2]
        widget_level_right_label = widgets[3]

        if left >= -99:
            l_value = 10**(left / 10)
            widget_level_left.set_value(l_value)
            widget_level_left_label.set_text(str(round(left)))
        else:
            widget_level_left.set_value(0)
            widget_level_left_label.set_text('-99')

        if right >= -99:
            r_value = 10**(right / 10)
            widget_level_right.set_value(r_value)
            widget_level_right_label.set_text(str(round(right)))
        else:
            widget_level_right.set_value(0)
            widget_level_right_label.set_text('-99')

    def ui_update_input_level(self, peak):
        widgets = [self.ui_input_level_left, self.ui_input_level_right,
                   self.ui_input_level_left_label,
                   self.ui_input_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_output_level(self, peak):
        widgets = [self.ui_output_level_left, self.ui_output_level_right,
                   self.ui_output_level_left_label,
                   self.ui_output_level_right_label]

        self.ui_update_level(widgets, peak)

    def reset(self):
        self.settings.reset('state')
        self.settings.reset('high-pass-filter')
        self.settings.reset('experimental-agc')
        self.settings.reset('extended-filter')
        self.settings.reset('delay-agnostic')
        self.settings.reset('target-level-dbfs')
        self.settings.reset('compression-gain-db')
        self.settings.reset('startup-min-volume')
        self.settings.reset('limiter')
        self.settings.reset('gain-control')
        self.settings.reset('gain-control-mode-adaptive')
        self.settings.reset('gain-control-mode-fixed')
        self.settings.reset('echo-cancel')
        self.settings.reset('echo-suppression-level-low')
        self.settings.reset('echo-suppression-level-moderate')
        self.settings.reset('echo-suppression-level-high')
        self.settings.reset('noise-suppression')
        self.settings.reset('noise-suppression-level-low')
        self.settings.reset('noise-suppression-level-modetate')
        self.settings.reset('noise-suppression-level-high')
        self.settings.reset('noise-suppression-level-very-high')
        self.settings.reset('voice-detection')
        self.settings.reset('voice-detection-frame-size-ms')
        self.settings.reset('voice-detection-likelihood-very-low')
        self.settings.reset('voice-detection-likelihood-low')
        self.settings.reset('voice-detection-likelihood-moderate')
        self.settings.reset('voice-detection-likelihood-high')
