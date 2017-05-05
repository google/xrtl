// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "xrtl/gfx/command_buffer.h"

namespace xrtl {
namespace gfx {

CommandBuffer::CommandBuffer() = default;

CommandBuffer::~CommandBuffer() { ReleaseDependencies(); }

void CommandBuffer::AttachDependency(void (*release_fn)(void* value_ptr),
                                     void* value_ptr) {
  dependencies_.push_back({release_fn, value_ptr});
}

void CommandBuffer::AttachDependencies(void (*release_fn)(void* value_ptr),
                                       void* const* value_ptrs,
                                       size_t value_count) {
  dependencies_.reserve(dependencies_.size() + value_count);
  for (size_t i = 0; i < value_count; ++i) {
    dependencies_.push_back({release_fn, value_ptrs[i]});
  }
}

void CommandBuffer::ReleaseDependencies() {
  for (auto& dependency : dependencies_) {
    dependency.release_fn(dependency.value_ptr);
  }
  dependencies_.resize(0);
}

}  // namespace gfx
}  // namespace xrtl
