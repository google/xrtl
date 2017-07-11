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

#include <spirv-tools/optimizer.hpp>

#include <utility>

#include "xrtl/base/logging.h"
#include "xrtl/base/tracing.h"

namespace xrtl {
namespace gfx {
namespace spirv {

SpirVOptimizer::SpirVOptimizer(Options options) : options_(std::move(options)) {
  // TODO(benvanik): evaluate the environment options. We could do 1.1.
  optimizer_.reset(new spvtools::Optimizer(SPV_ENV_UNIVERSAL_1_0));
  optimizer_->SetMessageConsumer(
      [](spv_message_level_t level, const char* source,
         const spv_position_t& position, const char* message) {
        // TODO(benvanik): better logging.
        LOG(INFO) << message;
      });

  if (options_.strip_debug_info) {
    optimizer_->RegisterPass(spvtools::CreateStripDebugInfoPass());
  }

  if (options_.freeze_specialization_values) {
    optimizer_->RegisterPass(spvtools::CreateFreezeSpecConstantValuePass());
  }

  if (options_.aggressive) {
    optimizer_->RegisterPass(spvtools::CreateInlinePass());
    optimizer_->RegisterPass(spvtools::CreateLocalAccessChainConvertPass());
    optimizer_->RegisterPass(spvtools::CreateInsertExtractElimPass());
    optimizer_->RegisterPass(
        spvtools::CreateLocalSingleBlockLoadStoreElimPass());
    optimizer_->RegisterPass(spvtools::CreateLocalSingleStoreElimPass());
    optimizer_->RegisterPass(spvtools::CreateBlockMergePass());
    optimizer_->RegisterPass(spvtools::CreateEliminateDeadConstantPass());
    optimizer_->RegisterPass(
        spvtools::CreateFoldSpecConstantOpAndCompositePass());
    optimizer_->RegisterPass(spvtools::CreateUnifyConstantPass());
  }

  if (options_.strip_debug_info) {
    optimizer_->RegisterPass(spvtools::CreateFlattenDecorationPass());
  }

  if (options_.remap_ids) {
    optimizer_->RegisterPass(spvtools::CreateCompactIdsPass());
  }
}

SpirVOptimizer::~SpirVOptimizer() = default;

bool SpirVOptimizer::Optimize(const std::vector<uint32_t>& source_spirv,
                              std::vector<uint32_t>* target_spirv) {
  target_spirv->clear();
  if (!optimizer_->Run(source_spirv.data(), source_spirv.size(),
                       target_spirv)) {
    // NOTE: the target may have been partially populated by the optimizer.
    target_spirv->clear();
    return false;
  }
  return true;
}

}  // namespace spirv
}  // namespace gfx
}  // namespace xrtl
