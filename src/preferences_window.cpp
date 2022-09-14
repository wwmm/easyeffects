/*
 *  Copyright © 2017-2023 Wellington Wallace
 *
 *  This file is part of Easy Effects.
 *
 *  Easy Effects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Easy Effects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "preferences_window.hpp"
#include "config.h"

namespace ui::preferences::window {

struct _PreferencesWindow {
  AdwPreferencesWindow parent_instance;

  ui::preferences::general::PreferencesGeneral* page_general;

  ui::preferences::spectrum::PreferencesSpectrum* page_spectrum;
};

G_DEFINE_TYPE(PreferencesWindow, preferences_window, ADW_TYPE_PREFERENCES_WINDOW)

void dispose(GObject* object) {
  auto* self = EE_PREFERENCES_WINDOW(object);

  gtk_window_set_icon_name(GTK_WINDOW(self),
                           IS_DEVEL_BUILD ? std::string(tags::app::id).append(".Devel").c_str() : tags::app::id);

  adw_preferences_window_remove(ADW_PREFERENCES_WINDOW(self), ADW_PREFERENCES_PAGE(self->page_general));
  adw_preferences_window_remove(ADW_PREFERENCES_WINDOW(self), ADW_PREFERENCES_PAGE(self->page_spectrum));

  util::debug("disposed");

  G_OBJECT_CLASS(preferences_window_parent_class)->dispose(object);
}

void preferences_window_class_init(PreferencesWindowClass* klass) {
  auto* object_class = G_OBJECT_CLASS(klass);
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  object_class->dispose = dispose;

  gtk_widget_class_set_template_from_resource(widget_class, tags::resources::preferences_window_ui);
}

void preferences_window_init(PreferencesWindow* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->page_general = ui::preferences::general::create();
  self->page_spectrum = ui::preferences::spectrum::create();

  adw_preferences_window_add(ADW_PREFERENCES_WINDOW(self), ADW_PREFERENCES_PAGE(self->page_general));
  adw_preferences_window_add(ADW_PREFERENCES_WINDOW(self), ADW_PREFERENCES_PAGE(self->page_spectrum));
}

auto create() -> PreferencesWindow* {
  return static_cast<PreferencesWindow*>(g_object_new(EE_TYPE_PREFERENCES_WINDOW, nullptr));
}

}  // namespace ui::preferences::window
