# -*- coding: utf-8 -*-

import logging

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, Pango


class ListSourceOutputs():

    def __init__(self, app_builder, effects, pulse_manager):
        self.builder = app_builder
        self.soe = effects
        self.pm = pulse_manager

        self.changing_source_output_volume = False
        self.handlers = {}

        self.log = logging.getLogger('PulseEffects')

        self.apps_box = self.builder.get_object('apps_box')

    def init(self):
        pass

    def connect_signals(self):
        self.pm.connect('source_output_added', self.on_source_output_added)
        self.pm.connect('source_output_changed', self.on_source_output_changed)
        self.pm.connect('source_output_removed', self.on_source_output_removed)

    def init_source_output_ui(self, app_box, source_output_parameters):
        idx = source_output_parameters[0]
        app_name = source_output_parameters[1]
        media_name = source_output_parameters[2]
        icon_name = source_output_parameters[3]
        audio_channels = source_output_parameters[4]
        max_volume_linear = source_output_parameters[5]
        rate = source_output_parameters[6]
        resample_method = source_output_parameters[7]
        sample_format = source_output_parameters[8]
        mute = source_output_parameters[9]
        connected = source_output_parameters[10]

        app_box.set_name('app_box_' + str(idx))
        app_box.set_homogeneous(True)

        info_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)
        control_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL,
                              spacing=0)

        app_box.pack_start(info_box, True, True, 0)
        app_box.pack_end(control_box, True, True, 0)

        # app icon
        icon = Gtk.Image.new_from_icon_name(icon_name,
                                            Gtk.IconSize.SMALL_TOOLBAR)

        icon.set_valign(Gtk.Align.CENTER)
        icon.set_margin_right(2)

        info_box.pack_start(icon, False, False, 0)

        # label
        label_text = '<b>' + app_name + '</b>' + ': ' + media_name

        label = Gtk.Label(label_text, xalign=0)
        label.set_use_markup(True)
        label.set_ellipsize(Pango.EllipsizeMode.END)
        label.set_valign(Gtk.Align.CENTER)

        info_box.pack_start(label, True, True, 0)

        # format, rate, channels and resample method
        label_text = sample_format + ', ' + \
            str(round(rate / 1000.0, 1)) + ' kHz, ' + \
            str(audio_channels) + 'ch, ' + resample_method

        label = Gtk.Label(label_text, xalign=0)
        label.set_margin_left(5)
        label.set_valign(Gtk.Align.CENTER)
        label.set_sensitive(False)

        info_box.pack_end(label, False, False, 0)

        # switch
        switch = Gtk.Switch()

        switch.set_active(connected)
        switch.set_name('switch_' + str(idx))
        switch.set_valign(Gtk.Align.CENTER)
        switch.set_margin_left(2)

        def move_source_output(obj, state):
            idx = int(obj.get_name().split('_')[1])

            if state:
                self.pm.move_source_output_to_pulseeffects_source(idx)
            else:
                self.pm.move_source_output_to_default_source(idx)

        switch.connect('state-set', move_source_output)

        control_box.pack_end(switch, False, False, 0)

        # volume
        volume_adjustment = Gtk.Adjustment(0, 0, 100, 1, 5, 0)

        volume_scale = Gtk.Scale(orientation=Gtk.Orientation.HORIZONTAL,
                                 adjustment=volume_adjustment)
        volume_scale.set_digits(0)
        volume_scale.set_value_pos(Gtk.PositionType.RIGHT)
        volume_scale.set_name('volume_' + str(idx) + '_' + str(audio_channels))
        volume_scale.set_valign(Gtk.Align.CENTER)

        volume_adjustment.set_value(max_volume_linear)

        def set_source_output_volume(obj):
            data = obj.get_name().split('_')
            idx = int(data[1])
            audio_channels = int(data[2])

            self.pm.set_source_output_volume(
                idx, audio_channels, obj.get_value())

        def slider_pressed(obj, event):
            self.changing_source_output_volume = True

        def slider_released(obj, event):
            self.changing_source_output_volume = False

        volume_scale.connect('button-press-event', slider_pressed)
        volume_scale.connect('button-release-event', slider_released)
        volume_scale.connect('value-changed', set_source_output_volume)

        control_box.pack_end(volume_scale, True, True, 0)

        # mute
        icon_name = 'audio-volume-high-symbolic'

        if mute:
            icon_name = 'audio-volume-muted-symbolic'

            volume_scale.set_sensitive(False)

        icon = Gtk.Image.new_from_icon_name(icon_name, Gtk.IconSize.BUTTON)

        mute_button = Gtk.ToggleButton()
        mute_button.set_image(icon)
        mute_button.set_margin_left(4)
        mute_button.set_name('mute_' + str(idx))
        mute_button.set_valign(Gtk.Align.CENTER)

        mute_button.set_active(mute)

        def on_mute_button_toggled(button):
            idx = int(button.get_name().split('_')[1])

            state = button.get_active()

            self.pm.set_source_output_mute(idx, state)

            parent = button.get_parent()

            for child in parent:
                data = child.get_name().split('_')

                if data[0] == 'volume':
                    child.set_sensitive(state)

                    break

        mute_button.connect('toggled', on_mute_button_toggled)

        control_box.pack_end(mute_button, False, False, 0)

    def on_source_output_added(self, obj, source_output_parameters):
        app_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL,
                          spacing=0)

        self.init_source_output_ui(app_box, source_output_parameters)

        self.apps_box.add(app_box)

        self.apps_box.show_all()

        if not self.soe.is_playing:
            self.soe.set_state('playing')

    def on_source_output_changed(self, obj, source_output_parameters):
        idx = source_output_parameters[0]

        children = self.apps_box.get_children()

        for child in children:
            child_name = child.get_name()

            if child_name == 'app_box_' + str(idx):
                if not self.changing_source_output_volume:
                    for c in child.get_children():
                        child.remove(c)

                    self.init_source_output_ui(child, source_output_parameters)

                    self.apps_box.show_all()

                break

    def on_source_output_removed(self, obj, idx):
        children = self.apps_box.get_children()

        n_children_before = len(children)

        for child in children:
            child_name = child.get_name()

            if child_name == 'app_box_' + str(idx):
                self.apps_box.remove(child)

                break

        n_children_after = len(self.apps_box.get_children())

        if n_children_before == 1 and n_children_after == 0:
            self.soe.set_state('paused')
