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

#include "xrtl/gfx/es3/es3_context.h"

#include "xrtl/base/tracing.h"
#include "xrtl/gfx/es3/es3_buffer.h"
#include "xrtl/gfx/es3/es3_command_buffer.h"
#include "xrtl/gfx/es3/es3_command_fence.h"
#include "xrtl/gfx/es3/es3_framebuffer.h"
#include "xrtl/gfx/es3/es3_image.h"
#include "xrtl/gfx/es3/es3_image_view.h"
#include "xrtl/gfx/es3/es3_memory_heap.h"
#include "xrtl/gfx/es3/es3_pipeline.h"
#include "xrtl/gfx/es3/es3_pipeline_layout.h"
#include "xrtl/gfx/es3/es3_program.h"
#include "xrtl/gfx/es3/es3_queue.h"
#include "xrtl/gfx/es3/es3_queue_fence.h"
#include "xrtl/gfx/es3/es3_render_pass.h"
#include "xrtl/gfx/es3/es3_resource_set.h"
#include "xrtl/gfx/es3/es3_resource_set_layout.h"
#include "xrtl/gfx/es3/es3_sampler.h"
#include "xrtl/gfx/es3/es3_shader.h"
#include "xrtl/gfx/es3/es3_shader_module.h"
#include "xrtl/gfx/es3/es3_swap_chain.h"
#include "xrtl/gfx/util/memory_command_buffer.h"

namespace xrtl {
namespace gfx {
namespace es3 {

ES3Context::ES3Context(ref_ptr<ContextFactory> context_factory,
                       absl::Span<const ref_ptr<Device>> devices,
                       Device::Features features,
                       ref_ptr<ES3PlatformContext> platform_context)
    : Context(devices, features),
      context_factory_(std::move(context_factory)),
      platform_context_(std::move(platform_context)) {
  // Setup the work queues.
  primary_queue_ = absl::make_unique<ES3Queue>(
      ES3Queue::Type::kCommandSubmission, platform_context_);
  presentation_queue_ =
      absl::make_unique<ES3Queue>(ES3Queue::Type::kPresentation, nullptr);
}

ES3Context::~ES3Context() {
  // Join with queue threads.
  presentation_queue_.reset();
  primary_queue_.reset();
}

bool ES3Context::DeserializePipelineCache(const void* existing_data,
                                          size_t existing_data_length) {
  // TODO(benvanik): program binaries.
  return true;
}

std::vector<uint8_t> ES3Context::SerializePipelineCache() {
  // TODO(benvanik): program binaries.
  return {};
}

ref_ptr<QueueFence> ES3Context::CreateQueueFence() {
  return make_ref<ES3QueueFence>(primary_queue_.get());
}

ref_ptr<CommandFence> ES3Context::CreateCommandFence() {
  return make_ref<ES3CommandFence>();
}

ref_ptr<ShaderModule> ES3Context::CreateShaderModule(
    ShaderModule::DataFormat data_format, const void* data,
    size_t data_length) {
  WTF_SCOPE0("ES3Context#CreateShaderModule");

  // If this gets slow we could move this to a dedicated translation/compilation
  // thread.
  auto shader = make_ref<ES3Shader>("main");
  if (!shader->TranslateSpirVBinary(reinterpret_cast<const uint32_t*>(data),
                                    data_length / 4)) {
    LOG(ERROR) << "Failed to translate SPIR-V binary";
    return nullptr;
  }

  auto shader_module = make_ref<ES3ShaderModule>(primary_queue_.get());
  shader_module->Register(shader);
  return shader_module;
}

ref_ptr<PipelineLayout> ES3Context::CreatePipelineLayout(
    absl::Span<const ref_ptr<ResourceSetLayout>> resource_set_layouts,
    absl::Span<const PipelineLayout::PushConstantRange> push_constant_ranges) {
  return make_ref<ES3PipelineLayout>(resource_set_layouts,
                                     push_constant_ranges);
}

ref_ptr<ComputePipeline> ES3Context::CreateComputePipeline(
    ref_ptr<PipelineLayout> pipeline_layout,
    ref_ptr<ShaderModule> shader_module, absl::string_view entry_point) {
  WTF_SCOPE0("ES3Context#CreateComputePipeline");

  auto shader = shader_module.As<ES3ShaderModule>()->Lookup(entry_point);
  if (!shader) {
    LOG(ERROR) << "Shader entry point '" << entry_point
               << "' not found in module";
    return nullptr;
  }
  ref_ptr<ES3Shader> shaders[1] = {shader};

  auto program = make_ref<ES3Program>(shaders);
  if (!program->Link()) {
    LOG(ERROR) << "Unable to link compute program";
    return nullptr;
  }

  return make_ref<ES3ComputePipeline>(primary_queue_.get(), pipeline_layout,
                                      shader_module, entry_point, program);
}

ref_ptr<RenderPipeline> ES3Context::CreateRenderPipeline(
    ref_ptr<PipelineLayout> pipeline_layout, ref_ptr<RenderPass> render_pass,
    int render_subpass, RenderState render_state,
    RenderPipeline::ShaderStages shader_stages) {
  WTF_SCOPE0("ES3Context#CreateRenderPipeline");

  std::vector<ref_ptr<ES3Shader>> shaders;
  if (!shader_stages.vertex_entry_point.empty()) {
    DCHECK(shader_stages.vertex_shader_module);
    shaders.push_back(
        shader_stages.vertex_shader_module.As<ES3ShaderModule>()->Lookup(
            shader_stages.vertex_entry_point));
    if (!shaders.back()) {
      LOG(ERROR) << "Vertex shader entry point '"
                 << shader_stages.vertex_entry_point << "' not found";
      return nullptr;
    }
  }
  if (!shader_stages.tessellation_control_entry_point.empty()) {
    DCHECK(shader_stages.tessellation_control_shader_module);
    shaders.push_back(
        shader_stages.tessellation_control_shader_module.As<ES3ShaderModule>()
            ->Lookup(shader_stages.tessellation_control_entry_point));
    if (!shaders.back()) {
      LOG(ERROR) << "Tessellation control shader entry point '"
                 << shader_stages.tessellation_control_entry_point
                 << "' not found";
      return nullptr;
    }
  }
  if (!shader_stages.tessellation_evaluation_entry_point.empty()) {
    DCHECK(shader_stages.tessellation_evaluation_shader_module);
    shaders.push_back(
        shader_stages.tessellation_evaluation_shader_module
            .As<ES3ShaderModule>()
            ->Lookup(shader_stages.tessellation_evaluation_entry_point));
    if (!shaders.back()) {
      LOG(ERROR) << "Tessellation evaluation shader entry point '"
                 << shader_stages.tessellation_evaluation_entry_point
                 << "' not found";
      return nullptr;
    }
  }
  if (!shader_stages.geometry_entry_point.empty()) {
    DCHECK(shader_stages.geometry_shader_module);
    shaders.push_back(
        shader_stages.geometry_shader_module.As<ES3ShaderModule>()->Lookup(
            shader_stages.geometry_entry_point));
    if (!shaders.back()) {
      LOG(ERROR) << "Geometry shader entry point '"
                 << shader_stages.geometry_entry_point << "' not found";
      return nullptr;
    }
  }
  if (!shader_stages.fragment_entry_point.empty()) {
    DCHECK(shader_stages.fragment_shader_module);
    shaders.push_back(
        shader_stages.fragment_shader_module.As<ES3ShaderModule>()->Lookup(
            shader_stages.fragment_entry_point));
    if (!shaders.back()) {
      LOG(ERROR) << "Fragment shader entry point '"
                 << shader_stages.fragment_entry_point << "' not found";
      return nullptr;
    }
  }

  // Note that though we are creating the program here we aren't linking it
  // until first use.
  auto program =
      make_ref<ES3Program>(absl::Span<const ref_ptr<ES3Shader>>(shaders));
  return make_ref<ES3RenderPipeline>(primary_queue_.get(), pipeline_layout,
                                     render_pass, render_subpass, render_state,
                                     shader_stages, program);
}

ref_ptr<ResourceSetLayout> ES3Context::CreateResourceSetLayout(
    absl::Span<const BindingSlot> binding_slots) {
  return make_ref<ES3ResourceSetLayout>(binding_slots);
}

ref_ptr<ResourceSet> ES3Context::CreateResourceSet(
    ref_ptr<ResourceSetLayout> resource_set_layout,
    absl::Span<const BindingValue> binding_values) {
  return make_ref<ES3ResourceSet>(resource_set_layout, binding_values);
}

ref_ptr<SwapChain> ES3Context::CreateSwapChain(
    ref_ptr<ui::Control> control, SwapChain::PresentMode present_mode,
    int image_count, absl::Span<const PixelFormat> pixel_formats) {
  WTF_SCOPE0("ES3Context#CreateSwapChain");

  // Shared memory pool for all frame buffer images.
  // TODO(benvanik): pool across swap chains.
  auto memory_heap =
      CreateMemoryHeap(MemoryType::kDeviceLocal, 64 * 1024 * 1024);
  DCHECK(memory_heap);

  return ES3SwapChain::Create(platform_context_, primary_queue_.get(),
                              presentation_queue_.get(), std::move(memory_heap),
                              std::move(control), present_mode, image_count,
                              pixel_formats);
}

ref_ptr<MemoryHeap> ES3Context::CreateMemoryHeap(MemoryType memory_type_mask,
                                                 size_t heap_size) {
  WTF_SCOPE0("ES3Context#CreateMemoryHeap");
  return make_ref<ES3MemoryHeap>(primary_queue_.get(), memory_type_mask,
                                 heap_size);
}

ref_ptr<Sampler> ES3Context::CreateSampler(Sampler::Params params) {
  return make_ref<ES3Sampler>(primary_queue_.get(), params);
}

ref_ptr<RenderPass> ES3Context::CreateRenderPass(
    absl::Span<const RenderPass::AttachmentDescription> attachments,
    absl::Span<const RenderPass::SubpassDescription> subpasses,
    absl::Span<const RenderPass::SubpassDependency> subpass_dependencies) {
  return make_ref<ES3RenderPass>(attachments, subpasses, subpass_dependencies);
}

ref_ptr<Framebuffer> ES3Context::CreateFramebuffer(
    ref_ptr<RenderPass> render_pass, Size3D size,
    absl::Span<const ref_ptr<ImageView>> attachments) {
  return make_ref<ES3Framebuffer>(render_pass, size, attachments);
}

ref_ptr<CommandBuffer> ES3Context::CreateCommandBuffer() {
  // TODO(benvanik): pooling.
  return make_ref<util::MemoryCommandBuffer>();
}

Context::SubmitResult ES3Context::Submit(
    absl::Span<const ref_ptr<QueueFence>> wait_queue_fences,
    absl::Span<const ref_ptr<CommandBuffer>> command_buffers,
    absl::Span<const ref_ptr<QueueFence>> signal_queue_fences,
    ref_ptr<Event> signal_handle) {
  WTF_SCOPE0("ES3Context#Submit");
  primary_queue_->EnqueueCommandBuffers(
      wait_queue_fences, nullptr, command_buffers, nullptr, signal_queue_fences,
      std::move(signal_handle));
  return SubmitResult::kSuccess;
}

Context::WaitResult ES3Context::WaitUntilQueuesIdle() {
  return WaitUntilQueuesIdle(OperationQueueMask::kAll);
}

Context::WaitResult ES3Context::WaitUntilQueuesIdle(
    OperationQueueMask queue_mask) {
  WTF_SCOPE0("ES3Context#WaitUntilQueuesIdle");
  bool any_failed = false;
  if (any(queue_mask &
          (OperationQueueMask::kRender | OperationQueueMask::kCompute |
           OperationQueueMask::kTransfer))) {
    any_failed = !primary_queue_->WaitUntilIdle() || any_failed;
  }
  if (any(queue_mask & OperationQueueMask::kPresent)) {
    any_failed = !presentation_queue_->WaitUntilIdle() || any_failed;
  }
  return any_failed ? WaitResult::kDeviceLost : WaitResult::kSuccess;
}

bool ES3Context::ReadBufferData(
    ref_ptr<Buffer> source_buffer,
    absl::Span<const ReadBufferRegion> data_regions) {
  WTF_SCOPE0("ES3Context#ReadBufferData:sync");
  ref_ptr<Event> fence = Event::CreateFence();
  if (!ReadBufferData({}, std::move(source_buffer), data_regions, {}, fence)) {
    return false;
  }
  return Thread::Wait(fence) == Thread::WaitResult::kSuccess;
}

bool ES3Context::ReadBufferData(
    absl::Span<const ref_ptr<QueueFence>> wait_queue_fences,
    ref_ptr<Buffer> source_buffer,
    absl::Span<const ReadBufferRegion> data_regions,
    absl::Span<const ref_ptr<QueueFence>> signal_queue_fences,
    ref_ptr<Event> signal_handle) {
  WTF_SCOPE0("ES3Context#ReadBufferData");
  // TODO(benvanik): staging buffer and copy command buffer.
  std::vector<ReadBufferRegion> data_regions_copy{data_regions.begin(),
                                                  data_regions.end()};
  auto data_regions_baton = MoveToLambda(data_regions_copy);
  primary_queue_->EnqueueCommandBuffers(
      wait_queue_fences,
      [source_buffer, data_regions_baton]() {
        WTF_SCOPE0("ES3Context#ReadBufferData:queue");
        source_buffer.As<ES3Buffer>()->ReadDataRegionsOnQueue(
            data_regions_baton.value);
      },
      {}, nullptr, signal_queue_fences, std::move(signal_handle));
  return true;
}

bool ES3Context::WriteBufferData(
    ref_ptr<Buffer> target_buffer,
    absl::Span<const WriteBufferRegion> data_regions) {
  WTF_SCOPE0("ES3Context#WriteBufferData:sync");
  ref_ptr<Event> fence = Event::CreateFence();
  if (!WriteBufferData({}, std::move(target_buffer), data_regions, {}, fence)) {
    return false;
  }
  return Thread::Wait(fence) == Thread::WaitResult::kSuccess;
}

bool ES3Context::WriteBufferData(
    absl::Span<const ref_ptr<QueueFence>> wait_queue_fences,
    ref_ptr<Buffer> target_buffer,
    absl::Span<const WriteBufferRegion> data_regions,
    absl::Span<const ref_ptr<QueueFence>> signal_queue_fences,
    ref_ptr<Event> signal_handle) {
  WTF_SCOPE0("ES3Context#WriteBufferData");
  // TODO(benvanik): staging buffer and copy command buffer.
  std::vector<WriteBufferRegion> data_regions_copy{data_regions.begin(),
                                                   data_regions.end()};
  auto data_regions_baton = MoveToLambda(data_regions_copy);
  primary_queue_->EnqueueCommandBuffers(
      wait_queue_fences,
      [target_buffer, data_regions_baton]() {
        WTF_SCOPE0("ES3Context#WriteBufferData:queue");
        target_buffer.As<ES3Buffer>()->WriteDataRegionsOnQueue(
            data_regions_baton.value);
      },
      {}, nullptr, signal_queue_fences, std::move(signal_handle));
  return true;
}

bool ES3Context::ReadImageData(ref_ptr<Image> source_image,
                               absl::Span<const ReadImageRegion> data_regions) {
  WTF_SCOPE0("ES3Context#ReadImageData:sync");
  ref_ptr<Event> fence = Event::CreateFence();
  if (!ReadImageData({}, std::move(source_image), data_regions, {}, fence)) {
    return false;
  }
  return Thread::Wait(fence) == Thread::WaitResult::kSuccess;
}

bool ES3Context::ReadImageData(
    absl::Span<const ref_ptr<QueueFence>> wait_queue_fences,
    ref_ptr<Image> source_image, absl::Span<const ReadImageRegion> data_regions,
    absl::Span<const ref_ptr<QueueFence>> signal_queue_fences,
    ref_ptr<Event> signal_handle) {
  WTF_SCOPE0("ES3Context#ReadImageData");
  // TODO(benvanik): staging buffer and copy command buffer.
  std::vector<ReadImageRegion> data_regions_copy{data_regions.begin(),
                                                 data_regions.end()};
  auto data_regions_baton = MoveToLambda(data_regions_copy);
  primary_queue_->EnqueueCommandBuffers(
      wait_queue_fences,
      [source_image, data_regions_baton]() {
        WTF_SCOPE0("ES3Context#ReadImageData:queue");
        source_image.As<ES3Image>()->ReadDataRegionsOnQueue(
            data_regions_baton.value);
      },
      {}, nullptr, signal_queue_fences, std::move(signal_handle));
  return true;
}

bool ES3Context::WriteImageData(
    ref_ptr<Image> target_image,
    absl::Span<const WriteImageRegion> data_regions) {
  WTF_SCOPE0("ES3Context#WriteImageData:sync");
  ref_ptr<Event> fence = Event::CreateFence();
  if (!WriteImageData({}, std::move(target_image), data_regions, {}, fence)) {
    return false;
  }
  return Thread::Wait(fence) == Thread::WaitResult::kSuccess;
}

bool ES3Context::WriteImageData(
    absl::Span<const ref_ptr<QueueFence>> wait_queue_fences,
    ref_ptr<Image> target_image,
    absl::Span<const WriteImageRegion> data_regions,
    absl::Span<const ref_ptr<QueueFence>> signal_queue_fences,
    ref_ptr<Event> signal_handle) {
  WTF_SCOPE0("ES3Context#WriteImageData");
  // TODO(benvanik): staging buffer and copy command buffer.
  std::vector<WriteImageRegion> data_regions_copy{data_regions.begin(),
                                                  data_regions.end()};
  auto data_regions_baton = MoveToLambda(data_regions_copy);
  primary_queue_->EnqueueCommandBuffers(
      wait_queue_fences,
      [target_image, data_regions_baton]() {
        WTF_SCOPE0("ES3Context#WriteImageData:queue");
        target_image.As<ES3Image>()->WriteDataRegionsOnQueue(
            data_regions_baton.value);
      },
      {}, nullptr, signal_queue_fences, std::move(signal_handle));
  return true;
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
