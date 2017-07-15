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

#ifndef XRTL_GFX_ES3_ES3_CONTEXT_H_
#define XRTL_GFX_ES3_ES3_CONTEXT_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "xrtl/base/threading/thread.h"
#include "xrtl/gfx/context.h"
#include "xrtl/gfx/context_factory.h"
#include "xrtl/gfx/es3/es3_common.h"
#include "xrtl/gfx/es3/es3_platform_context.h"

namespace xrtl {
namespace gfx {
namespace es3 {

class ES3Queue;

class ES3Context : public Context {
 public:
  ES3Context(ref_ptr<ContextFactory> context_factory,
             std::vector<ref_ptr<Device>> devices, Device::Features features,
             ref_ptr<ES3PlatformContext> platform_context);
  ~ES3Context() override;

  // GL platform context used for object allocation.
  ref_ptr<ES3PlatformContext> platform_context() const {
    return platform_context_;
  }

  bool DeserializePipelineCache(const void* existing_data,
                                size_t existing_data_length) override;

  std::vector<uint8_t> SerializePipelineCache() override;

  ref_ptr<QueueFence> CreateQueueFence() override;

  ref_ptr<CommandFence> CreateCommandFence() override;

  ref_ptr<ShaderModule> CreateShaderModule(ShaderModule::DataFormat data_format,
                                           const void* data,
                                           size_t data_length) override;

  ref_ptr<PipelineLayout> CreatePipelineLayout(
      ArrayView<ref_ptr<ResourceSetLayout>> resource_set_layouts,
      ArrayView<PipelineLayout::PushConstantRange> push_constant_ranges)
      override;

  ref_ptr<ComputePipeline> CreateComputePipeline(
      ref_ptr<PipelineLayout> pipeline_layout,
      ref_ptr<ShaderModule> shader_module,
      const std::string& entry_point) override;

  ref_ptr<RenderPipeline> CreateRenderPipeline(
      ref_ptr<PipelineLayout> pipeline_layout, ref_ptr<RenderPass> render_pass,
      int render_subpass, RenderState render_state,
      RenderPipeline::ShaderStages shader_stages) override;

  ref_ptr<ResourceSetLayout> CreateResourceSetLayout(
      ArrayView<ResourceSetLayout::BindingSlot> binding_slots) override;

  ref_ptr<ResourceSet> CreateResourceSet(
      ref_ptr<ResourceSetLayout> resource_set_layout,
      ArrayView<ResourceSet::BindingValue> binding_values) override;

  ref_ptr<SwapChain> CreateSwapChain(
      ref_ptr<ui::Control> control, SwapChain::PresentMode present_mode,
      int image_count, ArrayView<PixelFormat> pixel_formats) override;

  ref_ptr<MemoryHeap> CreateMemoryHeap(MemoryType memory_type_mask,
                                       size_t heap_size) override;

  ref_ptr<Sampler> CreateSampler(Sampler::Params params) override;

  ref_ptr<RenderPass> CreateRenderPass(
      ArrayView<RenderPass::AttachmentDescription> attachments,
      ArrayView<RenderPass::SubpassDescription> subpasses,
      ArrayView<RenderPass::SubpassDependency> subpass_dependencies) override;

  ref_ptr<Framebuffer> CreateFramebuffer(
      ref_ptr<RenderPass> render_pass, Size3D size,
      ArrayView<ref_ptr<ImageView>> attachments) override;

  ref_ptr<CommandBuffer> CreateCommandBuffer() override;

  SubmitResult Submit(ArrayView<ref_ptr<QueueFence>> wait_queue_fences,
                      ArrayView<ref_ptr<CommandBuffer>> command_buffers,
                      ArrayView<ref_ptr<QueueFence>> signal_queue_fences,
                      ref_ptr<Event> signal_handle) override;

  WaitResult WaitUntilQueuesIdle() override;

  WaitResult WaitUntilQueuesIdle(OperationQueueMask queue_mask) override;

 private:
  static void QueueThreadMain(void* param);
  void RunQueue();

  // Keep the context factory alive (as it owns our primary process EGL handle).
  ref_ptr<ContextFactory> context_factory_;

  // GL platform context used for object allocation.
  // Other contexts are used for queue management and swap chains.
  ref_ptr<ES3PlatformContext> platform_context_;

  // Primary command queue that owns the dedicated submission thread.
  std::unique_ptr<ES3Queue> primary_queue_;
  // Presentation queue used for swap chain presents.
  std::unique_ptr<ES3Queue> presentation_queue_;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_CONTEXT_H_
