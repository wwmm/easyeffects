# -*- coding: utf-8 -*-

import logging
import os

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GObject, Gst, GstInsertBin, Gtk

Gst.init(None)


class Pitch():

    def __init__(self, settings):
        self.settings = settings
        self.module_path = os.path.dirname(__file__)

        self.log = logging.getLogger('PulseEffects')

        self.old_compressor_gain_reduction = 0

        if Gst.ElementFactory.make(
                'ladspa-ladspa-rubberband-so-rubberband-pitchshifter-stereo'):
            self.is_installed = True
        else:
            self.is_installed = False

            self.log.warn('Pitch plugin was not found. Disabling it!')

        self.build_bin()

    def on_filter_added(self, bin, element, success, user_data):
        pass

    def build_bin(self):
        self.pitch = Gst.ElementFactory.make(
            'ladspa-ladspa-rubberband-so-rubberband-pitchshifter-stereo', None)
        self.input_level = Gst.ElementFactory.make('level',
                                                   'pitch_input_level')
        self.output_level = Gst.ElementFactory.make('level',
                                                    'pitch_output_level')

        self.bin = GstInsertBin.InsertBin.new('pitch_bin')

        if self.is_installed:
            self.bin.append(self.input_level, self.on_filter_added, None)
            self.bin.append(self.pitch, self.on_filter_added, None)
            self.bin.append(self.output_level, self.on_filter_added, None)

    def post_messages(self, state):
        self.input_level.set_property('post-messages', state)
        self.output_level.set_property('post-messages', state)

    def init_ui(self):
        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/pitch.glade')
        self.builder.connect_signals(self)

        self.ui_window = self.builder.get_object('window')
        self.ui_controls = self.builder.get_object('controls')
        self.ui_listbox_control = self.builder.get_object('listbox_control')

        self.ui_enable = self.builder.get_object('enable')
        self.ui_img_state = self.builder.get_object('img_state')
        self.ui_cents = self.builder.get_object('cents')
        self.ui_semitones = self.builder.get_object('semitones')
        self.ui_octaves = self.builder.get_object('octaves')
        self.ui_crispness = self.builder.get_object('crispness')
        self.ui_faster = self.builder.get_object('faster')
        self.ui_preserve_formant = self.builder.get_object('preserve_formant')

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

        self.settings.bind('pitch-state', self.ui_enable, 'active', flag)
        self.settings.bind('pitch-state', self.ui_img_state, 'visible',
                           flag)
        self.settings.bind('pitch-state', self.ui_controls, 'sensitive',
                           Gio.SettingsBindFlags.GET)

        self.settings.bind('pitch-cents', self.ui_cents, 'value', flag)
        self.settings.bind('pitch-semitones', self.ui_semitones, 'value', flag)
        self.settings.bind('pitch-octaves', self.ui_octaves, 'value', flag)
        self.settings.bind('pitch-crispness', self.ui_crispness, 'value', flag)
        self.settings.bind('pitch-faster', self.ui_faster, 'active', flag)
        self.settings.bind('pitch-preserve-formant', self.ui_preserve_formant,
                           'active', flag)

        # binding ui widgets to gstreamer plugins

        flag = GObject.BindingFlags.BIDIRECTIONAL | \
            GObject.BindingFlags.SYNC_CREATE

        self.ui_cents.bind_property('value', self.pitch, 'cents', flag)
        self.ui_semitones.bind_property('value', self.pitch, 'semitones', flag)
        self.ui_octaves.bind_property('value', self.pitch, 'octaves', flag)
        self.ui_crispness.bind_property('value', self.pitch, 'crispness', flag)
        self.ui_faster.bind_property('active', self.pitch, 'faster', flag)
        self.ui_preserve_formant.bind_property('active', self.pitch,
                                               'formant-preserving', flag)

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
        self.settings.reset('pitch-state')
        self.settings.reset('pitch-cents')
        self.settings.reset('pitch-semitones')
        self.settings.reset('pitch-octaves')
        self.settings.reset('pitch-crispness')
        self.settings.reset('pitch-formant-preserving')
        self.settings.reset('pitch-faster')
