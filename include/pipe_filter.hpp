#ifndef PIPE_FILTER_HPP
#define PIPE_FILTER_HPP

#include <pipewire/filter.h>
#include <pipewire/pipewire.h>
#include <iostream>

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
};

#endif