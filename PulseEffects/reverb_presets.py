# -*- coding: utf-8 -*-

from gi.repository import Gio, GLib


class ReverbPresets():

    def __init__(self, config):
        self.config = config  # configParser

        self.settings_sinkinputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.reverb')

        self.settings_sourceoutputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs.reverb')

    def add_section(self, settings, section):
        enabled = settings.get_value('state')
        room_size = settings.get_value('room-size')
        damping = settings.get_value('damping')
        width = settings.get_value('width')
        level = settings.get_value('level')

        self.config[section] = {'enabled': str(enabled),
                                'room size': str(room_size),
                                'damping': str(damping),
                                'width': str(width),
                                'level': str(level)}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        room_size = self.config.getfloat(section, 'room size', fallback=0.5)
        damping = self.config.getfloat(section, 'damping', fallback=0.2)
        width = self.config.getfloat(section, 'width', fallback=1.0)
        level = self.config.getfloat(section, 'level', fallback=0.5)

        settings.set_value('state', GLib.Variant('b', enabled))
        settings.set_value('room-size', GLib.Variant('d', room_size))
        settings.set_value('damping', GLib.Variant('d', damping))
        settings.set_value('width', GLib.Variant('d', width))
        settings.set_value('level', GLib.Variant('d', level))

    def save(self):
        self.add_section(self.settings_sinkinputs, 'apps_reverb')
        self.add_section(self.settings_sourceoutputs, 'mic_reverb')

    def load(self):
        self.load_section(self.settings_sinkinputs, 'apps_reverb')
        self.load_section(self.settings_sourceoutputs, 'mic_reverb')
