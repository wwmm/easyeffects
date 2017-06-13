# -*- coding: utf-8 -*-

from gi.repository import GLib


class Spectrum():

    def __init__(self, app):
        self.app = app
        self.builder = app.builder
        self.gst = app.gst
        self.settings = app.settings

        self.handlers = {
            'on_show_spectrum_state_set': self.on_show_spectrum_state_set,
            'on_spectrum_draw': self.on_spectrum_draw
        }

        self.gst.connect('new_spectrum', self.on_new_spectrum)

        self.spectrum_box = self.builder.get_object('spectrum_box')
        self.drawing_area = self.builder.get_object('spectrum')

        self.spectrum_magnitudes = []

    def init(self):
        show_spectrum_switch = self.builder.get_object('show_spectrum')
        show_spectrum = self.settings.get_value('show-spectrum').unpack()

        show_spectrum_switch.set_active(show_spectrum)

        # we need this when the saved value is equal to the widget default
        # value
        if show_spectrum:
            self.spectrum_box.show_all()
        else:
            self.spectrum_box.hide()

    def on_show_spectrum_state_set(self, obj, state):
        if state:
            self.spectrum_box.show_all()
        else:
            self.spectrum_box.hide()

        out = GLib.Variant('b', state)
        self.settings.set_value('show-spectrum', out)

    def on_spectrum_draw(self, drawing_area, ctx):
        ctx.paint()

        if self.spectrum_magnitudes:
            width = drawing_area.get_allocation().width
            height = drawing_area.get_allocation().height
            n_bars = len(self.spectrum_magnitudes)
            style = drawing_area.get_style_context()

            dx = width / n_bars

            bar_height = [mag * height for mag in self.spectrum_magnitudes]
            x = [n * dx for n in range(n_bars)]
            y = [height - bar_h for bar_h in bar_height]

            for n in range(n_bars):
                if bar_height[n] > 0:
                    ctx.rectangle(x[n], y[n], dx, bar_height[n])

            color = style.lookup_color('theme_selected_bg_color')[1]
            ctx.set_source_rgba(color.red, color.green, color.blue, 0.75)
            ctx.stroke()

    def on_new_spectrum(self, obj, magnitudes):
        self.spectrum_magnitudes = magnitudes

        self.drawing_area.queue_draw()
