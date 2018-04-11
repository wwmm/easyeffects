# -*- coding: utf-8 -*-

from gi.repository import Gio, GLib


class EqualizerPresets():

    def __init__(self, config):
        self.config = config  # configParser

        self.settings_sinkinputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sinkinputs.equalizer')

        self.settings_sourceoutputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs.equalizer')

    def add_section(self, settings, section):
        enabled = settings.get_value('state')
        input_gain = settings.get_value('input-gain')
        output_gain = settings.get_value('output-gain')

        gain, frequencies, qualities, types = [], [], [], []

        for n in range(30):
            gain.append(
                settings.get_value('band' + str(n) + '-gain'))

            frequencies.append(
                settings.get_value('band' + str(n) + '-frequency'))

            qualities.append(
                settings.get_value('band' + str(n) + '-quality'))

            types.append(
                settings.get_value('band' + str(n) + '-type'))

        self.config[section] = {'enabled': str(enabled),
                                'input_gain': str(input_gain),
                                'output_gain': str(output_gain),
                                'band0': str(gain[0]),
                                'band1': str(gain[1]),
                                'band2': str(gain[2]),
                                'band3': str(gain[3]),
                                'band4': str(gain[4]),
                                'band5': str(gain[5]),
                                'band6': str(gain[6]),
                                'band7': str(gain[7]),
                                'band8': str(gain[8]),
                                'band9': str(gain[9]),
                                'band10': str(gain[10]),
                                'band11': str(gain[11]),
                                'band12': str(gain[12]),
                                'band13': str(gain[13]),
                                'band14': str(gain[14]),
                                'band15': str(gain[15]),
                                'band16': str(gain[16]),
                                'band17': str(gain[17]),
                                'band18': str(gain[18]),
                                'band19': str(gain[19]),
                                'band20': str(gain[20]),
                                'band21': str(gain[21]),
                                'band22': str(gain[22]),
                                'band23': str(gain[23]),
                                'band24': str(gain[24]),
                                'band25': str(gain[25]),
                                'band26': str(gain[26]),
                                'band27': str(gain[27]),
                                'band28': str(gain[28]),
                                'band29': str(gain[29]),
                                'band0_freq': str(frequencies[0]),
                                'band1_freq': str(frequencies[1]),
                                'band2_freq': str(frequencies[2]),
                                'band3_freq': str(frequencies[3]),
                                'band4_freq': str(frequencies[4]),
                                'band5_freq': str(frequencies[5]),
                                'band6_freq': str(frequencies[6]),
                                'band7_freq': str(frequencies[7]),
                                'band8_freq': str(frequencies[8]),
                                'band9_freq': str(frequencies[9]),
                                'band10_freq': str(frequencies[10]),
                                'band11_freq': str(frequencies[11]),
                                'band12_freq': str(frequencies[12]),
                                'band13_freq': str(frequencies[13]),
                                'band14_freq': str(frequencies[14]),
                                'band15_freq': str(frequencies[15]),
                                'band16_freq': str(frequencies[16]),
                                'band17_freq': str(frequencies[17]),
                                'band18_freq': str(frequencies[18]),
                                'band19_freq': str(frequencies[19]),
                                'band20_freq': str(frequencies[20]),
                                'band21_freq': str(frequencies[21]),
                                'band22_freq': str(frequencies[22]),
                                'band23_freq': str(frequencies[23]),
                                'band24_freq': str(frequencies[24]),
                                'band25_freq': str(frequencies[25]),
                                'band26_freq': str(frequencies[26]),
                                'band27_freq': str(frequencies[27]),
                                'band28_freq': str(frequencies[28]),
                                'band29_freq': str(frequencies[29]),
                                'band0_qfactor': str(qualities[0]),
                                'band1_qfactor': str(qualities[1]),
                                'band2_qfactor': str(qualities[2]),
                                'band3_qfactor': str(qualities[3]),
                                'band4_qfactor': str(qualities[4]),
                                'band5_qfactor': str(qualities[5]),
                                'band6_qfactor': str(qualities[6]),
                                'band7_qfactor': str(qualities[7]),
                                'band8_qfactor': str(qualities[8]),
                                'band9_qfactor': str(qualities[9]),
                                'band10_qfactor': str(qualities[10]),
                                'band11_qfactor': str(qualities[11]),
                                'band12_qfactor': str(qualities[12]),
                                'band13_qfactor': str(qualities[13]),
                                'band14_qfactor': str(qualities[14]),
                                'band15_qfactor': str(qualities[15]),
                                'band16_qfactor': str(qualities[16]),
                                'band17_qfactor': str(qualities[17]),
                                'band18_qfactor': str(qualities[18]),
                                'band19_qfactor': str(qualities[19]),
                                'band20_qfactor': str(qualities[20]),
                                'band21_qfactor': str(qualities[21]),
                                'band22_qfactor': str(qualities[22]),
                                'band23_qfactor': str(qualities[23]),
                                'band24_qfactor': str(qualities[24]),
                                'band25_qfactor': str(qualities[25]),
                                'band26_qfactor': str(qualities[26]),
                                'band27_qfactor': str(qualities[27]),
                                'band28_qfactor': str(qualities[28]),
                                'band29_qfactor': str(qualities[29]),
                                'band0_type': str(types[0]),
                                'band1_type': str(types[1]),
                                'band2_type': str(types[2]),
                                'band3_type': str(types[3]),
                                'band4_type': str(types[4]),
                                'band5_type': str(types[5]),
                                'band6_type': str(types[6]),
                                'band7_type': str(types[7]),
                                'band8_type': str(types[8]),
                                'band9_type': str(types[9]),
                                'band10_type': str(types[10]),
                                'band11_type': str(types[11]),
                                'band12_type': str(types[12]),
                                'band13_type': str(types[13]),
                                'band14_type': str(types[14]),
                                'band15_type': str(types[15]),
                                'band16_type': str(types[16]),
                                'band17_type': str(types[17]),
                                'band18_type': str(types[18]),
                                'band19_type': str(types[19]),
                                'band20_type': str(types[20]),
                                'band21_type': str(types[21]),
                                'band22_type': str(types[22]),
                                'band23_type': str(types[23]),
                                'band24_type': str(types[24]),
                                'band25_type': str(types[25]),
                                'band26_type': str(types[26]),
                                'band27_type': str(types[27]),
                                'band28_type': str(types[28]),
                                'band29_type': str(types[29])}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)
        input_gain = self.config.getfloat(section, 'input_gain', fallback=0)
        output_gain = self.config.getfloat(section, 'output_gain', fallback=0)

        # it would make more sense to call these keys bandn_g but this would
        # break people presets...
        self.band0_g = self.config.getfloat(section, 'band0', fallback=0)
        self.band1_g = self.config.getfloat(section, 'band1', fallback=0)
        self.band2_g = self.config.getfloat(section, 'band2', fallback=0)
        self.band3_g = self.config.getfloat(section, 'band3', fallback=0)
        self.band4_g = self.config.getfloat(section, 'band4', fallback=0)
        self.band5_g = self.config.getfloat(section, 'band5', fallback=0)
        self.band6_g = self.config.getfloat(section, 'band6', fallback=0)
        self.band7_g = self.config.getfloat(section, 'band7', fallback=0)
        self.band8_g = self.config.getfloat(section, 'band8', fallback=0)
        self.band9_g = self.config.getfloat(section, 'band9', fallback=0)
        self.band10_g = self.config.getfloat(section, 'band10', fallback=0)
        self.band11_g = self.config.getfloat(section, 'band11', fallback=0)
        self.band12_g = self.config.getfloat(section, 'band12', fallback=0)
        self.band13_g = self.config.getfloat(section, 'band13', fallback=0)
        self.band14_g = self.config.getfloat(section, 'band14', fallback=0)
        self.band15_g = self.config.getfloat(section, 'band15', fallback=0)
        self.band16_g = self.config.getfloat(section, 'band16', fallback=0)
        self.band17_g = self.config.getfloat(section, 'band17', fallback=0)
        self.band18_g = self.config.getfloat(section, 'band18', fallback=0)
        self.band19_g = self.config.getfloat(section, 'band19', fallback=0)
        self.band20_g = self.config.getfloat(section, 'band20', fallback=0)
        self.band21_g = self.config.getfloat(section, 'band21', fallback=0)
        self.band22_g = self.config.getfloat(section, 'band22', fallback=0)
        self.band23_g = self.config.getfloat(section, 'band23', fallback=0)
        self.band24_g = self.config.getfloat(section, 'band24', fallback=0)
        self.band25_g = self.config.getfloat(section, 'band25', fallback=0)
        self.band26_g = self.config.getfloat(section, 'band26', fallback=0)
        self.band27_g = self.config.getfloat(section, 'band27', fallback=0)
        self.band28_g = self.config.getfloat(section, 'band28', fallback=0)
        self.band29_g = self.config.getfloat(section, 'band29', fallback=0)

        self.band0_f = self.config.getfloat(section, 'band0_freq',
                                            fallback=22.59)
        self.band1_f = self.config.getfloat(section, 'band1_freq',
                                            fallback=28.44)
        self.band2_f = self.config.getfloat(section, 'band2_freq',
                                            fallback=35.8)
        self.band3_f = self.config.getfloat(section, 'band3_freq',
                                            fallback=45.07)
        self.band4_f = self.config.getfloat(section, 'band4_freq',
                                            fallback=56.74)
        self.band5_f = self.config.getfloat(section, 'band5_freq',
                                            fallback=71.43)
        self.band6_f = self.config.getfloat(section, 'band6_freq',
                                            fallback=89.93)
        self.band7_f = self.config.getfloat(section, 'band7_freq',
                                            fallback=113.21)
        self.band8_f = self.config.getfloat(section, 'band8_freq',
                                            fallback=142.53)
        self.band9_f = self.config.getfloat(section, 'band9_freq',
                                            fallback=179.43)
        self.band10_f = self.config.getfloat(section, 'band10_freq',
                                             fallback=225.89)
        self.band11_f = self.config.getfloat(section, 'band11_freq',
                                             fallback=284.38)
        self.band12_f = self.config.getfloat(section, 'band12_freq',
                                             fallback=358.02)
        self.band13_f = self.config.getfloat(section, 'band13_freq',
                                             fallback=450.72)
        self.band14_f = self.config.getfloat(section, 'band14_freq',
                                             fallback=567.42)
        self.band15_f = self.config.getfloat(section, 'band15_freq',
                                             fallback=714.34)
        self.band16_f = self.config.getfloat(section, 'band16_freq',
                                             fallback=899.29)
        self.band17_f = self.config.getfloat(section, 'band17_freq',
                                             fallback=1132.15)
        self.band18_f = self.config.getfloat(section, 'band18_freq',
                                             fallback=1425.29)
        self.band19_f = self.config.getfloat(section, 'band19_freq',
                                             fallback=1794.33)
        self.band20_f = self.config.getfloat(section, 'band20_freq',
                                             fallback=2258.93)
        self.band21_f = self.config.getfloat(section, 'band21_freq',
                                             fallback=2843.82)
        self.band22_f = self.config.getfloat(section, 'band22_freq',
                                             fallback=3580.16)
        self.band23_f = self.config.getfloat(section, 'band23_freq',
                                             fallback=4507.15)
        self.band24_f = self.config.getfloat(section, 'band24_freq',
                                             fallback=5674.16)
        self.band25_f = self.config.getfloat(section, 'band25_freq',
                                             fallback=7143.35)
        self.band26_f = self.config.getfloat(section, 'band26_freq',
                                             fallback=8992.94)
        self.band27_f = self.config.getfloat(section, 'band27_freq',
                                             fallback=11321.45)
        self.band28_f = self.config.getfloat(section, 'band28_freq',
                                             fallback=14252.86)
        self.band29_f = self.config.getfloat(section, 'band29_freq',
                                             fallback=17943.28)

        self.band0_q = self.config.getfloat(section, 'band0_qfactor',
                                            fallback=2.21)
        self.band1_q = self.config.getfloat(section, 'band1_qfactor',
                                            fallback=2.21)
        self.band2_q = self.config.getfloat(section, 'band2_qfactor',
                                            fallback=2.21)
        self.band3_q = self.config.getfloat(section, 'band3_qfactor',
                                            fallback=2.21)
        self.band4_q = self.config.getfloat(section, 'band4_qfactor',
                                            fallback=2.21)
        self.band5_q = self.config.getfloat(section, 'band5_qfactor',
                                            fallback=2.21)
        self.band6_q = self.config.getfloat(section, 'band6_qfactor',
                                            fallback=2.21)
        self.band7_q = self.config.getfloat(section, 'band7_qfactor',
                                            fallback=2.21)
        self.band8_q = self.config.getfloat(section, 'band8_qfactor',
                                            fallback=2.21)
        self.band9_q = self.config.getfloat(section, 'band9_qfactor',
                                            fallback=2.21)
        self.band10_q = self.config.getfloat(section, 'band10_qfactor',
                                             fallback=2.21)
        self.band11_q = self.config.getfloat(section, 'band11_qfactor',
                                             fallback=2.21)
        self.band12_q = self.config.getfloat(section, 'band12_qfactor',
                                             fallback=2.21)
        self.band13_q = self.config.getfloat(section, 'band13_qfactor',
                                             fallback=2.21)
        self.band14_q = self.config.getfloat(section, 'band14_qfactor',
                                             fallback=2.21)
        self.band15_q = self.config.getfloat(section, 'band15_qfactor',
                                             fallback=2.21)
        self.band16_q = self.config.getfloat(section, 'band16_qfactor',
                                             fallback=2.21)
        self.band17_q = self.config.getfloat(section, 'band17_qfactor',
                                             fallback=2.21)
        self.band18_q = self.config.getfloat(section, 'band18_qfactor',
                                             fallback=2.21)
        self.band19_q = self.config.getfloat(section, 'band19_qfactor',
                                             fallback=2.21)
        self.band20_q = self.config.getfloat(section, 'band20_qfactor',
                                             fallback=2.21)
        self.band21_q = self.config.getfloat(section, 'band21_qfactor',
                                             fallback=2.21)
        self.band22_q = self.config.getfloat(section, 'band22_qfactor',
                                             fallback=2.21)
        self.band23_q = self.config.getfloat(section, 'band23_qfactor',
                                             fallback=2.21)
        self.band24_q = self.config.getfloat(section, 'band24_qfactor',
                                             fallback=2.21)
        self.band25_q = self.config.getfloat(section, 'band25_qfactor',
                                             fallback=2.21)
        self.band26_q = self.config.getfloat(section, 'band26_qfactor',
                                             fallback=2.21)
        self.band27_q = self.config.getfloat(section, 'band27_qfactor',
                                             fallback=2.21)
        self.band28_q = self.config.getfloat(section, 'band28_qfactor',
                                             fallback=2.21)
        self.band29_q = self.config.getfloat(section, 'band29_qfactor',
                                             fallback=2.21)

        self.band0_t = self.config.getfloat(section, 'band0_type', fallback=0)
        self.band1_t = self.config.getfloat(section, 'band1_type', fallback=0)
        self.band2_t = self.config.getfloat(section, 'band2_type', fallback=0)
        self.band3_t = self.config.getfloat(section, 'band3_type', fallback=0)
        self.band4_t = self.config.getfloat(section, 'band4_type', fallback=0)
        self.band5_t = self.config.getfloat(section, 'band5_type', fallback=0)
        self.band6_t = self.config.getfloat(section, 'band6_type', fallback=0)
        self.band7_t = self.config.getfloat(section, 'band7_type', fallback=0)
        self.band8_t = self.config.getfloat(section, 'band8_type', fallback=0)
        self.band9_t = self.config.getfloat(section, 'band9_type', fallback=0)
        self.band10_t = self.config.getfloat(section, 'band10_type',
                                             fallback=0)
        self.band11_t = self.config.getfloat(section, 'band11_type',
                                             fallback=0)
        self.band12_t = self.config.getfloat(section, 'band12_type',
                                             fallback=0)
        self.band13_t = self.config.getfloat(section, 'band13_type',
                                             fallback=0)
        self.band14_t = self.config.getfloat(section, 'band14_type',
                                             fallback=0)
        self.band15_t = self.config.getfloat(section, 'band15_type',
                                             fallback=0)
        self.band16_t = self.config.getfloat(section, 'band16_type',
                                             fallback=0)
        self.band17_t = self.config.getfloat(section, 'band17_type',
                                             fallback=0)
        self.band18_t = self.config.getfloat(section, 'band18_type',
                                             fallback=0)
        self.band19_t = self.config.getfloat(section, 'band19_type',
                                             fallback=0)
        self.band20_t = self.config.getfloat(section, 'band20_type',
                                             fallback=0)
        self.band21_t = self.config.getfloat(section, 'band21_type',
                                             fallback=0)
        self.band22_t = self.config.getfloat(section, 'band22_type',
                                             fallback=0)
        self.band23_t = self.config.getfloat(section, 'band23_type',
                                             fallback=0)
        self.band24_t = self.config.getfloat(section, 'band24_type',
                                             fallback=0)
        self.band25_t = self.config.getfloat(section, 'band25_type',
                                             fallback=0)
        self.band26_t = self.config.getfloat(section, 'band26_type',
                                             fallback=0)
        self.band27_t = self.config.getfloat(section, 'band27_type',
                                             fallback=0)
        self.band28_t = self.config.getfloat(section, 'band28_type',
                                             fallback=0)
        self.band29_t = self.config.getfloat(section, 'band29_type',
                                             fallback=0)

        settings.set_value('state', GLib.Variant('b', enabled))
        settings.set_value('input-gain',
                           GLib.Variant('d', input_gain))
        settings.set_value('output-gain',
                           GLib.Variant('d', output_gain))

        for n in range(30):
            g = GLib.Variant('d', getattr(self, 'band' + str(n) + '_g'))
            f = GLib.Variant('d', getattr(self, 'band' + str(n) + '_f'))
            q = GLib.Variant('d', getattr(self, 'band' + str(n) + '_q'))
            t = GLib.Variant('i', getattr(self, 'band' + str(n) + '_t'))

            settings.set_value('band' + str(n) + '-gain', g)
            settings.set_value('band' + str(n) + '-frequency', f)
            settings.set_value('band' + str(n) + '-quality', q)
            settings.set_value('band' + str(n) + '-type', t)

    def save(self):
        self.add_section(self.settings_sinkinputs, 'apps_equalizer')
        self.add_section(self.settings_sourceoutputs, 'mic_equalizer')

    def load(self):
        self.load_section(self.settings_sinkinputs, 'apps_equalizer')
        self.load_section(self.settings_sourceoutputs, 'mic_equalizer')
