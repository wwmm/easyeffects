# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import GLib, Gst, GstInsertBin, Gtk

Gst.init(None)


class Compressor():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.old_compressor_gain_reduction = 0

        self.builder = Gtk.Builder()

        self.builder.add_from_file(self.module_path + '/ui/compressor.glade')

        self.build_compressor_bin()

        self.load_ui()

        self.builder.connect_signals(self)

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_compressor_bin(self):
        self.compressor = Gst.ElementFactory.make(
            'ladspa-sc4-1882-so-sc4', None)
        compressor_input_level = Gst.ElementFactory.make(
            'level', 'compressor_input_level')
        compressor_output_level = Gst.ElementFactory.make(
            'level', 'compressor_output_level')

        self.bin = GstInsertBin.InsertBin.new('compressor_bin')
        self.bin.append(self.compressor, self.on_filter_added, None)
        self.bin.append(compressor_input_level, self.on_filter_added, None)
        self.bin.append(compressor_output_level, self.on_filter_added, None)

    def load_ui(self):
        self.ui_window = self.builder.get_object('window')

        self.ui_compressor_rms = self.builder.get_object('compressor_rms')
        self.ui_compressor_peak = self.builder.get_object('compressor_peak')
        self.ui_compressor_attack = self.builder.get_object(
            'compressor_attack')
        self.ui_compressor_release = self.builder.get_object(
            'compressor_release')
        self.ui_compressor_threshold = self.builder.get_object(
            'compressor_threshold')
        self.ui_compressor_ratio = self.builder.get_object(
            'compressor_ratio')
        self.ui_compressor_knee = self.builder.get_object('compressor_knee')
        self.ui_compressor_makeup = self.builder.get_object(
            'compressor_makeup')

        self.ui_compressor_input_level_left = self.builder.get_object(
            'compressor_input_level_left')
        self.ui_compressor_input_level_right = self.builder.get_object(
            'compressor_input_level_right')
        self.ui_compressor_output_level_left = self.builder.get_object(
            'compressor_output_level_left')
        self.ui_compressor_output_level_right = self.builder.get_object(
            'compressor_output_level_right')
        self.ui_compressor_gain_reduction_levelbar = self.builder.get_object(
            'compressor_gain_reduction_levelbar')

        self.ui_compressor_input_level_left_label = self.builder.get_object(
            'compressor_input_level_left_label')
        self.ui_compressor_input_level_right_label = self.builder.get_object(
            'compressor_input_level_right_label')
        self.ui_compressor_output_level_left_label = self.builder.get_object(
            'compressor_output_level_left_label')
        self.ui_compressor_output_level_right_label = self.builder.get_object(
            'compressor_output_level_right_label')
        self.ui_compressor_gain_reduction_level_label = \
            self.builder.get_object('compressor_gain_reduction_level_label')

        self.ui_compressor_gain_reduction_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_LOW', 6)
        self.ui_compressor_gain_reduction_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_HIGH', 18)
        self.ui_compressor_gain_reduction_levelbar.add_offset_value(
            'GTK_LEVEL_BAR_OFFSET_FULL', 24)

    def init_ui(self):
        self.compressor_user = self.settings.get_value(
            'compressor-user').unpack()
        self.apply_compressor_preset(self.compressor_user)

    def apply_compressor_preset(self, values):
        if values[0] == 0:
            self.ui_compressor_rms.set_active(True)
        elif values[0] == 1:
            self.ui_compressor_peak.set_active(True)

        self.ui_compressor_attack.set_value(values[1])
        self.ui_compressor_release.set_value(values[2])
        self.ui_compressor_threshold.set_value(values[3])
        self.ui_compressor_ratio.set_value(values[4])
        self.ui_compressor_knee.set_value(values[5])
        self.ui_compressor_makeup.set_value(values[6])

        # we need this when on value changed is not called
        self.compressor.set_property('rms-peak', values[0])
        self.compressor.set_property('attack-time', values[1])
        self.compressor.set_property('release-time', values[2])
        self.compressor.set_property('threshold-level', values[3])
        self.compressor.set_property('ratio', values[4])
        self.compressor.set_property('knee-radius', values[5])
        self.compressor.set_property('makeup-gain', values[6])

    def save_compressor_user(self, idx, value):
        self.compressor_user[idx] = value

        out = GLib.Variant('ad', self.compressor_user)

        self.settings.set_value('compressor-user', out)

    def on_compressor_measurement_type(self, obj):
        if obj.get_active():
            label = obj.get_label()

            if label == 'rms':
                self.compressor.set_property('rms-peak', 0)
                self.save_compressor_user(0, 0)
            elif label == 'peak':
                self.compressor.set_property('rms-peak', 1)
                self.save_compressor_user(0, 1)

    def on_compressor_attack_time_value_changed(self, obj):
        value = obj.get_value()
        self.compressor.set_property('attack-time', value)
        self.save_compressor_user(1, value)

    def on_compressor_release_time_value_changed(self, obj):
        value = obj.get_value()
        self.compressor.set_property('release-time', value)
        self.save_compressor_user(2, value)

    def on_compressor_threshold_value_changed(self, obj):
        value = obj.get_value()
        self.compressor.set_property('threshold-level', value)
        self.save_compressor_user(3, value)

    def on_compressor_ratio_value_changed(self, obj):
        value = obj.get_value()
        self.compressor.set_property('ratio', value)
        self.save_compressor_user(4, value)

    def on_compressor_knee_value_changed(self, obj):
        value = obj.get_value()
        self.compressor.set_property('knee-radius', value)
        self.save_compressor_user(5, value)

    def on_compressor_makeup_value_changed(self, obj):
        value = obj.get_value()
        self.compressor.set_property('makeup-gain', value)
        self.save_compressor_user(6, value)

    def on_compressor_preset_clicked(self, obj):
        obj_id = Gtk.Buildable.get_name(obj)

        if obj_id == 'no_compression':
            value = self.settings.get_value('compressor-no-compression')
            self.apply_compressor_preset(value)
        elif obj_id == 'vlc':
            value = self.settings.get_value('compressor-vlc')
            self.apply_compressor_preset(value)

    def ui_update_level(self, widgets, peak):
        left, right = peak[0], peak[1]

        widget_level_left = widgets[0]
        widget_level_right = widgets[1]
        widget_level_left_label = widgets[2]
        widget_level_right_label = widgets[3]

        if left >= -99:
            l_value = 10**(left / 20)
            widget_level_left.set_value(l_value)
            widget_level_left_label.set_text(str(round(left)))
        else:
            widget_level_left.set_value(0)
            widget_level_left_label.set_text('-99')

        if right >= -99:
            r_value = 10**(right / 20)
            widget_level_right.set_value(r_value)
            widget_level_right_label.set_text(str(round(right)))
        else:
            widget_level_right.set_value(0)
            widget_level_right_label.set_text('-99')

    def ui_update_compressor_input_level(self, peak):
        widgets = [self.ui_compressor_input_level_left,
                   self.ui_compressor_input_level_right,
                   self.ui_compressor_input_level_left_label,
                   self.ui_compressor_input_level_right_label]

        self.ui_update_level(widgets, peak)

    def ui_update_compressor_output_level(self, peak):
        widgets = [self.ui_compressor_output_level_left,
                   self.ui_compressor_output_level_right,
                   self.ui_compressor_output_level_left_label,
                   self.ui_compressor_output_level_right_label]

        self.ui_update_level(widgets, peak)

        gain_reduction = abs(round(
            self.compressor.get_property('gain-reduction')))

        if gain_reduction != self.old_compressor_gain_reduction:
            self.old_compressor_gain_reduction = gain_reduction

            self.ui_compressor_gain_reduction_levelbar.set_value(
                gain_reduction)
            self.ui_compressor_gain_reduction_level_label.set_text(
                str(round(gain_reduction)))

    def reset(self):
        self.settings.reset('compressor-user')
