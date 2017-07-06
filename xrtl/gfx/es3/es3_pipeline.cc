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

ES3ComputePipeline::ES3ComputePipeline(
    ref_ptr<ES3PlatformContext> platform_context,
    ref_ptr<PipelineLayout> pipeline_layout,
    ref_ptr<ShaderModule> shader_module, std::string entry_point,
    ref_ptr<ES3Program> program)
    : ComputePipeline(std::move(pipeline_layout), std::move(shader_module),
                      std::move(entry_point)),
      platform_context_(std::move(platform_context)),
      program_(std::move(program)) {}

ES3ComputePipeline::~ES3ComputePipeline() = default;

ES3RenderPipeline::ES3RenderPipeline(
    ref_ptr<ES3PlatformContext> platform_context,
    ref_ptr<PipelineLayout> pipeline_layout, ref_ptr<RenderPass> render_pass,
    int render_subpass, RenderState render_state,
    RenderPipeline::ShaderStages shader_stages, ref_ptr<ES3Program> program)
    : RenderPipeline(std::move(pipeline_layout), std::move(render_pass),
                     render_subpass, render_state, std::move(shader_stages)),
      platform_context_(std::move(platform_context)),
      program_(std::move(program)) {}

ES3RenderPipeline::~ES3RenderPipeline() = default;

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
