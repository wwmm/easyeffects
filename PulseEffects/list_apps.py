# -*- coding: utf-8 -*-

import logging

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk


class ListApps():

    def __init__(self, app):
        self.app = app
        self.builder = app.builder
        self.pm = app.pm
        self.gst = app.gst

        self.handlers = {}

        self.log = logging.getLogger('PulseEffects')

        self.pm.connect('sink_input_added', self.on_sink_input_added)
        self.pm.connect('sink_input_changed', self.on_sink_input_changed)
        self.pm.connect('sink_input_removed', self.on_sink_input_removed)

        self.apps_box = self.builder.get_object('apps_box')

    def init(self):
        pass

    def init_sink_input_ui(self, app_box, sink_input_parameters):
        idx = sink_input_parameters[0]
        app_name = sink_input_parameters[1]
        media_name = sink_input_parameters[2]
        icon_name = sink_input_parameters[3]
        audio_channels = sink_input_parameters[4]
        max_volume_dB = sink_input_parameters[5]
        rate = sink_input_parameters[6]
        resample_method = sink_input_parameters[7]
        connected = sink_input_parameters[8]

        app_box.set_name('app_box_' + str(idx))
        app_box.set_homogeneous(True)

        info_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=2)
        control_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL,
                              spacing=0)

        app_box.pack_start(info_box, True, True, 0)
        app_box.pack_end(control_box, True, True, 0)

        # app icon
        icon_size = Gtk.IconSize.BUTTON
        icon = Gtk.Image.new_from_icon_name(icon_name, icon_size)

        info_box.pack_start(icon, False, False, 0)

        # label
        label_text = '<b>' + app_name + '</b>' + ': ' + media_name

        label = Gtk.Label(label_text, xalign=0)
        label.set_use_markup(True)

        info_box.pack_start(label, True, True, 0)

        # rate and resample method
        label_text = str(rate) + ' Hz, ' + resample_method
        label = Gtk.Label(label_text, xalign=0)

        info_box.pack_end(label, False, False, 0)

        # switch
        switch = Gtk.Switch()

        switch.set_active(connected)
        switch.set_name('switch_' + str(idx))

        def move_sink_input(obj, state):
            idx = int(obj.get_name().split('_')[1])

            if state:
                self.pm.move_input_to_pulseeffects_sink(idx)
            else:
                self.pm.move_input_to_default_sink(idx)

        switch.connect('state-set', move_sink_input)

        control_box.pack_end(switch, False, False, 0)

        # volume
        volume_adjustment = Gtk.Adjustment(0, -100, 0, 1, 2, 0)

        volume_scale = Gtk.Scale(orientation=Gtk.Orientation.HORIZONTAL,
                                 adjustment=volume_adjustment)
        volume_scale.set_digits(1)
        volume_scale.set_value_pos(Gtk.PositionType.RIGHT)
        volume_scale.set_name('volume_' + str(idx) + '_' + str(audio_channels))

        volume_adjustment.set_value(max_volume_dB)

        def set_sink_input_volume(obj):
            data = obj.get_name().split('_')
            idx = int(data[1])
            audio_channels = int(data[2])

            self.pm.set_sink_input_volume(idx, audio_channels, obj.get_value())

        volume_scale.connect('value-changed', set_sink_input_volume)

        control_box.pack_end(volume_scale, True, True, 0)

    def on_sink_input_added(self, obj, sink_input_parameters):
        if self.app.ui_initialized:
            app_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL,
                              spacing=2)

            self.init_sink_input_ui(app_box, sink_input_parameters)

            self.apps_box.add(app_box)

            self.apps_box.show_all()

            if not self.gst.is_playing:
                self.gst.set_state('playing')
                self.log.info('pipeline state: playing')

    def on_sink_input_changed(self, obj, sink_input_parameters):
        if self.app.ui_initialized:
            idx = sink_input_parameters[0]

            children = self.apps_box.get_children()

            for child in children:
                child_name = child.get_name()

                if child_name == 'app_box_' + str(idx):
                    for c in child.get_children():
                        child.remove(c)

                    self.init_sink_input_ui(child, sink_input_parameters)

                    self.apps_box.show_all()

                    break

    def on_sink_input_removed(self, obj, idx):
        if self.app.ui_initialized:
            children = self.apps_box.get_children()

            for child in children:
                child_name = child.get_name()

                if child_name == 'app_box_' + str(idx):
                    self.apps_box.remove(child)

                    break

            if (len(self.apps_box.get_children()) == 0 and
                    not self.app.generating_test_signal):
                self.gst.set_state('paused')
                self.log.info('pipeline state: paused')
