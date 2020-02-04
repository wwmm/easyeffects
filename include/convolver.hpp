#ifndef CONVOLVER_HPP
#define CONVOLVER_HPP

#include "plugin_base.hpp"

class Convolver : public PluginBase {
 public:
  Convolver(const std::string& tag, const std::string& schema);
  Convolver(const Convolver&) = delete;
  auto operator=(const Convolver&) -> Convolver& = delete;
  Convolver(const Convolver&&) = delete;
  auto operator=(const Convolver &&) -> Convolver& = delete;
  ~Convolver() override;

  GstElement* convolver = nullptr;

 private:
  void bind_to_gsettings();
};

#endif
