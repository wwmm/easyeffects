# -*- coding: utf-8 -*-

from gi.repository import Gio, GLib


class WebrtcPresets():

    def __init__(self, config):
        self.config = config  # configParser

        self.settings_sourceoutputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs.webrtc')

    def add_section(self, settings, section):
        enabled = settings.get_value('state')

        self.config[section] = {'enabled': str(enabled)}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)

        settings.set_value('state', GLib.Variant('b', enabled))

    def save(self):
        self.add_section(self.settings_sourceoutputs, 'mic_webrtc')

    def load(self):
        self.load_section(self.settings_sourceoutputs, 'mic_webrtc')
