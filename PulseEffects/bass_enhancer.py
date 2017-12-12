# -*- coding: utf-8 -*-

import logging
import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GObject, Gst, GstInsertBin, Gtk

Gst.init(None)


class BassEnhancer():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.log = logging.getLogger('PulseEffects')

        if Gst.ElementFactory.make(
                'calf-sourceforge-net-plugins-BassEnhancer'):
            self.is_installed = True
        else:
            self.is_installed = False

            self.log.warn('Bass Enhancer plugin was not found. Disabling it!')

        self.build_bin()

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_bin(self):
        self.bass_enhancer = Gst.ElementFactory.make(
            'calf-sourceforge-net-plugins-BassEnhancer', None)

        self.input_level = Gst.ElementFactory.make('level',
                                                   'bass_enhancer_input_level')
        self.out_level = Gst.ElementFactory.make('level',
                                                 'bass_enhancer_output_level')

        self.bin = GstInsertBin.InsertBin.new('bass_enhancer_bin')

        if self.is_installed:
            # it seems there is a bug in gstreaner
            # booleans are inverted. For example we have to turn on bypass in
            # order to effects to be applied

            self.bass_enhancer.set_property('bypass', True)
            self.bass_enhancer.set_property('listen', True)
            self.bass_enhancer.set_property('floor-active', False)

            self.bin.append(self.input_level, self.on_filter_added, None)
            self.bin.append(self.bass_enhancer, self.on_filter_added, None)
            self.bin.append(self.out_level, self.on_filter_added, None)

    def post_messages(self, state):
        self.input_level.set_property('post-messages', state)
        self.out_level.set_property('post-messages', state)

    def init_ui(self):
        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/bass_enhancer.glade')

        self.builder.connect_signals(self)

        self.ui_window = self.builder.get_object('window')
        self.ui_controls = self.builder.get_object('controls')
        self.ui_listbox_control = self.builder.get_object('listbox_control')

        self.ui_enable = self.builder.get_object('enable')
        self.ui_img_state = self.builder.get_object('img_state')

        self.ui_input_gain = self.builder.get_object('input_gain')
        self.ui_output_gain = self.builder.get_object('output_gain')
        self.ui_amount = self.builder.get_object('amount')
        self.ui_harmonics = self.builder.get_object('harmonics')
        self.ui_scope = self.builder.get_object('scope')
        self.ui_floor = self.builder.get_object('floor')
        self.ui_blend = self.builder.get_object('blend')

        self.ui_input_level_left = self.builder.get_object('input_level_left')
        self.ui_input_level_right = self.builder.get_object(
            'input_level_right')
        self.ui_output_level_left = self.builder.get_object(
            'output_level_left')
        self.ui_output_level_right = self.builder.get_object(
            'output_level_right')
        self.ui_harmonics_levelbar = self.builder.get_object(
            'harmonics_levelbar')

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

        flag = GObject.BindingFlags.DEFAULT

        self.ui_amount.bind_property('value', self.bass_enhancer, 'amount',
                                     flag)
        self.ui_harmonics.bind_property('value', self.bass_enhancer, 'drive',
                                        flag)
        self.ui_scope.bind_property('value', self.bass_enhancer, 'freq', flag)
        self.ui_floor.bind_property('value', self.bass_enhancer, 'floor', flag)
        self.ui_blend.bind_property('value', self.bass_enhancer, 'blend', flag)

        # binding ui widgets to gsettings

        flag = Gio.SettingsBindFlags.DEFAULT

        self.settings.bind('bass-enhancer-state', self.ui_enable, 'active',
                           flag)
        self.settings.bind('bass-enhancer-state', self.ui_img_state, 'visible',
                           flag)
        self.settings.bind('bass-enhancer-state', self.ui_controls,
                           'sensitive', Gio.SettingsBindFlags.GET)
        self.settings.bind('bass-enhancer-input-gain', self.ui_input_gain,
                           'value', flag)
        self.settings.bind('bass-enhancer-output-gain', self.ui_output_gain,
                           'value', flag)
        self.settings.bind('bass-enhancer-amount', self.ui_amount, 'value',
                           flag)
        self.settings.bind('bass-enhancer-harmonics', self.ui_harmonics,
                           'value', flag)
        self.settings.bind('bass-enhancer-scope', self.ui_scope, 'value', flag)
        self.settings.bind('bass-enhancer-floor', self.ui_floor, 'value', flag)
        self.settings.bind('bass-enhancer-blend', self.ui_blend, 'value', flag)

    def on_input_gain_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20.0)

        self.bass_enhancer.set_property('level-in', value_linear)

    def on_output_gain_value_changed(self, obj):
        value_db = obj.get_value()
        value_linear = 10**(value_db / 20.0)

        self.bass_enhancer.set_property('level-out', value_linear)

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

        harmonics = self.bass_enhancer.get_property('meter-drive')
        self.ui_harmonics_levelbar.set_value(harmonics)

    def reset(self):
        self.settings.reset('bass-enhancer-state')
        self.settings.reset('bass-enhancer-input-gain')
        self.settings.reset('bass-enhancer-output-gain')
        self.settings.reset('bass-enhancer-amount')
        self.settings.reset('bass-enhancer-harmonics')
        self.settings.reset('bass-enhancer-scope')
        self.settings.reset('bass-enhancer-floor')
        self.settings.reset('bass-enhancer-blend')
