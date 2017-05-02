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

#ifndef XRTL_GFX_COMMAND_FENCE_H_
#define XRTL_GFX_COMMAND_FENCE_H_

#include "xrtl/base/ref_ptr.h"

namespace xrtl {
namespace gfx {

// A fence that orders commands within a command buffer.
// These may be signaled once per object. Attempting to signal an already-
// signaled fence may produce undefined results.
//
// These are device-side only and are only used to order the way commands
// are processed within command buffers. They cannot be used for synchronization
// with the CPU (use Context::WaitUntilQueuesIdle for that).
class CommandFence : public RefObject<CommandFence> {
 public:
  virtual ~CommandFence() = default;

 protected:
  CommandFence() = default;
};

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_COMMAND_FENCE_H_
