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

#include "xrtl/gfx/spirv/spirv_optimizer.h"

namespace xrtl {
namespace gfx {
namespace spirv {

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  SpirVOptimizer::Options options;
  options.strip_debug_info = true;
  options.freeze_specialization_values = true;
  options.inline_functions = true;
  options.remap_ids = true;
  SpirVOptimizer spirv_optimizer(options);
  std::vector<uint32_t> source_spirv;
  source_spirv.resize(size / 4);
  std::memcpy(source_spirv.data(), data, (size / 4) * 4);
  std::vector<uint32_t> target_spirv;
  spirv_optimizer.Optimize(source_spirv, &target_spirv);
  return 0;
}

}  // namespace spirv
}  // namespace gfx
}  // namespace xrtl
