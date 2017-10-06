# -*- coding: utf-8 -*-

import gettext
import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import GLib, Gtk

from PulseEffects.load_presets import LoadPresets

gettext.textdomain('PulseEffects')
_ = gettext.gettext


class PresetsManager():

    def __init__(self, app):
        self.app = app
        self.module_path = os.path.dirname(__file__)

        self.dir = os.path.join(GLib.get_user_config_dir(), 'PulseEffects')

        self.lp = LoadPresets()

        self.load_menu()

    def load_menu(self):
        self.menu_button = self.app.builder.get_object(
            'presets_popover_button')

        self.builder = Gtk.Builder()
        self.builder.add_from_file(self.module_path + '/ui/presets_menu.glade')

        menu = self.builder.get_object('menu')
        self.listbox = self.builder.get_object('listbox')

        menu.set_relative_to(self.menu_button)

        def button_clicked(arg, popover_menu):
            if popover_menu.get_visible():
                popover_menu.hide()
            else:
                popover_menu.show_all()

        self.menu_button.connect("clicked", button_clicked, menu)
        self.listbox.connect('row-activated', self.on_listbox_row_activated)

        self.init_listbox()

    def add_to_listbox(self, name):
        row = Gtk.ListBoxRow()
        row.set_name(name)
        row.set_margin_top(6)
        row.set_margin_bottom(6)

        box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)
        # box.set_homogeneous(True)

        row.add(box)

        # preset label

        label = Gtk.Label(name, xalign=0)
        label.set_use_markup(True)
        label.set_valign(Gtk.Align.CENTER)

        # save button

        save_button = Gtk.Button()

        icon = Gtk.Image.new_from_icon_name('document-save-symbolic',
                                            Gtk.IconSize.SMALL_TOOLBAR)

        save_button.set_image(icon)
        save_button.set_valign(Gtk.Align.CENTER)
        save_button.set_halign(Gtk.Align.CENTER)

        # delete button

        delete_button = Gtk.Button()

        icon = Gtk.Image.new_from_icon_name('list-remove-symbolic',
                                            Gtk.IconSize.SMALL_TOOLBAR)

        delete_button.set_image(icon)
        delete_button.set_valign(Gtk.Align.CENTER)
        delete_button.set_halign(Gtk.Align.CENTER)

        context = delete_button.get_style_context()
        context.add_class('destructive-action')

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

    def on_listbox_row_activated(self, obj, row):
        name = row.get_name()

        self.menu_button.set_label(name)

        path = os.path.join(self.dir, name + '.preset')

        self.lp.set_config_path(path)

        self.lp.load_sink_inputs_presets(self.app.sie.settings)
        self.lp.load_source_outputs_presets(self.app.soe.settings)
