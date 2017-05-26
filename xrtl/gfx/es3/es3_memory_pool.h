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

#ifndef XRTL_GFX_ES3_ES3_MEMORY_POOL_H_
#define XRTL_GFX_ES3_ES3_MEMORY_POOL_H_

#include "xrtl/gfx/es3/es3_common.h"
#include "xrtl/gfx/es3/es3_platform_context.h"
#include "xrtl/gfx/memory_pool.h"

namespace xrtl {
namespace gfx {
namespace es3 {

class ES3MemoryPool : public MemoryPool {
 public:
  ES3MemoryPool(ref_ptr<ES3PlatformContext> platform_context,
                MemoryType memory_type_mask, size_t chunk_size);
  ~ES3MemoryPool() override;

  void Reclaim() override;

  AllocationResult AllocateBuffer(size_t size, Buffer::Usage usage_mask,
                                  ref_ptr<Buffer>* out_buffer) override;

  AllocationResult AllocateImage(Image::CreateParams create_params,
                                 ref_ptr<Image>* out_image) override;

 private:
  ref_ptr<ES3PlatformContext> platform_context_;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_MEMORY_POOL_H_
