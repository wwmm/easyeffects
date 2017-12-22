# -*- coding: utf-8 -*-

import logging
import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GObject, Gst, Gtk

Gst.init(None)


class Convolver():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.log = logging.getLogger('PulseEffects')

        self.build_bin()

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_bin(self):
        self.bin = Gst.Bin.new('convolver_bin')

        self.input_level = Gst.ElementFactory.make('level',
                                                   'convolver_input_level')

        self.deinterleave = Gst.ElementFactory.make('deinterleave',
                                                    'convolver_deinterleave')

        self.queue_L = Gst.ElementFactory.make('queue', None)
        self.queue_R = Gst.ElementFactory.make('queue', None)

        self.fir_L = Gst.ElementFactory.make('audiofirfilter', 'fir_L')
        self.fir_R = Gst.ElementFactory.make('audiofirfilter', 'fir_R')

        self.output_level = Gst.ElementFactory.make('level',
                                                    'convolver_output_level')

        self.bin.add(self.input_level)
        self.bin.add(self.deinterleave)
        self.bin.add(self.queue_L)
        self.bin.add(self.queue_R)
        self.bin.add(self.fir_L)
        self.bin.add(self.fir_R)
        self.bin.add(self.output_level)

        self.input_level.link(self.deinterleave)

        self.queue_L.link(self.fir_L)
        self.queue_R.link(self.fir_R)

        static_pad = self.input_level.get_static_pad("sink")
        ghost_pad = Gst.GhostPad.new('sink', static_pad)
        self.bin.add_pad(ghost_pad)
        static_pad = self.output_level.get_static_pad("src")
        ghost_pad = Gst.GhostPad.new('src', static_pad)
        self.bin.add_pad(ghost_pad)

        self.deinterleave.set_property('keep-positions', True)

        self.deinterleave.connect('pad-added', self.on_pad_added)
        self.deinterleave.connect('pad-removed', self.on_pad_removed)
        self.deinterleave.connect('no-more-pads', self.on_no_more_pads)

    def on_pad_added(self, element, pad):
        pad_info = pad.get_name().split('_')

        if pad_info[1] == '0':  # left channel pad
            pad.link(self.queue_L.get_static_pad('sink'))
        else:
            pad.link(self.queue_R.get_static_pad('sink'))

    def on_no_more_pads(self, user_data):
        self.interleave = Gst.ElementFactory.make('interleave', None)

        self.bin.add(self.interleave)

        self.interleave.link(self.output_level)

        self.interleave.set_property('channel-positions-from-input', True)

        self.request_pad_L = self.interleave.get_request_pad('sink_%u')

        self.fir_L.get_static_pad('src').link(self.request_pad_L)

        self.request_pad_R = self.interleave.get_request_pad('sink_%u')

        self.fir_R.get_static_pad('src').link(self.request_pad_R)

        self.interleave.set_state(Gst.State.PLAYING)

    def on_pad_removed(self, element, pad):
        if self.interleave:
            self.bin.remove(self.interleave)
            self.interleave = None

    def post_messages(self, state):
        self.input_level.set_property('post-messages', state)
        self.output_level.set_property('post-messages', state)

    def init_ui(self):
        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/convolver.glade')

        self.ui_window = self.builder.get_object('window')
        self.ui_controls = self.builder.get_object('controls')
        self.ui_listbox_control = self.builder.get_object('listbox_control')

        self.ui_enable = self.builder.get_object('enable')
        self.ui_img_state = self.builder.get_object('img_state')
        # self.ui_cutoff = self.builder.get_object('cutoff')
        # self.ui_poles = self.builder.get_object('poles')

        self.ui_input_level_left = self.builder.get_object(
            'input_level_left')
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

        flag = GObject.BindingFlags.BIDIRECTIONAL

        # self.ui_cutoff.bind_property('value', self.convolver, 'cutoff', flag)
        # self.ui_poles.bind_property('value', self.convolver, 'poles', flag)

        # binding ui widgets to gsettings

        flag = Gio.SettingsBindFlags.DEFAULT

        self.settings.bind('convolver-state', self.ui_enable, 'active', flag)
        self.settings.bind('convolver-state', self.ui_img_state, 'visible',
                           flag)
        # self.settings.bind('convolver-state', self.ui_controls, 'sensitive',
        #                    Gio.SettingsBindFlags.GET)
        # self.settings.bind('lowpass-cutoff', self.ui_cutoff, 'value', flag)
        # self.settings.bind('lowpass-poles', self.ui_poles, 'value', flag)

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
        self.settings.reset('convolver-state')
        # self.settings.reset('lowpass-cutoff')
        # self.settings.reset('lowpass-poles')
