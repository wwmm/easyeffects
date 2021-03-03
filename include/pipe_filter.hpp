#ifndef PIPE_FILTER_HPP
#define PIPE_FILTER_HPP

#include <pipewire/filter.h>
#include <pipewire/pipewire.h>
#include "util.hpp"

namespace pf {

struct data;

struct port {
  struct data* data;
};

struct data {
  struct port *in_left, *in_right, *out_left, *out_right;
};

class PipeFilter {
 public:
  PipeFilter(pw_core* core);
  PipeFilter(const PipeFilter&) = delete;
  auto operator=(const PipeFilter&) -> PipeFilter& = delete;
  PipeFilter(const PipeFilter&&) = delete;
  auto operator=(const PipeFilter&&) -> PipeFilter& = delete;
  ~PipeFilter();

  std::string log_tag = "pipe_filter: ";

  pw_filter* filter = nullptr;

  spa_hook listener{};

  data pf_data{};
};

};  // namespace pf

#endif