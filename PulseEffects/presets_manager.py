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

        label = Gtk.Label(name, xalign=0)
        label.set_use_markup(True)
        label.set_valign(Gtk.Align.CENTER)

        row.add(label)

        row.set_name(name)

        row.set_margin_top(6)
        row.set_margin_bottom(6)

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
