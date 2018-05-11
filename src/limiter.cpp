#include "limiter.hpp"

Limiter::Limiter(std::string schema)
    : settings(g_settings_new(schema.c_str())) {}

Limiter::~Limiter() {}
