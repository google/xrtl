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

#include "absl/strings/string_view.h"
#include "absl/types/span.h"
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

// Defines a ReadBufferData buffer region.
struct ReadBufferRegion {
  ReadBufferRegion() = default;
  ReadBufferRegion(size_t source_offset, void* target_data,
                   size_t target_data_length)
      : source_offset(source_offset),
        target_data(target_data),
        target_data_length(target_data_length) {}

  // Byte offset into the source buffer to read the data from.
  size_t source_offset = 0;

  // Target data buffer to populate with the buffer contents.
  // The buffer must remain valid for the duration of the read operation.
  void* target_data = nullptr;
  size_t target_data_length = 0;
};

// Defines a WriteBufferData buffer region.
struct WriteBufferRegion {
  WriteBufferRegion() = default;
  WriteBufferRegion(size_t target_offset, const void* source_data,
                    size_t source_data_length)
      : target_offset(target_offset),
        source_data(source_data),
        source_data_length(source_data_length) {}

  // Byte offset into the target buffer to write the data to.
  size_t target_offset = 0;

  // Source data buffer to read buffer contents from.
  // The buffer must remain valid for the duration of the write operation.
  const void* source_data = nullptr;
  size_t source_data_length = 0;
};

// Defines a ReadImageData buffer region.
struct ReadImageRegion {
  ReadImageRegion() = default;
  ReadImageRegion(Image::LayerRange source_layer_range, void* target_data,
                  size_t target_data_length)
      : source_layer_range(source_layer_range),
        target_data(target_data),
        target_data_length(target_data_length) {}

  // Layer range in the source image to read the data from.
  Image::LayerRange source_layer_range;

  // Target data buffer to populate with the image contents.
  // The buffer must remain valid for the duration of the read operation.
  void* target_data = nullptr;
  size_t target_data_length = 0;
};

// Defines a WriteImageData buffer region.
struct WriteImageRegion {
  WriteImageRegion() = default;
  WriteImageRegion(Image::LayerRange target_layer_range,
                   const void* source_data, size_t source_data_length)
      : target_layer_range(target_layer_range),
        source_data(source_data),
        source_data_length(source_data_length) {}

  // Layer range in the target image to write the data to.
  Image::LayerRange target_layer_range;

  // Source data buffer to read image contents from.
  // The buffer must remain valid for the duration of the write operation.
  const void* source_data = nullptr;
  size_t source_data_length = 0;
};

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
  absl::Span<const ref_ptr<Device>> devices() const { return devices_; }

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
      absl::Span<const ref_ptr<ResourceSetLayout>> resource_set_layouts,
      absl::Span<const PipelineLayout::PushConstantRange>
          push_constant_ranges) = 0;

  // Creates a compute pipeline with the given shader.
  virtual ref_ptr<ComputePipeline> CreateComputePipeline(
      ref_ptr<PipelineLayout> pipeline_layout,
      ref_ptr<ShaderModule> shader_module, absl::string_view entry_point) = 0;

  // Creates a render pipeline with the given shaders and parameters.
  virtual ref_ptr<RenderPipeline> CreateRenderPipeline(
      ref_ptr<PipelineLayout> pipeline_layout, ref_ptr<RenderPass> render_pass,
      int render_subpass, RenderState render_state,
      RenderPipeline::ShaderStages shader_stages) = 0;

  // Creates a resource set layout.
  virtual ref_ptr<ResourceSetLayout> CreateResourceSetLayout(
      absl::Span<const BindingSlot> binding_slots) = 0;

  // Creates a binding set used to bind resources to pipelines.
  // A binding set is only tied to a particular pipeline layout and may be used
  // with any pipeline sharing that layout.
  // The binding values provided must match 1:1 with the bindings as defined in
  // the pipeline layout.
  virtual ref_ptr<ResourceSet> CreateResourceSet(
      ref_ptr<ResourceSetLayout> resource_set_layout,
      absl::Span<const BindingValue> binding_values) = 0;

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
      int image_count, absl::Span<const PixelFormat> pixel_formats) = 0;

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
      absl::Span<const RenderPass::AttachmentDescription> attachments,
      absl::Span<const RenderPass::SubpassDescription> subpasses,
      absl::Span<const RenderPass::SubpassDependency> subpass_dependencies) = 0;

  // Creates a new framebuffer for the given render pass.
  // The sizes of the attachments provided must be greater than or equal to
  // the provided framebuffer size. All attachments must match the render
  // pass attachment order and formats.
  //
  // TODO(benvanik): device limits.
  virtual ref_ptr<Framebuffer> CreateFramebuffer(
      ref_ptr<RenderPass> render_pass, Size3D size,
      absl::Span<const ref_ptr<ImageView>> attachments) = 0;

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
      absl::Span<const ref_ptr<QueueFence>> wait_queue_fences,
      absl::Span<const ref_ptr<CommandBuffer>> command_buffers,
      absl::Span<const ref_ptr<QueueFence>> signal_queue_fences,
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

  // Reads blocks of data from the buffer at the given offsets.
  // This performs no synchronization with the underlying memory and callers
  // must ensure that there are no commands in-flight that modify the data.
  //
  // The source buffer must have been allocated with a usage mode including
  // Buffer::Usage::kTransferSource.
  //
  // This may block on the context queues and should be avoided. Prefer to use
  // the asynchronous ReadBufferData or a command buffer with CopyBuffer into a
  // staging buffer instead.
  //
  // Returns true if the number of requested bytes were written into the buffer.
  //
  // Usage:
  //  std::vector<uint8_t> buffer(source_buffer_size);
  //  ReadBufferData(source_buffer, {{0, buffer.data(), buffer.size()}});
  virtual bool ReadBufferData(
      ref_ptr<Buffer> source_buffer,
      absl::Span<const ReadBufferRegion> data_regions) = 0;

  // Reads blocks of data from the buffer at the given offsets.
  // This synchronizes on the provided queue fences and then signals once the
  // read has completed and data has been fully populated.
  //
  // The data pointer provided must remain valid until the read completes.
  //
  // The source buffer must have been allocated with a usage mode including
  // Buffer::Usage::kTransferSource.
  //
  // This is roughly equivalent to submitting a command buffer with a
  // CopyBuffer into a mapped staging buffer and memcpy'ing the data out,
  // only it may be slightly more efficient than replicating this yourself on
  // certain implementations. The internal staging buffer may require
  // reallocation and can cause unpredictable memory growth if not careful. If
  // deep pipelining of reads is required it's best to implement that yourself.
  //
  // Returns true if the asynchronous read request is issued. The source buffer
  // is available for writes as soon as the signal_queue_fences are signaled and
  // the read heap data is available on the CPU after the signal_handle has been
  // signaled.
  virtual bool ReadBufferData(
      absl::Span<const ref_ptr<QueueFence>> wait_queue_fences,
      ref_ptr<Buffer> source_buffer,
      absl::Span<const ReadBufferRegion> data_regions,
      absl::Span<const ref_ptr<QueueFence>> signal_queue_fences,
      ref_ptr<Event> signal_handle) = 0;
  bool ReadBufferData(ref_ptr<QueueFence> wait_queue_fence,
                      ref_ptr<Buffer> source_buffer,
                      absl::Span<const ReadBufferRegion> data_regions,
                      ref_ptr<QueueFence> signal_queue_fence) {
    return ReadBufferData({wait_queue_fence}, std::move(source_buffer),
                          data_regions, {signal_queue_fence}, nullptr);
  }
  bool ReadBufferData(ref_ptr<QueueFence> wait_queue_fence,
                      ref_ptr<Buffer> source_buffer,
                      absl::Span<const ReadBufferRegion> data_regions,
                      ref_ptr<Event> signal_handle) {
    return ReadBufferData({wait_queue_fence}, std::move(source_buffer),
                          data_regions, {}, std::move(signal_handle));
  }

  // Writes blocks of data into the buffer at the given offsets.
  // This performs no synchronization with the underlying memory and callers
  // must ensure that there are no commands in-flight that modify the data.
  //
  // The target buffer must have been allocated with a usage mode including
  // Buffer::Usage::kTransferTarget.
  //
  // This may block on the context queues and should be avoided. Prefer to use
  // the asynchronous WriteBufferData or a command buffer with CopyBuffer from a
  // staging buffer instead.
  //
  // Returns true if the number of requested bytes were read from the data
  // pointer.
  //
  // Usage:
  //  std::vector<uint8_t> buffer(source_buffer_size);
  //  // ... fill buffer ...
  //  WriteBufferData(target_buffer, {{0, buffer.data(), buffer.size()}});
  virtual bool WriteBufferData(
      ref_ptr<Buffer> target_buffer,
      absl::Span<const WriteBufferRegion> data_regions) = 0;

  // Writes blocks of data into the buffer at the given offsets.
  // This synchronizes on the provided queue fences and then signals once the
  // write has completed and buffer has been fully populated.
  //
  // The data pointer provided must remain valid until the write completes.
  //
  // The target buffer must have been allocated with a usage mode including
  // Buffer::Usage::kTransferTarget.
  //
  // This is roughly equivalent to submitting a command buffer with a
  // CopyBuffer from a mapped staging buffer, only it may be slightly more
  // efficient than replicating this yourself on certain implementations. The
  // internal staging buffer may require reallocation and can cause
  // unpredictable memory growth if not careful. If deep pipelining of writes is
  // required it's best to implement that yourself.
  //
  // Returns true if the asynchronous write request is issued. The target buffer
  // is available for reads as soon as the signal_queue_fences are signaled and
  // the source heap data may be freed after the signal_handle has been
  // signaled.
  virtual bool WriteBufferData(
      absl::Span<const ref_ptr<QueueFence>> wait_queue_fences,
      ref_ptr<Buffer> target_buffer,
      absl::Span<const WriteBufferRegion> data_regions,
      absl::Span<const ref_ptr<QueueFence>> signal_queue_fences,
      ref_ptr<Event> signal_handle) = 0;
  bool WriteBufferData(ref_ptr<QueueFence> wait_queue_fence,
                       ref_ptr<Buffer> target_buffer,
                       absl::Span<const WriteBufferRegion> data_regions,
                       ref_ptr<QueueFence> signal_queue_fence) {
    return WriteBufferData({wait_queue_fence}, std::move(target_buffer),
                           data_regions, {signal_queue_fence}, nullptr);
  }
  bool WriteBufferData(ref_ptr<QueueFence> wait_queue_fence,
                       ref_ptr<Buffer> target_buffer,
                       absl::Span<const WriteBufferRegion> data_regions,
                       ref_ptr<Event> signal_handle) {
    return WriteBufferData({wait_queue_fence}, std::move(target_buffer),
                           data_regions, {}, std::move(signal_handle));
  }

  // Reads blocks of data from the image at the given source layer ranges.
  // This performs no synchronization with the underlying memory and callers
  // must ensure that there are no commands in-flight that modify the data.
  //
  // The source image must have been allocated with a usage mode including
  // Image::Usage::kTransferSource.
  //
  // This may block on the context queues and should be avoided. Prefer to use
  // the asynchronous ReadImageData or a command buffer with CopyImageToBuffer
  // into a staging buffer instead.
  //
  // Returns true if the number of requested bytes were populated into the data
  // pointer.
  //
  // Usage:
  //  std::vector<uint8_t> buffer(source_image_size);
  //  ReadImageData(source_image, {{source_image->entire_range(),
  //                                buffer.data(), buffer.size()}});
  virtual bool ReadImageData(
      ref_ptr<Image> source_image,
      absl::Span<const ReadImageRegion> data_regions) = 0;

  // Reads blocks of data from the image at the given source layer ranges.
  // This synchronizes on the provided queue fences and then signals once the
  // read has completed and data has been fully populated.
  //
  // The data pointer provided must remain valid until the read completes.
  //
  // The source image must have been allocated with a usage mode including
  // Image::Usage::kTransferSource.
  //
  // This is roughly equivalent to submitting a command buffer with a
  // CopyImageToBuffer into a mapped staging buffer and memcpy'ing the data out,
  // only it may be slightly more efficient than replicating this yourself on
  // certain implementations. The internal staging buffer may require
  // reallocation and can cause unpredictable memory growth if not careful. If
  // deep pipelining of reads is required it's best to implement that yourself.
  //
  // Returns true if the asynchronous read request is issued. The source image
  // is available for writes as soon as the signal_queue_fences are signaled and
  // the read heap data is available on the CPU after the signal_handle has been
  // signaled.
  virtual bool ReadImageData(
      absl::Span<const ref_ptr<QueueFence>> wait_queue_fences,
      ref_ptr<Image> source_image,
      absl::Span<const ReadImageRegion> data_regions,
      absl::Span<const ref_ptr<QueueFence>> signal_queue_fences,
      ref_ptr<Event> signal_handle) = 0;
  bool ReadImageData(ref_ptr<QueueFence> wait_queue_fence,
                     ref_ptr<Image> source_image,
                     absl::Span<const ReadImageRegion> data_regions,
                     ref_ptr<QueueFence> signal_queue_fence) {
    return ReadImageData({wait_queue_fence}, std::move(source_image),
                         data_regions, {signal_queue_fence}, nullptr);
  }
  bool ReadImageData(ref_ptr<QueueFence> wait_queue_fence,
                     ref_ptr<Image> source_image,
                     absl::Span<const ReadImageRegion> data_regions,
                     ref_ptr<Event> signal_handle) {
    return ReadImageData({wait_queue_fence}, std::move(source_image),
                         data_regions, {}, std::move(signal_handle));
  }

  // Writes blocks of data into the image at the given target layer ranges.
  // This performs no synchronization with the underlying memory and callers
  // must ensure that there are no commands in-flight that modify the data.
  //
  // The target image must have been allocated with a usage mode including
  // Image::Usage::kTransferTarget.
  //
  // This may block on the context queues and should be avoided. Prefer to use
  // the asynchronous WriteImageData or a command buffer with CopyBufferToImage
  // from a staging buffer instead.
  //
  // Returns true if the number of requested bytes were written to the image.
  //
  // Usage:
  //  std::vector<uint8_t> buffer(source_buffer_size);
  //  // ... fill buffer ...
  //  WriteImageData(target_image, {{target_image->entire_range(),
  //                                 buffer.data(), buffer.size()}});
  virtual bool WriteImageData(
      ref_ptr<Image> target_image,
      absl::Span<const WriteImageRegion> data_regions) = 0;

  // Writes blocks of data into the image at the given target layer ranges.
  // This synchronizes on the provided queue fences and then signals once the
  // write has completed and buffer has been fully populated.
  //
  // The data pointer provided must remain valid until the write completes.
  //
  // The target image must have been allocated with a usage mode including
  // Image::Usage::kTransferTarget.
  //
  // This is roughly equivalent to submitting a command buffer with a
  // CopyBufferToImage from a mapped staging buffer, only it may be slightly
  // more efficient than replicating this yourself on certain implementations.
  // The internal staging buffer may require reallocation and can cause
  // unpredictable memory growth if not careful. If deep pipelining of writes is
  // required it's best to implement that yourself.
  //
  // Returns true if the asynchronous write request is issued. The target image
  // is available for reads as soon as the signal_queue_fences are signaled and
  // the source heap data may be freed after the signal_handle has been
  // signaled.
  virtual bool WriteImageData(
      absl::Span<const ref_ptr<QueueFence>> wait_queue_fences,
      ref_ptr<Image> target_image,
      absl::Span<const WriteImageRegion> data_regions,
      absl::Span<const ref_ptr<QueueFence>> signal_queue_fences,
      ref_ptr<Event> signal_handle) = 0;
  bool WriteImageData(ref_ptr<QueueFence> wait_queue_fence,
                      ref_ptr<Image> target_image,
                      absl::Span<const WriteImageRegion> data_regions,
                      ref_ptr<QueueFence> signal_queue_fence) {
    return WriteImageData({wait_queue_fence}, std::move(target_image),
                          data_regions, {signal_queue_fence}, nullptr);
  }
  bool WriteImageData(ref_ptr<QueueFence> wait_queue_fence,
                      ref_ptr<Image> target_image,
                      absl::Span<const WriteImageRegion> data_regions,
                      ref_ptr<Event> signal_handle) {
    return WriteImageData({wait_queue_fence}, std::move(target_image),
                          data_regions, {}, std::move(signal_handle));
  }

 protected:
  Context(absl::Span<const ref_ptr<Device>> devices, Device::Features features)
      : devices_(devices.begin(), devices.end()),
        features_(std::move(features)) {}

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
