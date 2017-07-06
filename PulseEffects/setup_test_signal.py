# -*- coding: utf-8 -*-

import os

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import GLib, Gtk


class SetupTestSignal():

    def __init__(self, app_builder, effects):
        self.app_builder = app_builder
        self.effects = effects
        self.module_path = os.path.dirname(__file__)
