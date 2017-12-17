# -*- coding: utf-8 -*-

import logging
import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GObject, Gst, GstInsertBin, Gtk

Gst.init(None)


class Maximizer():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.log = logging.getLogger('PulseEffects')

        self.old_limiter_attenuation = 0

        if Gst.ElementFactory.make('ladspa-zamaximx2-ladspa-so-zamaximx2'):
            self.is_installed = True
        else:
            self.is_installed = False

            self.log.warn('Maximizer plugin was not found. Disabling it!')

        self.build_bin()

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_bin(self):
        self.maximizer = Gst.ElementFactory.make(
            'ladspa-zamaximx2-ladspa-so-zamaximx2', None)
        self.in_level = Gst.ElementFactory.make('level',
                                                'maximizer_input_level')
        self.out_level = Gst.ElementFactory.make('level',
                                                 'maximizer_output_level')

        self.bin = GstInsertBin.InsertBin.new('maximizer_bin')

        if self.is_installed:
            self.bin.append(self.in_level, self.on_filter_added, None)
            self.bin.append(self.maximizer, self.on_filter_added, None)
            self.bin.append(self.out_level, self.on_filter_added, None)

    def post_messages(self, state):
        self.in_level.set_property('post-messages', state)
        self.out_level.set_property('post-messages', state)

    def init_ui(self):
        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/maximizer.glade')
        self.builder.connect_signals(self)

        self.ui_window = self.builder.get_object('window')
        self.ui_controls = self.builder.get_object('controls')
        self.ui_listbox_control = self.builder.get_object('listbox_control')

        self.ui_enable = self.builder.get_object('enable')
        self.ui_img_state = self.builder.get_object('img_state')
        self.ui_release = self.builder.get_object('release')
        self.ui_ceiling = self.builder.get_object('ceiling')
        self.ui_threshold = self.builder.get_object('threshold')
        self.ui_attenuation_levelbar = self.builder.get_object(
            'attenuation_levelbar')

        self.ui_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_LOW', 10)
        self.ui_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_HIGH', 30)
        self.ui_attenuation_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_FULL', 40)

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
        self.ui_attenuation_level_label = self.builder.get_object(
            'attenuation_level_label')

    def bind(self):
        # binding ui widgets to gstreamer plugins

        flag = GObject.BindingFlags.BIDIRECTIONAL

        self.ui_release.bind_property('value', self.maximizer, 'release', flag)
        self.ui_ceiling.bind_property('value', self.maximizer,
                                      'output-ceiling', flag)
        self.ui_threshold.bind_property('value', self.maximizer,
                                        'threshold', flag)

        # binding ui widgets to gstreamer plugins

        flag = Gio.SettingsBindFlags.DEFAULT

        self.settings.bind('maximizer-state', self.ui_enable, 'active', flag)
        self.settings.bind('maximizer-state', self.ui_img_state, 'visible',
                           flag)
        self.settings.bind('maximizer-state', self.ui_controls, 'sensitive',
                           Gio.SettingsBindFlags.GET)
        self.settings.bind('maximizer-release', self.ui_release, 'value', flag)
        self.settings.bind('maximizer-ceiling', self.ui_ceiling, 'value', flag)
        self.settings.bind('maximizer-threshold', self.ui_threshold, 'value',
                           flag)

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

        attenuation = round(self.maximizer.get_property('gain-reduction'))

        print('latency: ', self.maximizer.get_property('param--latency'))

        if attenuation != self.old_limiter_attenuation:
            self.old_limiter_attenuation = attenuation

            self.ui_attenuation_levelbar.set_value(attenuation)
            self.ui_attenuation_level_label.set_text(str(round(attenuation)))

    def reset(self):
        self.settings.reset('maximizer-state')
        self.settings.reset('maximizer-release')
        self.settings.reset('maximizer-ceiling')
        self.settings.reset('maximizer-threshold')
