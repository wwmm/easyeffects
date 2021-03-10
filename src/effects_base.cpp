#include "effects_base.hpp"

EffectsBase::EffectsBase(std::string tag, const std::string& schema, PipeManager* pipe_manager)
    : log_tag(std::move(tag)),
      settings(Gio::Settings::create(schema)),
      global_settings(Gio::Settings::create("com.github.wwmm.pulseeffects")),
      pm(pipe_manager) {}

EffectsBase::~EffectsBase() {}