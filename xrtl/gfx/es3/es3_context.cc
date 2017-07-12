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
#include "xrtl/gfx/es3/es3_command_buffer.h"
#include "xrtl/gfx/es3/es3_command_fence.h"
#include "xrtl/gfx/es3/es3_framebuffer.h"
#include "xrtl/gfx/es3/es3_image_view.h"
#include "xrtl/gfx/es3/es3_memory_pool.h"
#include "xrtl/gfx/es3/es3_pipeline.h"
#include "xrtl/gfx/es3/es3_pipeline_layout.h"
#include "xrtl/gfx/es3/es3_program.h"
#include "xrtl/gfx/es3/es3_queue_fence.h"
#include "xrtl/gfx/es3/es3_render_pass.h"
#include "xrtl/gfx/es3/es3_resource_set.h"
#include "xrtl/gfx/es3/es3_resource_set_layout.h"
#include "xrtl/gfx/es3/es3_sampler.h"
#include "xrtl/gfx/es3/es3_shader.h"
#include "xrtl/gfx/es3/es3_shader_module.h"
#include "xrtl/gfx/es3/es3_swap_chain.h"
#include "xrtl/gfx/util/memory_command_buffer.h"
#include "xrtl/gfx/util/memory_command_decoder.h"

namespace xrtl {
namespace gfx {
namespace es3 {

ES3Context::ES3Context(ref_ptr<ContextFactory> context_factory,
                       std::vector<ref_ptr<Device>> devices,
                       Device::Features features,
                       ref_ptr<ES3PlatformContext> platform_context)
    : Context(std::move(devices), features),
      context_factory_(std::move(context_factory)),
      platform_context_(std::move(platform_context)) {
  // Spawn the thread that will execute command buffers.
  queue_work_pending_event_ = Event::CreateAutoResetEvent(false);
  queue_work_completed_event_ = Event::CreateAutoResetEvent(false);
  Thread::CreateParams create_params;
  create_params.name = "ES3ContextQueueThread";
  queue_thread_ = Thread::Create(create_params, QueueThreadMain, this);
}

ES3Context::~ES3Context() {
  // Join with queue thread.
  if (queue_thread_) {
    {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      queue_running_ = false;
    }
    queue_work_pending_event_->Set();
    queue_thread_->Join();
  }
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
  return make_ref<ES3QueueFence>();
}

ref_ptr<CommandFence> ES3Context::CreateCommandFence() {
  return make_ref<ES3CommandFence>();
}

ref_ptr<ShaderModule> ES3Context::CreateShaderModule(
    ShaderModule::DataFormat data_format, const void* data,
    size_t data_length) {
  auto shader = make_ref<ES3Shader>(platform_context_, "main");
  if (!shader->CompileSpirVBinary(reinterpret_cast<const uint32_t*>(data),
                                  data_length / 4)) {
    LOG(ERROR) << "Failed to translate/compile SPIR-V binary";
    return nullptr;
  }
  auto shader_module = make_ref<ES3ShaderModule>(platform_context_);
  shader_module->Register(shader);
  return shader_module;
}

ref_ptr<PipelineLayout> ES3Context::CreatePipelineLayout(
    ArrayView<ref_ptr<ResourceSetLayout>> resource_set_layouts,
    ArrayView<PipelineLayout::PushConstantRange> push_constant_ranges) {
  return make_ref<ES3PipelineLayout>(resource_set_layouts,
                                     push_constant_ranges);
}

ref_ptr<ComputePipeline> ES3Context::CreateComputePipeline(
    ref_ptr<PipelineLayout> pipeline_layout,
    ref_ptr<ShaderModule> shader_module, const std::string& entry_point) {
  WTF_SCOPE0("ES3Context#CreateComputePipeline");

  auto shader = shader_module.As<ES3ShaderModule>()->Lookup(entry_point);
  if (!shader) {
    LOG(ERROR) << "Shader entry point '" << entry_point
               << "' not found in module";
    return nullptr;
  }
  std::vector<ref_ptr<ES3Shader>> shaders;
  shaders.push_back(shader);

  auto program = make_ref<ES3Program>(platform_context_, shaders);
  if (!program->Link()) {
    LOG(ERROR) << "Unable to link compute program";
    return nullptr;
  }

  return make_ref<ES3ComputePipeline>(platform_context_, pipeline_layout,
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

  auto program = make_ref<ES3Program>(platform_context_, shaders);
  if (!program->Link()) {
    LOG(ERROR) << "Unable to link render program";
    return nullptr;
  }

  return make_ref<ES3RenderPipeline>(platform_context_, pipeline_layout,
                                     render_pass, render_subpass, render_state,
                                     shader_stages, program);
}

ref_ptr<ResourceSetLayout> ES3Context::CreateResourceSetLayout(
    ArrayView<ResourceSetLayout::BindingSlot> binding_slots) {
  return make_ref<ES3ResourceSetLayout>(binding_slots);
}

ref_ptr<ResourceSet> ES3Context::CreateResourceSet(
    ref_ptr<ResourceSetLayout> resource_set_layout,
    ArrayView<ResourceSet::BindingValue> binding_values) {
  return make_ref<ES3ResourceSet>(resource_set_layout, binding_values);
}

ref_ptr<SwapChain> ES3Context::CreateSwapChain(
    ref_ptr<ui::Control> control, SwapChain::PresentMode present_mode,
    int image_count, ArrayView<PixelFormat> pixel_formats) {
  // Shared memory pool for all frame buffer images.
  // TODO(benvanik): pool across swap chains.
  auto memory_pool = CreateMemoryPool(MemoryType::kDeviceLocal, 0);
  DCHECK(memory_pool);

  return ES3SwapChain::Create(platform_context_, std::move(memory_pool),
                              std::move(control), present_mode, image_count,
                              pixel_formats);
}

ref_ptr<MemoryPool> ES3Context::CreateMemoryPool(MemoryType memory_type_mask,
                                                 size_t chunk_size) {
  return make_ref<ES3MemoryPool>(platform_context_, memory_type_mask,
                                 chunk_size);
}

ref_ptr<Sampler> ES3Context::CreateSampler(Sampler::Params params) {
  return make_ref<ES3Sampler>(platform_context_, params);
}

ref_ptr<RenderPass> ES3Context::CreateRenderPass(
    ArrayView<RenderPass::AttachmentDescription> attachments,
    ArrayView<RenderPass::SubpassDescription> subpasses,
    ArrayView<RenderPass::SubpassDependency> subpass_dependencies) {
  return make_ref<ES3RenderPass>(attachments, subpasses, subpass_dependencies);
}

ref_ptr<Framebuffer> ES3Context::CreateFramebuffer(
    ref_ptr<RenderPass> render_pass, Size3D size,
    ArrayView<ref_ptr<ImageView>> attachments) {
  return make_ref<ES3Framebuffer>(render_pass, size, attachments);
}

ref_ptr<CommandBuffer> ES3Context::CreateCommandBuffer() {
  // TODO(benvanik): pooling.
  return make_ref<util::MemoryCommandBuffer>();
}

Context::SubmitResult ES3Context::Submit(
    ArrayView<ref_ptr<QueueFence>> wait_queue_fences,
    ArrayView<ref_ptr<CommandBuffer>> command_buffers,
    ArrayView<ref_ptr<QueueFence>> signal_queue_fences,
    ref_ptr<Event> signal_handle) {
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    DCHECK(queue_running_);
    queue_.emplace(wait_queue_fences, command_buffers, signal_queue_fences,
                   signal_handle);
    queue_work_pending_event_->Set();
  }

  return SubmitResult::kSuccess;
}

Context::WaitResult ES3Context::WaitUntilQueuesIdle() {
  while (true) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (!queue_running_ || queue_.empty()) {
      return WaitResult::kSuccess;
    }
    Thread::Wait(queue_work_completed_event_);
  }
  return WaitResult::kSuccess;
}

Context::WaitResult ES3Context::WaitUntilQueuesIdle(
    OperationQueueMask queue_mask) {
  // We only have one queue so no need to mask.
  return WaitUntilQueuesIdle();
}

void ES3Context::QueueThreadMain(void* param) {
  reinterpret_cast<ES3Context*>(param)->RunQueue();
}

void ES3Context::RunQueue() {
  // Acquire and lock the GL context we'll use to execute commands. It's only
  // ever used by this thread so it's safe to keep active forever.
  ES3PlatformContext::ThreadLock context_lock(
      ES3PlatformContext::AcquireThreadContext(platform_context_));
  if (!context_lock.is_held()) {
    LOG(FATAL) << "Unable to make current the queue platform context";
  }

  // Create the native command buffer that takes a recorded memory command
  // buffer and makes GL calls.
  auto queue_command_buffer = make_ref<ES3CommandBuffer>();

  while (true) {
    QueueEntry queue_entry;
    {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      if (!queue_running_) {
        // Queue is shutting down; exit thread.
        break;
      }

      // Attempt to dequeue a command buffer set.
      if (!queue_.empty()) {
        queue_entry = std::move(queue_.front());
        queue_.pop();
      } else {
        // Signal that there was no work available.
        queue_work_completed_event_->Set();
      }
    }

    // If there was no work pending wait for more work.
    if (queue_entry.command_buffers.empty()) {
      Thread::Wait(queue_work_pending_event_);
      continue;
    }

    // Wait on queue fences.
    std::vector<ref_ptr<WaitHandle>> wait_events;
    wait_events.reserve(queue_entry.wait_queue_fences.size());
    for (const auto& queue_fence : queue_entry.wait_queue_fences) {
      wait_events.push_back(queue_fence.As<ES3QueueFence>()->event());
    }
    if (Thread::WaitAll(wait_events) != Thread::WaitResult::kSuccess) {
      break;
    }

    // Execute command buffers.
    for (const auto& command_buffer : queue_entry.command_buffers) {
      // Reset GL state.
      queue_command_buffer->PrepareState();

      // Get the underlying memory command buffer stream.
      auto memory_command_buffer =
          command_buffer.As<util::MemoryCommandBuffer>();
      auto command_reader = memory_command_buffer->GetReader();

      // Execute the command buffer against our native GL implementation.
      util::MemoryCommandDecoder::Decode(&command_reader,
                                         queue_command_buffer.get());

      // Reset our execution command buffer to clear all state.
      // This ensures that the next time we start using it the state is clean
      // (as expected by command buffers).
      queue_command_buffer->Reset();

      // Reset the command buffer now that we have executed it. This should
      // release any resources kept alive exclusively by the command buffer.
      memory_command_buffer->Reset();
    }

    // TODO(benvanik): avoid? need to flush to ensure presents on other threads
    //                 see the outputs, probably.
    glFlush();

    // Signal queue fences.
    for (const auto& queue_fence : queue_entry.signal_queue_fences) {
      queue_fence.As<ES3QueueFence>()->event()->Set();
    }

    // Signal CPU event.
    if (queue_entry.signal_handle) {
      queue_entry.signal_handle->Set();
    }
  }

  queue_work_completed_event_->Set();
  queue_command_buffer.reset();
  context_lock.reset();
  ES3PlatformContext::ReleaseThreadContext();
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
