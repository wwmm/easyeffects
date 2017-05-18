# -*- coding: utf-8 -*-


class Spectrum():

    def __init__(self, app):
        self.app = app
        self.builder = app.builder
        self.gst = app.gst
        self.settings = app.settings

        self.handlers = {
            'on_spectrum_draw': self.on_spectrum_draw
        }

        self.gst.connect('new_spectrum', self.on_new_spectrum)

        self.drawing_area = self.builder.get_object('spectrum')

        self.spectrum_magnitudes = []

    def on_spectrum_draw(self, drawing_area, ctx):
        ctx.paint()

        if self.spectrum_magnitudes:
            width = drawing_area.get_allocation().width
            height = drawing_area.get_allocation().height
            n_bars = len(self.spectrum_magnitudes)
            style = drawing_area.get_style_context()

            dx = width / n_bars

            for n in range(n_bars):
                mag = self.spectrum_magnitudes[n]

                if mag > 0:
                    bar_height = self.spectrum_magnitudes[n] * 1.5

                    ctx.rectangle(n * dx, height - bar_height, dx, bar_height)

            color = style.lookup_color('theme_selected_bg_color')[1]
            ctx.set_source_rgba(color.red, color.green, color.blue, 1.0)
            ctx.stroke()

    def on_new_spectrum(self, obj, magnitudes):
        self.spectrum_magnitudes = magnitudes

        self.drawing_area.queue_draw()
