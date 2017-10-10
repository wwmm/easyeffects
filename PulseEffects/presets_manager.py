# -*- coding: utf-8 -*-

import gettext
import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gio, GLib, Gtk

from PulseEffects.load_presets import LoadPresets
from PulseEffects.save_presets import SavePresets

gettext.textdomain('PulseEffects')
_ = gettext.gettext


class PresetsManager():

    def __init__(self, app):
        self.app = app
        self.module_path = os.path.dirname(__file__)

        self.dir = os.path.join(GLib.get_user_config_dir(), 'PulseEffects')

        self.lp = LoadPresets()
        self.sp = SavePresets()

        self.load_menu()

    def load_menu(self):
        self.menu_button = self.app.builder.get_object(
            'presets_popover_button')

        self.builder = Gtk.Builder.new_from_file(self.module_path +
                                                 '/ui/presets_menu.glade')

        menu = self.builder.get_object('menu')
        self.listbox = self.builder.get_object('listbox')
        self.scrolled_window = self.builder.get_object('scrolled_window')
        self.new_preset_name = self.builder.get_object('new_preset_name')

        # listbox style
        provider = Gtk.CssProvider()

        css_file = Gio.File.new_for_path(self.module_path + '/ui/listbox.css')

        provider.load_from_file(css_file)

        Gtk.StyleContext.add_provider(self.listbox.get_style_context(),
                                      provider,
                                      Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        menu.set_relative_to(self.menu_button)

        def button_clicked(arg, popover_menu):
            if popover_menu.get_visible():
                popover_menu.hide()
            else:
                w, h = self.app.window.get_size()

                self.scrolled_window.set_max_content_height(int(0.8 * h))

                popover_menu.show_all()

                # for some reason when listbox is inside a scrolledwindow
                # it autoselects a row at startup

                if self.menu_button.get_label() == _('Presets'):
                    self.listbox.unselect_all()

        self.menu_button.connect("clicked", button_clicked, menu)
        self.listbox.connect('row-activated', self.on_listbox_row_activated)

        self.init_listbox()

    def add_to_listbox(self, name):
        row = Gtk.ListBoxRow()
        row.set_name(name)
        row.set_margin_top(6)
        row.set_margin_bottom(6)
        row.set_margin_left(6)
        row.set_margin_right(6)

        box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=6)

        row.add(box)

        # preset label

        label = Gtk.Label(name, xalign=0)
        label.set_use_markup(True)
        label.set_valign(Gtk.Align.CENTER)

        # save button

        save_button = Gtk.Button()
        save_button.set_name(name)

        icon = Gtk.Image.new_from_icon_name('document-save-symbolic',
                                            Gtk.IconSize.SMALL_TOOLBAR)

        save_button.set_image(icon)
        save_button.set_valign(Gtk.Align.CENTER)
        save_button.set_halign(Gtk.Align.CENTER)
        save_button.set_tooltip_text(_('Save Current Settings to Preset'))

        save_button.connect('clicked', self.on_save)

        # delete button

        delete_button = Gtk.Button()
        delete_button.set_name(name)

        icon = Gtk.Image.new_from_icon_name('list-remove-symbolic',
                                            Gtk.IconSize.SMALL_TOOLBAR)

        delete_button.set_image(icon)
        delete_button.set_valign(Gtk.Align.CENTER)
        delete_button.set_halign(Gtk.Align.CENTER)
        delete_button.set_tooltip_text(_('Delete Preset'))

        context = delete_button.get_style_context()
        context.add_class('destructive-action')

        delete_button.connect('clicked', self.on_delete)

        box.pack_start(label, False, False, 0)
        box.pack_end(delete_button, False, False, 0)
        box.pack_end(save_button, False, False, 0)

        self.listbox.add(row)

    def init_listbox(self):
        children = self.listbox.get_children()

        for child in children:
            self.listbox.remove(child)

        file_list = os.listdir(self.dir)

        for f in file_list:
            if f.endswith('.preset'):
                name = f.split('.')[0]

                self.add_to_listbox(name)

        self.listbox.show_all()

    def save_preset(self, path):
        self.sp.set_output_path(path)

        self.sp.save_sink_inputs_presets(self.app.sie.settings)
        self.sp.save_source_outputs_presets(self.app.soe.settings)

        self.sp.write_config()

    def on_listbox_row_activated(self, obj, row):
        name = row.get_name()

        self.menu_button.set_label(name)

        path = os.path.join(self.dir, name + '.preset')

        self.lp.set_config_path(path)

        self.lp.load_sink_inputs_presets(self.app.sie.settings)
        self.lp.load_source_outputs_presets(self.app.soe.settings)

    def on_add_preset_clicked(self, obj):
        name = self.new_preset_name.get_text()

        if name.endswith('.preset'):
            name = name.split('.')[0]

            self.new_preset_name.set_text(name)

        if name:
            # checking if preset name already exists

            children = self.listbox.get_children()

            add_preset = True

            for child in children:
                if child.get_name() == name:
                    add_preset = False
                    break

            if add_preset:
                self.new_preset_name.set_text('')

                self.add_to_listbox(name)

                self.listbox.show_all()

                path = os.path.join(self.dir, name + ".preset")

                self.save_preset(path)

    def on_save(self, obj):
        path = os.path.join(self.dir, obj.get_name() + ".preset")

        self.save_preset(path)

    def on_delete(self, obj):
        name = obj.get_name()

        children = self.listbox.get_children()

        for child in children:
            if child.get_name() == name:
                self.listbox.remove(child)

                path = os.path.join(self.dir, name + ".preset")

                os.remove(path)
