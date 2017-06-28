# -*- coding: utf-8 -*-

import gi
import numpy as np
gi.require_version('PangoCairo', '1.0')
from gi.repository import GLib, Pango, PangoCairo


class Spectrum():

    def __init__(self, app):
        self.app = app
        self.builder = app.builder
        self.sie = app.sie
        self.settings = app.settings

        self.show_spectrum = True
        self.mouse_inside = False
        self.mouse_freq = 0
        self.mouse_intensity = 0

        self.font_description = Pango.FontDescription('Monospace')

        self.handlers = {
            'on_spectrum_draw': self.on_spectrum_draw,
            'on_spectrum_enter_notify_event':
                self.on_spectrum_enter_notify_event,
            'on_spectrum_leave_notify_event':
                self.on_spectrum_leave_notify_event,
            'on_spectrum_motion_notify_event':
                self.on_spectrum_motion_notify_event
        }

        self.spectrum_box = self.builder.get_object('spectrum_box')
        self.drawing_area = self.builder.get_object('spectrum')

        self.spectrum_magnitudes = np.array([])

    def init(self):
        pass

    def show(self):
        self.spectrum_box.show_all()
        self.show_spectrum = True

    def hide(self):
        self.spectrum_box.hide()
        self.show_spectrum = False

    def on_spectrum_draw(self, drawing_area, ctx):
        ctx.paint()

        n_bars = self.spectrum_magnitudes.size

        if n_bars > 0:
            width = drawing_area.get_allocation().width
            height = drawing_area.get_allocation().height
            style = drawing_area.get_style_context()

            bar_height = self.spectrum_magnitudes * height
            x = np.linspace(0, width, n_bars)
            y = height - bar_height
            dx = width / n_bars

            rectangle = ctx.rectangle
            rectangles = np.vectorize(rectangle)
            rectangles(x, y, dx, bar_height)

            color = style.lookup_color('theme_selected_bg_color')[1]
            ctx.set_source_rgba(color.red, color.green, color.blue, 1.0)
            ctx.set_line_width(1.1)
            ctx.stroke()

            if self.mouse_inside:
                label = str(self.mouse_freq) + ' Hz, '
                label += str(self.mouse_intensity) + ' dB'

                layout = PangoCairo.create_layout(ctx)
                layout.set_text(label, -1)
                layout.set_font_description(self.font_description)

                text_width, text_height = layout.get_pixel_size()

                ctx.move_to(width - text_width, 0)

                PangoCairo.show_layout(ctx, layout)

    def on_new_spectrum(self, obj, magnitudes):
        if self.show_spectrum:
            self.spectrum_magnitudes = magnitudes

            self.drawing_area.queue_draw()

    def on_spectrum_enter_notify_event(self, drawing_area, event_crossing):
        self.mouse_inside = True

    def on_spectrum_leave_notify_event(self, drawing_area, event_crossing):
        self.mouse_inside = False

    def on_spectrum_motion_notify_event(self, drawing_area, event_motion):
        width = drawing_area.get_allocation().width
        height = drawing_area.get_allocation().height

        # frequency axis is logarithmic
        # 20 Hz = 10^(1.3), 20000 Hz = 10^(4.3)

        self.mouse_freq = round(10**(1.3 + event_motion.x * 3.0 / width), 1)

        # intensity scale is in decibel
        # minimum intensity is -120 dB and maximum is 0 dB

        self.mouse_intensity = round(- event_motion.y * 120 / height, 1)

    def reset(self):
        self.settings.reset('show-spectrum')
        self.settings.reset('spectrum-n-points')

        self.init()
