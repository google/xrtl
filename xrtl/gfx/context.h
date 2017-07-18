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

#ifndef XRTL_GFX_CONTEXT_H_
#define XRTL_GFX_CONTEXT_H_

#include <string>
#include <utility>
#include <vector>

#include "xrtl/base/array_view.h"
#include "xrtl/base/ref_ptr.h"
#include "xrtl/base/threading/event.h"
#include "xrtl/gfx/command_buffer.h"
#include "xrtl/gfx/command_fence.h"
#include "xrtl/gfx/device.h"
#include "xrtl/gfx/framebuffer.h"
#include "xrtl/gfx/image_view.h"
#include "xrtl/gfx/memory_heap.h"
#include "xrtl/gfx/pipeline.h"
#include "xrtl/gfx/pipeline_layout.h"
#include "xrtl/gfx/pixel_format.h"
#include "xrtl/gfx/queue_fence.h"
#include "xrtl/gfx/render_pass.h"
#include "xrtl/gfx/resource_set.h"
#include "xrtl/gfx/resource_set_layout.h"
#include "xrtl/gfx/sampler.h"
#include "xrtl/gfx/shader_module.h"
#include "xrtl/gfx/swap_chain.h"
#include "xrtl/ui/control.h"

namespace xrtl {
namespace gfx {

// A device (or multi-device) context.
// This is the primary interface used to allocate resources and manage command
// queues.
//
// Context operations (such as creation) are thread-safe as are the queues
// maintained by the context. The contents of the resources created must be
// synchronized by the application using barriers and external locks.
//
// Most objects created by the context are pooled, such as fences and command
// buffers. Always release references to them as soon as they are done to ensure
// they can be reused by other code.
class Context : public RefObject<Context> {
 public:
  virtual ~Context() = default;

  // The devices that are in use by the context.
  const std::vector<ref_ptr<Device>>& devices() const { return devices_; }

  // Limits of the device (or devices). Attempting to use values out of these
  // ranges will result in failures that are difficult to detect so always check
  // first.
  const Device::Limits& limits() const { return devices_[0]->limits(); }

  // Enabled device features for use by the context.
  const Device::Features& features() const { return features_; }

  // Deserializes pipeline cache data from a buffer.
  // The data provided may be used to initialize the cache, if it is compatible.
  virtual bool DeserializePipelineCache(const void* existing_data,
                                        size_t existing_data_length) = 0;
  bool DeserializePipelineCache(const std::vector<uint8_t>& existing_data) {
    return DeserializePipelineCache(existing_data.data(), existing_data.size());
  }

  // Serializes the current pipeline cache data to a buffer.
  // Applications can save this buffer and use it when recreating the pipeline
  // cache. If the platform does not support serialization the return will be
  // empty.
  virtual std::vector<uint8_t> SerializePipelineCache() = 0;

  // Creates a new queue fence that can be used to synchronize across command
  // buffer submissions to queues.
  virtual ref_ptr<QueueFence> CreateQueueFence() = 0;
  // Creates a new command fence that can be used to order commands within
  // command buffers.
  virtual ref_ptr<CommandFence> CreateCommandFence() = 0;

  // Creates a shader module from the data in the specified format.
  virtual ref_ptr<ShaderModule> CreateShaderModule(
      ShaderModule::DataFormat data_format, const void* data,
      size_t data_length) = 0;
  ref_ptr<ShaderModule> CreateShaderModule(ShaderModule::DataFormat data_format,
                                           const std::vector<uint8_t>& data) {
    return CreateShaderModule(data_format, data.data(), data.size());
  }
  ref_ptr<ShaderModule> CreateShaderModule(ShaderModule::DataFormat data_format,
                                           const std::vector<uint32_t>& data) {
    return CreateShaderModule(data_format, data.data(), data.size() * 4);
  }

  // Creates a pipeline layout.
  virtual ref_ptr<PipelineLayout> CreatePipelineLayout(
      ArrayView<ref_ptr<ResourceSetLayout>> resource_set_layouts,
      ArrayView<PipelineLayout::PushConstantRange> push_constant_ranges) = 0;

  // Creates a compute pipeline with the given shader.
  virtual ref_ptr<ComputePipeline> CreateComputePipeline(
      ref_ptr<PipelineLayout> pipeline_layout,
      ref_ptr<ShaderModule> shader_module, const std::string& entry_point) = 0;

  // Creates a render pipeline with the given shaders and parameters.
  virtual ref_ptr<RenderPipeline> CreateRenderPipeline(
      ref_ptr<PipelineLayout> pipeline_layout, ref_ptr<RenderPass> render_pass,
      int render_subpass, RenderState render_state,
      RenderPipeline::ShaderStages shader_stages) = 0;

  // Creates a resource set layout.
  virtual ref_ptr<ResourceSetLayout> CreateResourceSetLayout(
      ArrayView<ResourceSetLayout::BindingSlot> binding_slots) = 0;

  // Creates a binding set used to bind resources to pipelines.
  // A binding set is only tied to a particular pipeline layout and may be used
  // with any pipeline sharing that layout.
  // The binding values provided must match 1:1 with the bindings as defined in
  // the pipeline layout.
  virtual ref_ptr<ResourceSet> CreateResourceSet(
      ref_ptr<ResourceSetLayout> resource_set_layout,
      ArrayView<ResourceSet::BindingValue> binding_values) = 0;

  // Creates a new swap chain using the given control as a display surface.
  // The present_mode defines how the images are queued for display and
  // the image_count determines how many images are available for
  // use. Note that images can be very large and very expensive so it is
  // a good idea to keep the total count at a minimum (usually 2 for
  // double-buffering). The pixel formats are suggestions sorted priority order.
  // If none of the provided pixel formats are available for use one will be
  // chosen by the system and should be queried from the swap chain.
  // Returns nullptr if the given control does not support being a swap chain
  // target.
  virtual ref_ptr<SwapChain> CreateSwapChain(
      ref_ptr<ui::Control> control, SwapChain::PresentMode present_mode,
      int image_count, ArrayView<PixelFormat> pixel_formats) = 0;

  // Creates a new resource memory heap.
  // The heap can be used to create images and buffers of the given memory
  // type.
  //
  // The memory heap will request hardware resources in the provided heap size
  // and then dole out images and buffers from that allocation. Heap sizes
  // should be sufficiently large to prevent frequent exhaustion but not so
  // large as to potentially run out of device memory. 64-128MB is often a good
  // size to start with. The provided heap size may be rounded up to alignment
  // restrictions of the device.
  //
  // Returns nullptr if the memory type mask is invalid.
  virtual ref_ptr<MemoryHeap> CreateMemoryHeap(MemoryType memory_type_mask,
                                               size_t heap_size) = 0;

  // Creates a new image sampler.
  virtual ref_ptr<Sampler> CreateSampler(Sampler::Params params) = 0;

  // Creates a new render pass.
  virtual ref_ptr<RenderPass> CreateRenderPass(
      ArrayView<RenderPass::AttachmentDescription> attachments,
      ArrayView<RenderPass::SubpassDescription> subpasses,
      ArrayView<RenderPass::SubpassDependency> subpass_dependencies) = 0;

  // Creates a new framebuffer for the given render pass.
  // The sizes of the attachments provided must be greater than or equal to
  // the provided framebuffer size. All attachments must match the render
  // pass attachment order and formats.
  //
  // TODO(benvanik): device limits.
  virtual ref_ptr<Framebuffer> CreateFramebuffer(
      ref_ptr<RenderPass> render_pass, Size3D size,
      ArrayView<ref_ptr<ImageView>> attachments) = 0;

  // Creates a new command buffer.
  // When submitted the command buffer may be executed in parallel with other
  // command buffers based on which queues are available on the context devices.
  // Once submitted a command buffer should be released by the application so
  // that it may be recycled. Command buffer reuse is not currently supported
  // and attempting to resubmit a command buffer will result in an error.
  virtual ref_ptr<CommandBuffer> CreateCommandBuffer() = 0;

  // Defines the return value for command buffer submit operations.
  enum class SubmitResult {
    // Submit completed and the command buffers are now queued for execution.
    // This does not indicate whether they completed executing!
    kSuccess,
    // One or more of the command buffers have been submitted multiple times.
    // This is not currently supported.
    kCommandBufferReused,
    // Submit failed because the device had been lost or the submit caused it
    // to be lost.
    kDeviceLost,
  };

  // Submits one or more command buffers for execution on the context.
  // The command buffers may execute immediately or be queued for execution.
  // The execution order of command buffers submitted as a batch is in order,
  // though the commands within the buffers may execute in parallel (especially
  // likely if they use different queues) from the same submit batch or others.
  // Always use QueueFences to ensure ordering where required.
  //
  // The command buffers will wait to execute until all wait_queue_fences have
  // be signaled. After the command buffers have completed execution all
  // provided signal_queue_fences will be signaled.
  //
  // The provided signal_handle will be set when the command buffers have
  // completed execution; only then is it safe to recycle the command buffer.
  // If the submit call fails immediately due to device loss the signal will not
  // be set.
  virtual SubmitResult Submit(
      ArrayView<ref_ptr<QueueFence>> wait_queue_fences,
      ArrayView<ref_ptr<CommandBuffer>> command_buffers,
      ArrayView<ref_ptr<QueueFence>> signal_queue_fences,
      ref_ptr<Event> signal_handle) = 0;
  SubmitResult Submit(ref_ptr<CommandBuffer> command_buffer,
                      ref_ptr<QueueFence> signal_queue_fence) {
    return Submit({}, {command_buffer}, {signal_queue_fence}, nullptr);
  }
  SubmitResult Submit(ref_ptr<QueueFence> wait_queue_fence,
                      ref_ptr<CommandBuffer> command_buffer,
                      ref_ptr<QueueFence> signal_queue_fence) {
    return Submit({wait_queue_fence}, {command_buffer}, {signal_queue_fence},
                  nullptr);
  }

  // Defines the return value for queue wait operations.
  enum class WaitResult {
    // Wait completed successfully and all command buffers in the specified
    // queues have completed execution.
    kSuccess,
    // Wait failed because the device was lost while waiting.
    kDeviceLost,
  };

  // TODO(benvanik): find a way to use a WaitHandle - external fences?

  // Blocks until all queues on all devices are idle.
  // This is akin to a glFinish and should never be called during sustained
  // operation - just on major lifetime events (suspend, shutdown, etc).
  //
  // Upon successful return all command buffers that were submitted have been
  // executed and retired. If the wait fails the device may be left in an
  // indeterminate state (usually the cause of a device loss).
  virtual WaitResult WaitUntilQueuesIdle() = 0;

  // Blocks until all queues matching the mask are idle.
  // Upon successful return all command buffers that were submitted to queues
  // matching the mask will have been executed and retired. If the wait fails
  // the device may be left in an indeterminate state (usually the cause of a
  // device loss).
  virtual WaitResult WaitUntilQueuesIdle(OperationQueueMask queue_mask) = 0;

 protected:
  Context(std::vector<ref_ptr<Device>> devices, Device::Features features)
      : devices_(std::move(devices)), features_(std::move(features)) {}

  std::vector<ref_ptr<Device>> devices_;
  Device::Features features_;
};

std::ostream& operator<<(std::ostream& stream,
                         const Context::SubmitResult& value);
std::ostream& operator<<(std::ostream& stream,
                         const Context::WaitResult& value);

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_CONTEXT_H_
