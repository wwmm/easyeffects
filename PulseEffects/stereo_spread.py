# -*- coding: utf-8 -*-

import logging
import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GObject, Gst, GstInsertBin, Gtk

Gst.init(None)


class StereoSpread():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.log = logging.getLogger('PulseEffects')

        if Gst.ElementFactory.make(
                'calf-sourceforge-net-plugins-MultiSpread'):
            self.is_installed = True
        else:
            self.is_installed = False

            self.log.warn('Stereo Spread plugin was not found. Disabling it!')

        self.build_bin()

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_bin(self):
        self.input_gain = Gst.ElementFactory.make('volume', None)
        self.stereo_spread = Gst.ElementFactory.make(
            'calf-sourceforge-net-plugins-MultiSpread', None)

        self.input_level = Gst.ElementFactory.make('level',
                                                   'stereo_spread_input_level')
        self.out_level = Gst.ElementFactory.make('level',
                                                 'stereo_spread_output_level')

        self.bin = GstInsertBin.InsertBin.new('stereo_spread_bin')

        if self.is_installed:
            # it seems there is a bug in gstreaner
            # booleans are inverted. For example we have to turn on bypass in
            # order to effects to be applied

            self.stereo_spread.set_property('bypass', True)
            self.stereo_spread.set_property('level-in', 1.0)

            self.bin.append(self.input_gain, self.on_filter_added, None)
            self.bin.append(self.input_level, self.on_filter_added, None)
            self.bin.append(self.stereo_spread, self.on_filter_added, None)
            self.bin.append(self.out_level, self.on_filter_added, None)

    def post_messages(self, state):
        self.input_level.set_property('post-messages', state)
        self.output_level.set_property('post-messages', state)

    def init_ui(self):
        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/stereo_spread.glade')

        self.builder.connect_signals(self)

        self.ui_window = self.builder.get_object('window')
        self.ui_controls = self.builder.get_object('controls')
        self.ui_listbox_control = self.builder.get_object('listbox_control')

        self.ui_enable = self.builder.get_object('enable')
        self.ui_img_state = self.builder.get_object('img_state')

        self.ui_input_gain = self.builder.get_object('input_gain')
        self.ui_output_gain = self.builder.get_object('output_gain')
        self.ui_amount0 = self.builder.get_object('amount0')
        self.ui_amount1 = self.builder.get_object('amount1')
        self.ui_amount2 = self.builder.get_object('amount2')
        self.ui_amount3 = self.builder.get_object('amount3')
        self.ui_filters = self.builder.get_object('filters')
        self.ui_mono = self.builder.get_object('mono')

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
        # binding ui widgets to gsettings

        flag = Gio.SettingsBindFlags.DEFAULT

        self.settings.bind('stereo-spread-state', self.ui_enable, 'active',
                           flag)
        self.settings.bind('stereo-spread-state', self.ui_img_state, 'visible',
                           flag)
        self.settings.bind('stereo-spread-state', self.ui_controls,
                           'sensitive', Gio.SettingsBindFlags.GET)

        self.settings.bind('stereo-spread-input-gain', self.ui_input_gain,
                           'value', flag)
        self.settings.bind('stereo-spread-output-gain', self.ui_output_gain,
                           'value', flag)

        self.settings.bind('stereo-spread-amount0', self.ui_amount0, 'value',
                           flag)
        self.settings.bind('stereo-spread-amount1', self.ui_amount1, 'value',
                           flag)
        self.settings.bind('stereo-spread-amount2', self.ui_amount2, 'value',
                           flag)
        self.settings.bind('stereo-spread-amount3', self.ui_amount3, 'value',
                           flag)
        self.settings.bind('stereo-spread-filters', self.ui_filters,
                           'value', flag)
        self.settings.bind('stereo-spread-mono', self.ui_mono, 'active', flag)

        # binding ui widgets to gstreamer plugins

        flag = GObject.BindingFlags.BIDIRECTIONAL | \
            GObject.BindingFlags.SYNC_CREATE

        # it seems there is a bug in gstreaner. Calf plugin booleans
        # booleans are inverted. For example we have to turn on bypass in
        # order to effects to be applied

        flag_invert_boolean = flag | GObject.BindingFlags.INVERT_BOOLEAN

        self.ui_amount0.bind_property('value', self.stereo_spread, 'amount0',
                                      flag)
        self.ui_amount1.bind_property('value', self.stereo_spread, 'amount1',
                                      flag)
        self.ui_amount2.bind_property('value', self.stereo_spread, 'amount2',
                                      flag)
        self.ui_amount3.bind_property('value', self.stereo_spread, 'amount3',
                                      flag)
        self.ui_filters.bind_property('value', self.stereo_spread, 'filters',
                                      flag)
        self.ui_mono.bind_property('active', self.stereo_spread, 'mono',
                                   flag_invert_boolean)

    def on_input_gain_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20.0)

        self.input_gain.set_property('volume', value_linear)

    def on_output_gain_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20.0)

        self.stereo_spread.set_property('level-out', value_linear)

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

        harmonics = self.stereo_spread.get_property('meter-drive')
        self.ui_harmonics_levelbar.set_value(harmonics)

    def reset(self):
        self.settings.reset('stereo-spread-state')
        self.settings.reset('stereo-spread-input-gain')
        self.settings.reset('stereo-spread-output-gain')
        self.settings.reset('stereo-spread-amount0')
        self.settings.reset('stereo-spread-amount1')
        self.settings.reset('stereo-spread-amount2')
        self.settings.reset('stereo-spread-amount3')
        self.settings.reset('stereo-spread-filters')
        self.settings.reset('stereo-spread-mono')
