# -*- coding: utf-8 -*-
import gettext

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, Gtk, Pango
from PulseEffects.effects_base import EffectsBase

gettext.textdomain('PulseEffects')
_ = gettext.gettext


class SourceOutputEffects(EffectsBase):

    def __init__(self, pulse_manager):
        self.pm = pulse_manager
        self.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs')

        EffectsBase.__init__(self, self.pm.default_source_rate, self.settings)

        self.log_tag = 'mic: '
        self.changing_source_output_volume = False
        self.switch_on_all_apps = False

        self.set_source_monitor_name(self.pm.default_source_name)
        self.set_output_sink_name('PulseEffects_mic')

        self.pm.connect('source_output_added', self.on_source_output_added)
        self.pm.connect('source_output_changed', self.on_source_output_changed)
        self.pm.connect('source_output_removed', self.on_source_output_removed)

        self.listbox.show_all()

        # it makes no sense to show the calibration button here
        self.equalizer.ui_eq_calibrate_button.destroy()

        # adding effects widgets to the stack
        self.stack.add_named(self.limiter.ui_window, 'limiter')
        self.stack.add_named(self.compressor.ui_window, 'compressor')
        self.stack.add_named(self.reverb.ui_window, 'reverb')
        self.stack.add_named(self.highpass.ui_window, 'highpass')
        self.stack.add_named(self.lowpass.ui_window, 'lowpass')
        self.stack.add_named(self.equalizer.ui_window, 'equalizer')

        # on/off switches connections
        self.compressor.ui_enable.connect('state-set',
                                          self.on_compressor_enable)
        self.reverb.ui_enable.connect('state-set', self.on_reverb_enable)
        self.highpass.ui_enable.connect('state-set', self.on_highpass_enable)
        self.lowpass.ui_enable.connect('state-set', self.on_lowpass_enable)
        self.equalizer.ui_enable.connect('state-set', self.on_equalizer_enable)

        # order is important
        if self.limiter.is_installed:
            self.limiter.bind()
        else:
            self.limiter.ui_window.set_sensitive(False)
            self.limiter.ui_limiter_enable.set_sensitive(False)

        if self.compressor.is_installed:
            self.compressor.bind()
        else:
            self.compressor.ui_window.set_sensitive(False)
            self.compressor.ui_enable.set_sensitive(False)

        self.reverb.bind()
        self.highpass.bind()
        self.lowpass.bind()
        self.equalizer.bind()

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

        if not self.is_playing:
            self.set_state('playing')

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
            self.set_state('ready')

    def reset(self):
        self.limiter.reset()
        self.compressor.reset()
        self.reverb.reset()
        self.highpass.reset()
        self.lowpass.reset()
        self.equalizer.reset()
