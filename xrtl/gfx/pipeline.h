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

#ifndef XRTL_GFX_PIPELINE_H_
#define XRTL_GFX_PIPELINE_H_

#include <string>
#include <utility>

#include "xrtl/base/ref_ptr.h"
#include "xrtl/gfx/pipeline_layout.h"
#include "xrtl/gfx/render_state.h"
#include "xrtl/gfx/shader_module.h"

namespace xrtl {
namespace gfx {

// Base shader pipeline.
class Pipeline : public RefObject<Pipeline> {
 public:
  virtual ~Pipeline() = default;

  // Layout of the pipeline denoting what other pipelines it is compatible with.
  ref_ptr<PipelineLayout> pipeline_layout() const { return pipeline_layout_; }

 protected:
  explicit Pipeline(ref_ptr<PipelineLayout> pipeline_layout)
      : pipeline_layout_(pipeline_layout) {}

  ref_ptr<PipelineLayout> pipeline_layout_;
};

// A pipeline used for compute operations.
class ComputePipeline : public Pipeline {
 public:
  ~ComputePipeline() override = default;

  // Source shader module.
  ref_ptr<ShaderModule> shader_module() const { return shader_module_; }
  // Entry point name within the shader module.
  const std::string& entry_point() const { return entry_point_; }

 protected:
  ComputePipeline(ref_ptr<PipelineLayout> pipeline_layout,
                  ref_ptr<ShaderModule> shader_module, std::string entry_point)
      : Pipeline(std::move(pipeline_layout)),
        shader_module_(std::move(shader_module)),
        entry_point_(std::move(entry_point)) {}

  ref_ptr<ShaderModule> shader_module_;
  std::string entry_point_;
};

// A pipeline used for rendering.
// Each render pipeline is specific to a (render pass, subpass) pair and fully
// describes all static render state to be used while the pipeline is active.
// Some state is dynamically specified on the RenderPassCommandEncoder.
class RenderPipeline : public Pipeline {
 public:
  // All shader stage modules and entry points.
  // If any are omitted the shader stage will not be enabled for the pipeline.
  // Multiple stages may refrence the same shader module so long as they have
  // differing and stage-compatible entry points.
  struct ShaderStages {
    ref_ptr<ShaderModule> vertex_shader_module;
    std::string vertex_entry_point;
    ref_ptr<ShaderModule> tessellation_control_shader_module;
    std::string tessellation_control_entry_point;
    ref_ptr<ShaderModule> tessellation_evaluation_shader_module;
    std::string tessellation_evaluation_entry_point;
    ref_ptr<ShaderModule> geometry_shader_module;
    std::string geometry_entry_point;
    ref_ptr<ShaderModule> fragment_shader_module;
    std::string fragment_entry_point;
  };

  ~RenderPipeline() override = default;

  // Render pass the pipeline is used in.
  ref_ptr<RenderPass> render_pass() const { return render_pass_; }
  // Subpass index within the render pass the pipeline is used in.
  int render_subpass() const { return render_subpass_; }

  // All render state for the pipeline.
  const RenderState& render_state() const { return render_state_; }

  // All shader stages for the pipeline, possibly only partially populated.
  const ShaderStages& shader_stages() const { return shader_stages_; }

 protected:
  RenderPipeline(ref_ptr<PipelineLayout> pipeline_layout,
                 ref_ptr<RenderPass> render_pass, int render_subpass,
                 RenderState render_state,
                 RenderPipeline::ShaderStages shader_stages)
      : Pipeline(std::move(pipeline_layout)),
        render_pass_(std::move(render_pass)),
        render_subpass_(render_subpass),
        render_state_(std::move(render_state)),
        shader_stages_(std::move(shader_stages)) {}

  ref_ptr<RenderPass> render_pass_;
  int render_subpass_ = 0;
  RenderState render_state_;
  ShaderStages shader_stages_;
};

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_PIPELINE_H_
