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

#include "xrtl/gfx/es3/es3_pipeline.h"

namespace xrtl {
namespace gfx {
namespace es3 {

ES3ComputePipeline::ES3ComputePipeline(ES3ObjectLifetimeQueue* queue,
                                       ref_ptr<PipelineLayout> pipeline_layout,
                                       ref_ptr<ShaderModule> shader_module,
                                       absl::string_view entry_point,
                                       ref_ptr<ES3Program> program)
    : ComputePipeline(std::move(pipeline_layout), std::move(shader_module),
                      entry_point),
      queue_(queue),
      program_(std::move(program)) {}

ES3ComputePipeline::~ES3ComputePipeline() = default;

void ES3ComputePipeline::PrepareAllocation() {
  queue_->EnqueueObjectAllocation(this);
}

void ES3ComputePipeline::Release() { queue_->EnqueueObjectDeallocation(this); }

bool ES3ComputePipeline::AllocateOnQueue() { return program_->Link(); }

void ES3ComputePipeline::DeallocateOnQueue() { program_.reset(); }

ES3RenderPipeline::ES3RenderPipeline(ES3ObjectLifetimeQueue* queue,
                                     ref_ptr<PipelineLayout> pipeline_layout,
                                     ref_ptr<RenderPass> render_pass,
                                     int render_subpass,
                                     RenderState render_state,
                                     RenderPipeline::ShaderStages shader_stages,
                                     ref_ptr<ES3Program> program)
    : RenderPipeline(std::move(pipeline_layout), std::move(render_pass),
                     render_subpass, std::move(render_state),
                     std::move(shader_stages)),
      queue_(queue),
      program_(std::move(program)) {}

ES3RenderPipeline::~ES3RenderPipeline() = default;

void ES3RenderPipeline::PrepareAllocation() {
  queue_->EnqueueObjectAllocation(this);
}

void ES3RenderPipeline::Release() { queue_->EnqueueObjectDeallocation(this); }

bool ES3RenderPipeline::AllocateOnQueue() { return program_->Link(); }

void ES3RenderPipeline::DeallocateOnQueue() { program_.reset(); }

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
