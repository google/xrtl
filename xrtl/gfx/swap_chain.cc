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

#include "xrtl/gfx/swap_chain.h"

namespace xrtl {
namespace gfx {

std::ostream& operator<<(std::ostream& stream,
                         const SwapChain::ResizeResult& value) {
  switch (value) {
    XRTL_UNREACHABLE_DEFAULT();
    case SwapChain::ResizeResult::kSuccess:
      return stream << "ResizeResult::kSuccess";
    case SwapChain::ResizeResult::kOutOfMemory:
      return stream << "ResizeResult::kOutOfMemory";
    case SwapChain::ResizeResult::kDeviceLost:
      return stream << "ResizeResult::kDeviceLost";
  }
}

std::ostream& operator<<(std::ostream& stream,
                         const SwapChain::AcquireResult& value) {
  switch (value) {
    XRTL_UNREACHABLE_DEFAULT();
    case SwapChain::AcquireResult::kSuccess:
      return stream << "AcquireResult::kSuccess";
    case SwapChain::AcquireResult::kResizeRequired:
      return stream << "AcquireResult::kResizeRequired";
    case SwapChain::AcquireResult::kTimeout:
      return stream << "AcquireResult::kTimeout";
    case SwapChain::AcquireResult::kDiscardPending:
      return stream << "AcquireResult::kDiscardPending";
    case SwapChain::AcquireResult::kDeviceLost:
      return stream << "AcquireResult::kDeviceLost";
  }
}

std::ostream& operator<<(std::ostream& stream,
                         const SwapChain::PresentResult& value) {
  switch (value) {
    XRTL_UNREACHABLE_DEFAULT();
    case SwapChain::PresentResult::kSuccess:
      return stream << "PresentResult::kSuccess";
    case SwapChain::PresentResult::kResizeRequired:
      return stream << "PresentResult::kResizeRequired";
    case SwapChain::PresentResult::kDiscardPending:
      return stream << "PresentResult::kDiscardPending";
    case SwapChain::PresentResult::kDeviceLost:
      return stream << "PresentResult::kDeviceLost";
  }
}

}  // namespace gfx
}  // namespace xrtl
