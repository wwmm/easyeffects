/**
 * Copyright © 2017-2026 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <QString>
#include "effects_base.hpp"
#include "pw_manager.hpp"
#include "pw_objects.hpp"

class StreamOutputEffects : public EffectsBase {
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON
  QML_UNCREATABLE("Use the c++ instance")

 public:
  StreamOutputEffects(pw::Manager* pipe_manager);
  StreamOutputEffects(const StreamOutputEffects&) = delete;
  auto operator=(const StreamOutputEffects&) -> StreamOutputEffects& = delete;
  StreamOutputEffects(const StreamOutputEffects&&) = delete;
  auto operator=(const StreamOutputEffects&&) -> StreamOutputEffects& = delete;
  ~StreamOutputEffects() override;

  inline static StreamOutputEffects* singletonInstance = nullptr;

  // Singleton provider for QML
  static StreamOutputEffects* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    Q_UNUSED(jsEngine)

    // The instance has to exist before it is used. We cannot replace it.
    Q_ASSERT(singletonInstance);

    // The engine has to have the same thread affinity as the singleton.

    Q_ASSERT(qmlEngine->thread() == singletonInstance->thread());

    // Explicitly specify C++ ownership so that the engine doesn't delete the instance.

    QJSEngine::setObjectOwnership(singletonInstance, QJSEngine::CppOwnership);

    return singletonInstance;
  }

  void set_bypass(const bool& state);

 private:
  bool bypass = false;

  void connect_filters(const bool& bypass = false);

  void disconnect_filters();

  auto apps_want_to_play() -> bool;

  void on_link_changed(pw::LinkInfo link_info);

  void on_link_removed();
};
