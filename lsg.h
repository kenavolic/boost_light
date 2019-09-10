// Copyright 2019 Ken Avolic <kenavolic@none.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <functional>
#include <vector>

///
/// Light signal (lsg) is a simple template
/// to build a signal slot mechanism customized for your
/// needs.
///

namespace lsg {
template <typename T> struct signal {
  template <typename C> void connect(C &&u) {
    callbacks.push_back(std::forward<C>(u));
  }

  template <typename... Args> void operator()(Args &&... args) {
    for (auto &c : callbacks)
      c(std::forward<Args>(args)...);
  }

private:
  std::vector<std::function<T>> callbacks;
};
} // namespace lsg