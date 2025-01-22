#pragma once

// NOLINTBEGIN(bugprone-macro-parentheses,cppcoreguidelines-macro-usage)
#define UPDATE_PROPERTY(key, property)                                                                            \
  {                                                                                                               \
    settings->set##property(json.at(section).at(instance_name).value(key, settings->default##property##Value())); \
  }

#define UPDATE_PROPERTY_INSIDE_SUBSECTION(json_section, key, property)                                         \
  {                                                                                                            \
    settings->set##property(                                                                                   \
        json.at(section).at(instance_name).at(json_section).value(key, settings->default##property##Value())); \
  }

#define UPDATE_ENUM_LIKE_PROPERTY(key, property)                                                                   \
  {                                                                                                                \
    if (const auto idx =                                                                                           \
            settings->default##property##LabelsValue().indexOf(json.at(section).at(instance_name).value(key, "")); \
        idx != -1) {                                                                                               \
      settings->set##property(idx);                                                                                \
    }                                                                                                              \
  }

#define UPDATE_ENUM_LIKE_PROPERTY_INSIDE_SUBSECTION(json_section, key, property) \
  {                                                                              \
    if (const auto idx = settings->default##property##LabelsValue().indexOf(     \
            json.at(section).at(instance_name).at(json_section).value(key, "")); \
        idx != -1) {                                                             \
      settings->set##property(idx);                                              \
    }                                                                            \
  }
// NOLINTEND(bugprone-macro-parentheses,cppcoreguidelines-macro-usage)