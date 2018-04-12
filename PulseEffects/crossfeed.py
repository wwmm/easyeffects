# -*- coding: utf-8 -*-

import logging
import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GObject, Gst, GstInsertBin, Gtk

Gst.init(None)


class Crossfeed():

    def __init__(self):
        self.settings = None
        self.module_path = os.path.dirname(__file__)

        self.log = logging.getLogger('PulseEffects')

        if Gst.ElementFactory.make('bs2b'):
            self.is_installed = True
        else:
            self.is_installed = False

            self.log.warn('Crossfeed plugin was not found. Disabling it!')

        self.build_bin()

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_bin(self):
        self.crossfeed = Gst.ElementFactory.make('bs2b', None)
        self.input_level = Gst.ElementFactory.make('level',
                                                   'crossfeed_input_level')
        self.output_level = Gst.ElementFactory.make('level',
                                                    'crossfeed_output_level')

        self.bin = GstInsertBin.InsertBin.new('crossfeed_bin')

        if self.is_installed:
            self.bin.append(self.input_level, self.on_filter_added, None)
            self.bin.append(self.crossfeed, self.on_filter_added, None)
            self.bin.append(self.output_level, self.on_filter_added, None)

    def post_messages(self, state):
        self.input_level.set_property('post-messages', state)
        self.output_level.set_property('post-messages', state)

    def init_ui(self):
        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/crossfeed.glade')
        self.builder.connect_signals(self)

        self.ui_window = self.builder.get_object('window')
        self.ui_controls = self.builder.get_object('controls')
        self.ui_listbox_control = self.builder.get_object('listbox_control')

        self.ui_enable = self.builder.get_object('enable')
        self.ui_img_state = self.builder.get_object('img_state')
        self.ui_cutoff = self.builder.get_object('cutoff')
        self.ui_feed = self.builder.get_object('feed')

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
        # binding ui widgets to gsettings

        flag = Gio.SettingsBindFlags.DEFAULT

        self.settings.bind('state', self.ui_enable, 'active', flag)
        self.settings.bind('state', self.ui_img_state, 'visible', flag)
        self.settings.bind('state', self.ui_controls, 'sensitive',
                           Gio.SettingsBindFlags.GET)
        self.settings.bind('fcut', self.ui_cutoff, 'value', flag)
        self.settings.bind('feed', self.ui_feed, 'value', flag)

        # binding ui widgets to gstreamer plugins

        flag = GObject.BindingFlags.BIDIRECTIONAL | \
            GObject.BindingFlags.SYNC_CREATE

        self.ui_cutoff.bind_property('value', self.crossfeed, 'fcut', flag)

    def on_feed_value_changed(self, obj):
        self.crossfeed.set_property('feed', int(10 * obj.get_value()))

    def apply_crossfeed_preset(self, values):
        self.ui_cutoff.set_value(values[0])
        self.ui_feed.set_value(values[1])

    def on_crossfeed_preset_clicked(self, obj):
        obj_id = Gtk.Buildable.get_name(obj)

        if obj_id == 'cmoy':
            value = self.settings.get_value('cmoy')
            self.apply_crossfeed_preset(value)
        elif obj_id == 'default':
            value = self.settings.get_value('default')
            self.apply_crossfeed_preset(value)
        elif obj_id == 'jmeier':
            value = self.settings.get_value('jmeier')
            self.apply_crossfeed_preset(value)

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
        self.settings.reset('fcut')
        self.settings.reset('feed')
        self.settings.reset('default')
        self.settings.reset('cmoy')
        self.settings.reset('jmeier')
