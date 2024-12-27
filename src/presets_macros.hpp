#pragma once

// NOLINTBEGIN(bugprone-macro-parentheses,cppcoreguidelines-macro-usage)
#define UPDATE_PROPERTY(key, property) \
  { settings->set##property(json.at(section).at(instance_name).value(key, settings->default##property##Value())); }
// NOLINTEND(bugprone-macro-parentheses,cppcoreguidelines-macro-usage)