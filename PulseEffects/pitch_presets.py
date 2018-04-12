# -*- coding: utf-8 -*-

from gi.repository import Gio, GLib


class PitchPresets():

    def __init__(self, config):
        self.config = config  # configParser

        self.settings_sourceoutputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs.pitch')

    def add_section(self, settings, section):
        enabled = settings.get_value('state')
        cents = settings.get_value('cents')
        semitones = settings.get_value('semitones')
        octaves = settings.get_value('octaves')
        crispness = settings.get_value('crispness')
        faster = settings.get_value('faster')
        preserve_formant = settings.get_value('preserve-formant')

        self.config[section] = {'enabled': str(enabled),
                                'cents': str(cents),
                                'semitones': str(semitones),
                                'octaves': str(octaves),
                                'crispness': str(crispness),
                                'faster': str(faster),
                                'preserve_formant': str(preserve_formant)}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        cents = self.config.getfloat(section, 'cents', fallback=0)
        semitones = self.config.getint(section, 'semitones', fallback=0)
        octaves = self.config.getint(section, 'octaves', fallback=0)
        crispness = self.config.getint(section, 'crispness', fallback=3)
        faster = self.config.getboolean(section, 'faster', fallback=False)
        preserve_formant = self.config.getboolean(section, 'preserve_formant',
                                                  fallback=False)

        settings.set_value('state', GLib.Variant('b', enabled))
        settings.set_value('cents', GLib.Variant('d', cents))
        settings.set_value('semitones', GLib.Variant('i', semitones))
        settings.set_value('octaves', GLib.Variant('i', octaves))
        settings.set_value('crispness', GLib.Variant('i', crispness))
        settings.set_value('faster', GLib.Variant('b', faster))
        settings.set_value('preserve-formant',
                           GLib.Variant('b', preserve_formant))

    def save(self):
        self.add_section(self.settings_sourceoutputs, 'mic_pitch')

    def load(self):
        self.load_section(self.settings_sourceoutputs, 'mic_pitch')
