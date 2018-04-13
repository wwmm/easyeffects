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

            caps = ['audio/x-raw', 'format=S16LE', 'rate=48000', 'channels=2']

            src_caps = Gst.Caps.from_string(','.join(caps))

            probe_caps.set_property('caps', src_caps)

            probe_queue.set_property('silent', True)
            probe_queue.set_property('max-size-buffers', 0)
            probe_queue.set_property('max-size-time', 0)
            probe_queue.set_property('max-size-bytes', 0)

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

            dsp_convert = Gst.ElementFactory.make('audioconvert', None)
            dsp_resample = Gst.ElementFactory.make('audioresample', None)
            dsp_caps = Gst.ElementFactory.make('capsfilter', None)

            dsp_caps.set_property('caps', src_caps)

            # self.webrtc.set_property('echo-cancel', True)
            # self.webrtc.set_property('echo-suppression-level', 'high')
            # self.webrtc.set_property('noise-suppression', True)
            # self.webrtc.set_property('noise-suppression-level', 'low')
            # self.webrtc.set_property('high-pass-filter', True)

            self.bin.append(self.in_level, self.on_filter_added, None)
            self.bin.append(dsp_convert, self.on_filter_added, None)
            self.bin.append(dsp_resample, self.on_filter_added, None)
            self.bin.append(dsp_caps, self.on_filter_added, None)
            self.bin.append(self.webrtc, self.on_filter_added, None)
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

        # self.settings.bind('echo-cancel', self.ui_echo_cancel, 'value',
        # flag)
        # self.settings.bind('ceiling', self.ui_ceiling, 'value', flag)
        # self.settings.bind('threshold', self.ui_threshold, 'value',
        #                    flag)

        # binding ui widgets to gstreamer plugins

        flag = GObject.BindingFlags.BIDIRECTIONAL | \
            GObject.BindingFlags.SYNC_CREATE

        # self.ui_release.bind_property('value', self.webrtc, 'release', flag)
        # self.ui_ceiling.bind_property('value', self.webrtc,
        #                               'output-ceiling', flag)
        # self.ui_threshold.bind_property('value', self.webrtc,
        #                                 'threshold', flag)

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
        # self.settings.reset('release')
        # self.settings.reset('ceiling')
        # self.settings.reset('threshold')
