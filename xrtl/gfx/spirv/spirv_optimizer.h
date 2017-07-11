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

#ifndef XRTL_GFX_SPIRV_SPIRV_OPTIMIZER_H_
#define XRTL_GFX_SPIRV_SPIRV_OPTIMIZER_H_

#include <memory>
#include <vector>

namespace spvtools {
class Optimizer;
}  // namespace spvtools

namespace xrtl {
namespace gfx {
namespace spirv {

// SPIR-V optimizer interface.
// Performs some basic optimizations on SPIR-V bytecode to reduce at-rest size
// and perform ahead-of-time specialization and inlining.
//
// The current set of optimizations is currently pretty weak and you should not
// expect fantastic results from this.
//
// Optimizer instances is not thread safe but multiple threads may concurrently
// use unique instances.
class SpirVOptimizer {
 public:
  struct Options {
    bool strip_debug_info = false;
    bool freeze_specialization_values = false;
    bool aggressive = false;
    bool remap_ids = false;
  };

  explicit SpirVOptimizer(Options options);
  ~SpirVOptimizer();

  // Runs an optimization pass on the input SPIR-V bytecode.
  // Returns true if optimization succeeded and target_spirv contains the
  // optimized bytecode.
  bool Optimize(const std::vector<uint32_t>& source_spirv,
                std::vector<uint32_t>* target_spirv);

 private:
  Options options_;
  std::unique_ptr<spvtools::Optimizer> optimizer_;
};

}  // namespace spirv
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_SPIRV_SPIRV_OPTIMIZER_H_
