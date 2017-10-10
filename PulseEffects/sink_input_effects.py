# -*- coding: utf-8 -*-

import gettext

import gi
gi.require_version('GstInsertBin', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GstInsertBin, Gtk, Pango
from PulseEffects.effects_base import EffectsBase
from PulseEffects.panorama import Panorama


gettext.textdomain('PulseEffects')
_ = gettext.gettext


class SinkInputEffects(EffectsBase):

    def __init__(self, pulse_manager):
        self.pm = pulse_manager
        self.settings = Gio.Settings('com.github.wwmm.pulseeffects.sinkinputs')

        EffectsBase.__init__(self, self.pm.default_sink_rate, self.settings)

        self.log_tag = 'apps: '
        self.changing_sink_input_volume = False
        self.switch_on_all_apps = False
        self.panorama_ready = False

        self.set_source_monitor_name(self.pm.apps_sink_monitor_name)
        self.set_output_sink_name(self.pm.default_sink_name)

        self.pm.connect('sink_input_added', self.on_sink_input_added)
        self.pm.connect('sink_input_changed', self.on_sink_input_changed)
        self.pm.connect('sink_input_removed', self.on_sink_input_removed)

        self.panorama = Panorama(self.settings)

        self.insert_in_listbox('panorama', 1)

        self.listbox.show_all()

        # adding effects widgets to the stack

        self.stack.add_named(self.limiter.ui_window, 'Limiter')
        self.stack.add_named(self.panorama.ui_window, 'Panorama')
        self.stack.add_named(self.compressor.ui_window, 'Compressor')
        self.stack.add_named(self.reverb.ui_window, 'Reverb')
        self.stack.add_named(self.highpass.ui_window, 'Highpass')
        self.stack.add_named(self.lowpass.ui_window, 'Lowpass')
        self.stack.add_named(self.equalizer.ui_window, 'Equalizer')

        # on/off switches connections
        self.panorama.ui_enable.connect('state-set', self.on_panorama_enable)
        self.compressor.ui_enable.connect('state-set',
                                          self.on_compressor_enable)
        self.reverb.ui_enable.connect('state-set', self.on_reverb_enable)
        self.highpass.ui_enable.connect('state-set', self.on_highpass_enable)
        self.lowpass.ui_enable.connect('state-set', self.on_lowpass_enable)
        self.equalizer.ui_enable.connect('state-set', self.on_equalizer_enable)

        # effects wrappers
        self.panorama_wrapper = GstInsertBin.InsertBin.new('panorama_wrapper')

        # appending effects wrappers to effects bin
        self.effects_bin.insert_after(self.panorama_wrapper,
                                      self.limiter_wrapper,
                                      self.on_filter_added,
                                      self.log_tag)

        # order of bind is important and may lead to load failure if
        # done otherwise

        if self.limiter.is_installed:
            self.limiter.bind()
        else:
            self.limiter.ui_window.set_sensitive(False)
            self.limiter.ui_limiter_enable.set_sensitive(False)

        if self.panorama.is_installed:
            self.panorama.bind()
        else:
            self.panorama.ui_window.set_sensitive(False)
            self.panorama.ui_enable.set_sensitive(False)

        if self.compressor.is_installed:
            self.compressor.bind()
        else:
            self.compressor.ui_window.set_sensitive(False)
            self.compressor.ui_enable.set_sensitive(False)

        self.reverb.bind()
        self.highpass.bind()
        self.lowpass.bind()
        self.equalizer.bind()

    def init_sink_input_ui(self, app_box, sink_input_parameters):
        idx = sink_input_parameters[0]
        app_name = sink_input_parameters[1]
        media_name = sink_input_parameters[2]
        icon_name = sink_input_parameters[3]
        audio_channels = sink_input_parameters[4]
        max_volume_linear = sink_input_parameters[5]
        rate = sink_input_parameters[6]
        resample_method = sink_input_parameters[7]
        sample_format = sink_input_parameters[8]
        mute = sink_input_parameters[9]
        connected = sink_input_parameters[10]

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

        switch.set_name('switch_' + str(idx))
        switch.set_valign(Gtk.Align.CENTER)
        switch.set_margin_left(2)

        def move_sink_input(obj, state):
            idx = int(obj.get_name().split('_')[1])

            if state:
                self.pm.move_sink_input_to_pulseeffects_sink(idx)
            else:
                self.pm.move_sink_input_to_default_sink(idx)

        switch.connect('state-set', move_sink_input)

        if self.switch_on_all_apps:
            switch.set_active(True)
            switch.set_sensitive(False)
        else:
            switch.set_active(connected)

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

        def set_sink_input_volume(obj):
            data = obj.get_name().split('_')
            idx = int(data[1])
            audio_channels = int(data[2])

            self.pm.set_sink_input_volume(idx, audio_channels, obj.get_value())

        def slider_pressed(obj, event):
            self.changing_sink_input_volume = True

        def slider_released(obj, event):
            self.changing_sink_input_volume = False

        volume_scale.connect('button-press-event', slider_pressed)
        volume_scale.connect('button-release-event', slider_released)
        volume_scale.connect('value-changed', set_sink_input_volume)

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

            self.pm.set_sink_input_mute(idx, state)

            parent = button.get_parent()

            for child in parent:
                data = child.get_name().split('_')

                if data[0] == 'volume':
                    child.set_sensitive(state)

                    break

        mute_button.connect('toggled', on_mute_button_toggled)

        control_box.pack_end(mute_button, False, False, 0)

    def on_sink_input_added(self, obj, sink_input_parameters):
        app_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL,
                          spacing=0)

        self.init_sink_input_ui(app_box, sink_input_parameters)

        self.apps_box.add(app_box)

        self.apps_box.show_all()

        if not self.is_playing:
            self.set_state('playing')

    def on_sink_input_changed(self, obj, sink_input_parameters):
        idx = sink_input_parameters[0]

        children = self.apps_box.get_children()

        for child in children:
            child_name = child.get_name()

            if child_name == 'app_box_' + str(idx):
                if not self.changing_sink_input_volume:
                    for c in child.get_children():
                        child.remove(c)

                    self.init_sink_input_ui(child, sink_input_parameters)

                    self.apps_box.show_all()

                break

    def on_sink_input_removed(self, obj, idx):
        children = self.apps_box.get_children()

        n_children_before = len(children)

        for child in children:
            child_name = child.get_name()

            if child_name == 'app_box_' + str(idx):
                self.apps_box.remove(child)
                break

        n_children_after = len(self.apps_box.get_children())

        if n_children_before == 1 and n_children_after == 0:
            self.set_state('ready')

    def on_listbox_row_activated(self, obj, row):
        EffectsBase.on_listbox_row_activated(self, obj, row)

        name = row.get_name()

        if name == 'panorama':
            self.stack.set_visible_child(self.panorama.ui_window)

    def on_message_element(self, bus, msg):
        EffectsBase.on_message_element(self, bus, msg)

        plugin = msg.src.get_name()

        if plugin == 'panorama_input_level':
            peak = msg.get_structure().get_value('peak')

            self.panorama.ui_update_panorama_input_level(peak)
        elif plugin == 'panorama_output_level':
            peak = msg.get_structure().get_value('peak')

            self.panorama.ui_update_panorama_output_level(peak)

        return True

    def on_panorama_enable(self, obj, state):
        if state:
            self.panorama_wrapper.append(self.panorama.bin,
                                         self.on_filter_added,
                                         self.log_tag)
        else:
            self.panorama_wrapper.remove(self.panorama.bin,
                                         self.on_filter_removed,
                                         self.log_tag)

    def reset(self):
        self.limiter.reset()
        self.panorama.reset()
        self.compressor.reset()
        self.reverb.reset()
        self.highpass.reset()
        self.lowpass.reset()
        self.equalizer.reset()
