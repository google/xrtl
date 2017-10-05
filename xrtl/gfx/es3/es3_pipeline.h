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

#ifndef XRTL_GFX_ES3_ES3_PIPELINE_H_
#define XRTL_GFX_ES3_ES3_PIPELINE_H_

#include <string>
#include <utility>

#include "xrtl/gfx/es3/es3_common.h"
#include "xrtl/gfx/es3/es3_platform_context.h"
#include "xrtl/gfx/es3/es3_program.h"
#include "xrtl/gfx/pipeline.h"

namespace xrtl {
namespace gfx {
namespace es3 {

class ES3ComputePipeline : public ComputePipeline {
 public:
  ES3ComputePipeline(ref_ptr<ES3PlatformContext> platform_context,
                     ref_ptr<PipelineLayout> pipeline_layout,
                     ref_ptr<ShaderModule> shader_module,
                     absl::string_view entry_point,
                     ref_ptr<ES3Program> program);
  ~ES3ComputePipeline() override;

  ref_ptr<ES3Program> program() const { return program_; }

 private:
  ref_ptr<ES3PlatformContext> platform_context_;
  ref_ptr<ES3Program> program_;
};

class ES3RenderPipeline : public RenderPipeline {
 public:
  ES3RenderPipeline(ref_ptr<ES3PlatformContext> platform_context,
                    ref_ptr<PipelineLayout> pipeline_layout,
                    ref_ptr<RenderPass> render_pass, int render_subpass,
                    RenderState render_state,
                    RenderPipeline::ShaderStages shader_stages,
                    ref_ptr<ES3Program> program);
  ~ES3RenderPipeline() override;

  ref_ptr<ES3Program> program() const { return program_; }

 private:
  ref_ptr<ES3PlatformContext> platform_context_;
  ref_ptr<ES3Program> program_;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_PIPELINE_H_
