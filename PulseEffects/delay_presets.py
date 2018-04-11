# -*- coding: utf-8 -*-

from gi.repository import Gio, GLib


class DelayPresets():

    def __init__(self, config):
        self.config = config  # configParser

        self.settings_sinkinputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.delay')

    def add_section(self, settings, section):
        enabled = settings.get_value('state')
        m_l = settings.get_value('m-l')
        cm_l = settings.get_value('cm-l')
        m_r = settings.get_value('m-r')
        cm_r = settings.get_value('cm-r')
        temperature = settings.get_value('temperature')

        self.config[section] = {'enabled': str(enabled),
                                'm_l': str(m_l),
                                'cm_l': str(cm_l),
                                'm_r': str(m_r),
                                'cm_r': str(cm_r),
                                'temperature': str(temperature)}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        m_l = self.config.getfloat(section, 'm_l', fallback=0.0)
        cm_l = self.config.getfloat(section, 'cm_l', fallback=0.0)
        m_r = self.config.getfloat(section, 'm_r', fallback=0.0)
        cm_r = self.config.getfloat(section, 'cm_r', fallback=0.0)
        temperature = self.config.getfloat(section, 'temperature',
                                           fallback=20.0)

        settings.set_value('state', GLib.Variant('b', enabled))
        settings.set_value('m-l', GLib.Variant('d', m_l))
        settings.set_value('cm-l', GLib.Variant('d', cm_l))
        settings.set_value('m-r', GLib.Variant('d', m_r))
        settings.set_value('cm-r', GLib.Variant('d', cm_r))
        settings.set_value('temperature', GLib.Variant('d', temperature))

    def save(self):
        self.add_section(self.settings_sinkinputs, 'apps_delay')

    def load(self):
        self.load_section(self.settings_sinkinputs, 'apps_delay')
