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

        self.pm.connect('sink_inputs_changed', self.on_sink_inputs_changed)

        self.apps_box = self.builder.get_object('apps_box')

    def init(self):
        self.build_apps_list()

    def build_apps_list(self):
        children = self.apps_box.get_children()

        for child in children:
            self.apps_box.remove(child)

        sink_inputs = self.pm.sink_inputs

        for i in sink_inputs:
            idx = i[0]
            app_name = i[1]
            media_name = i[2]
            icon_name = i[3]
            volume = i[4]
            connected = i[5]

            row = Gtk.ListBoxRow()

            hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=2)

            row.add(hbox)

            # app icon
            icon_size = Gtk.IconSize.BUTTON
            icon = Gtk.Image.new_from_icon_name(icon_name, icon_size)

            hbox.pack_start(icon, False, False, 0)

            # label
            label_text = '<b>' + app_name + '</b>' + ': ' + media_name

            label = Gtk.Label(label_text, xalign=0)
            label.set_use_markup(True)

            hbox.pack_start(label, True, True, 0)

            # switch
            switch = Gtk.Switch()

            switch.set_active(connected)
            switch.set_name('switch_' + str(idx))

            def move_sink_input(obj, state):
                idx = int(obj.get_name().split('_')[1])

                if state:
                    self.pm.grab_input(idx)
                else:
                    self.pm.move_input_to_default_sink(idx)

            switch.connect('state-set', move_sink_input)

            hbox.pack_end(switch, False, False, 0)

            # volume

            volume_adjustment = Gtk.Adjustment(0, -100, 0, 1, 10, 0)

            volume_scale = Gtk.Scale(orientation=Gtk.Orientation.HORIZONTAL,
                                     adjustment=volume_adjustment)
            volume_scale.set_digits(2)
            volume_scale.set_value_pos(Gtk.PositionType.RIGHT)
            volume_scale.set_hexpand(True)

            max_volume = self.pm.get_max_volume(volume)

            volume_adjustment.set_value(self.pm.volume_to_dB(max_volume))

            hbox.pack_end(volume_scale, True, True, 0)

            self.apps_box.add(row)

        self.apps_box.show_all()

    def on_sink_inputs_changed(self, obj):
        if self.app.ui_initialized:
            self.build_apps_list()

        if len(self.pm.sink_inputs) > 0 or self.app.generating_test_signal:
            if not self.gst.is_playing:
                self.gst.set_state('playing')
        else:
            self.gst.set_state('paused')
