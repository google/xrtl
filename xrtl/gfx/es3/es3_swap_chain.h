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

#ifndef XRTL_GFX_ES3_ES3_SWAP_CHAIN_H_
#define XRTL_GFX_ES3_ES3_SWAP_CHAIN_H_

#include "xrtl/gfx/es3/es3_common.h"
#include "xrtl/gfx/es3/es3_platform_context.h"
#include "xrtl/gfx/memory_pool.h"
#include "xrtl/gfx/swap_chain.h"
#include "xrtl/ui/control.h"

namespace xrtl {
namespace gfx {
namespace es3 {

class ES3SwapChain : public SwapChain {
 public:
  static ref_ptr<ES3SwapChain> Create(
      ref_ptr<ES3PlatformContext> shared_platform_context,
      ref_ptr<MemoryPool> memory_pool, ref_ptr<ui::Control> control,
      PresentMode present_mode, int image_count,
      ArrayView<PixelFormat> pixel_formats);

  ~ES3SwapChain() override = default;

 protected:
  ES3SwapChain(PresentMode present_mode, int image_count,
               ArrayView<PixelFormat> pixel_formats)
      : SwapChain(present_mode, image_count) {}
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_SWAP_CHAIN_H_
