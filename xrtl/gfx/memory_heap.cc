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

#include "xrtl/gfx/memory_heap.h"

namespace xrtl {
namespace gfx {

std::ostream& operator<<(std::ostream& stream,
                         const MemoryHeap::AllocationResult& value) {
  switch (value) {
    XRTL_UNREACHABLE_DEFAULT();
    case MemoryHeap::AllocationResult::kSuccess:
      return stream << "AllocationResult::kSuccess";
    case MemoryHeap::AllocationResult::kInvalidArguments:
      return stream << "AllocationResult::kInvalidArguments";
    case MemoryHeap::AllocationResult::kUnsupported:
      return stream << "AllocationResult::kUnsupported";
    case MemoryHeap::AllocationResult::kLimitsExceeded:
      return stream << "AllocationResult::kLimitsExceeded";
    case MemoryHeap::AllocationResult::kOutOfMemory:
      return stream << "AllocationResult::kOutOfMemory";
  }
}

}  // namespace gfx
}  // namespace xrtl
