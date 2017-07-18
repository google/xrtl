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

#include "xrtl/base/logging.h"
#include "xrtl/base/system_clock.h"
#include "xrtl/base/threading/event.h"
#include "xrtl/base/threading/thread.h"
#include "xrtl/gfx/context.h"
#include "xrtl/gfx/context_factory.h"
#include "xrtl/gfx/spirv/shader_compiler.h"
#include "xrtl/testing/demo_main.h"
#include "xrtl/ui/display_link.h"
#include "xrtl/ui/window.h"

namespace xrtl {
namespace examples {
namespace {

using gfx::Buffer;
using gfx::Context;
using gfx::ContextFactory;
using gfx::Image;
using gfx::ImageView;
using gfx::MemoryHeap;
using gfx::RenderPass;
using gfx::RenderPipeline;
using gfx::RenderState;
using gfx::ResourceSet;
using gfx::ResourceSetLayout;
using gfx::Sampler;
using gfx::ShaderModule;
using gfx::SwapChain;
using gfx::spirv::ShaderCompiler;
using ui::Control;
using ui::Window;

// Matches the push constants block in the shader.
// NOTE: layout is std140.
struct PushConstants {
  float mix_value;
  float unused[3];
};

// Matches the uniform buffer block in the shader.
// NOTE: layout is std140.
struct UniformBlock {
  float mix_base;
  float unused[3];
};

// This example demonstrates rendering a fancy triangle with the XRTL APIs
// including how to handle errors. For a smaller example highlighting the
// graphics APIs instead see triangle_example.cc.
class TriangleFullExample : private Control::Listener {
 public:
  TriangleFullExample() {
    message_loop_ = MessageLoop::Create();
    done_event_ = Event::CreateFence();
  }

  ~TriangleFullExample() { Thread::Wait(message_loop_->Exit()); }

  ref_ptr<WaitHandle> Run() {
    // Create and open window.
    window_ = Window::Create(message_loop_);
    window_->set_title("Triangle Example");
    auto control = window_->root_control();
    control->set_listener(this);
    control->set_size({640, 480});
    control->set_background_color({255, 0, 0, 255});
    Thread::Wait(window_->Open());
    return done_event_;
  }

 private:
  // Creates a context based on flags and sets up a swap chain for display.
  // Returns false if no context could be created.
  bool CreateContext() {
    // Get a context factory for the desired context type.
    // The chosen factory will be based on the --gfx= flag or the provided
    // value.
    auto context_factory = ContextFactory::Create();
    if (!context_factory) {
      LOG(ERROR) << "Unable to create context factory";
      return false;
    }
    if (!context_factory->default_device()) {
      LOG(ERROR) << "No compatible device available for use";
      return false;
    }

    // Set required features/extensions.
    // TODO(benvanik): something sensible.
    gfx::Device::Features required_features;

    // Attempt to create the context.
    auto create_result = context_factory->CreateContext(
        context_factory->default_device(), required_features, &context_);
    switch (create_result) {
      case ContextFactory::CreateResult::kSuccess:
        break;
      default:
        LOG(ERROR) << "Failed to create context";
        return false;
    }

    // Create the swap chain used for presentation.
    swap_chain_ = context_->CreateSwapChain(
        window_->root_control(), SwapChain::PresentMode::kLowLatency, 1,
        {gfx::PixelFormats::kB8G8R8A8UNorm});
    if (!swap_chain_) {
      LOG(ERROR) << "Failed to create swap chain";
      return false;
    }

    // Allocate a memory heap to allocate buffers and textures.
    memory_heap_ = context_->CreateMemoryHeap(
        gfx::MemoryType::kHostVisible | gfx::MemoryType::kHostCoherent,
        16 * 1024 * 1024);
    if (!memory_heap_) {
      LOG(ERROR) << "Unable to create memory heap";
      return false;
    }

    return true;
  }

  // Creates the input geometry for the triangle.
  // Returns false if the geometry could not be prepared.
  bool CreateGeometry() {
    struct Vertex {
      float x, y, z;
      float u, v;
      float r, g, b, a;
    };
    const Vertex kVertexData[] = {
        {1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},   // v0
        {-1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f},  // v1
        {0.0f, -1.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f},  // v2
    };

    // Allocate a buffer for the geometry.
    auto allocation_result = memory_heap_->AllocateBuffer(
        sizeof(kVertexData), Buffer::Usage::kVertexBuffer, &triangle_buffer_);
    switch (allocation_result) {
      case MemoryHeap::AllocationResult::kSuccess:
        break;
      default:
        LOG(ERROR) << "Failed to allocate geometry buffer";
        return false;
    }

    // Write data directly into the buffer.
    // A real app would want to use a staging buffer.
    if (!triangle_buffer_->WriteData(0, kVertexData, sizeof(kVertexData))) {
      LOG(ERROR) << "Failed to write data into geometry buffer";
      return false;
    }

    return true;
  }

  // Creates a grid pattern texture we blend onto the triangle.
  // Returns false if the texture or sampler could not be prepared.
  bool CreateGridTexture() {
    const int kWidth = 8;
    const int kHeight = 8;

    std::vector<uint32_t> image_data(kWidth * kHeight);
    for (int y = 0; y < 8; ++y) {
      int yg = y < 4 ? 1 : 0;
      for (int x = 0; x < 8; ++x) {
        int xg = x < 4 ? 1 : 0;
        image_data[y * kWidth + x] = (xg ^ yg) ? 0xFFFFFFFF : 0xFF000000;
      }
    }

    gfx::Image::CreateParams create_params;
    create_params.format = gfx::PixelFormats::kR8G8B8A8UNorm;
    create_params.tiling_mode = Image::TilingMode::kLinear;
    create_params.size = {kWidth, kHeight};
    create_params.initial_layout = Image::Layout::kPreinitialized;

    auto allocation_result = memory_heap_->AllocateImage(
        create_params, Image::Usage::kSampled, &grid_image_);
    switch (allocation_result) {
      case MemoryHeap::AllocationResult::kSuccess:
        break;
      default:
        LOG(ERROR) << "Failed to allocate texture image";
        return false;
    }

    // Write data directly into the image.
    // A real app would want to use a staging buffer.
    if (!grid_image_->WriteData(grid_image_->entire_range(), image_data.data(),
                                image_data.size() * 4)) {
      LOG(ERROR) << "Failed to write data into texture image";
      return false;
    }

    // Create simple view into the image.
    grid_image_view_ = grid_image_->CreateView();
    if (!grid_image_view_) {
      LOG(ERROR) << "Failed to create image view";
      return false;
    }

    // Create a nearest-neighbor sampler we'll use for the grid.
    Sampler::Params sampler_params;
    nearest_sampler_ = context_->CreateSampler(sampler_params);
    if (!nearest_sampler_) {
      LOG(ERROR) << "Failed to create sampler";
      return false;
    }

    return true;
  }

  // Creates a render pipeline used to render our triangle.
  // Returns false if the pipeline could not be prepared.
  bool CreateRenderPipeline() {
    // Create render pass.
    RenderPass::AttachmentDescription color_attachment;
    color_attachment.format = gfx::PixelFormats::kB8G8R8A8UNorm;
    color_attachment.load_op = RenderPass::LoadOp::kClear;
    color_attachment.store_op = RenderPass::StoreOp::kStore;
    color_attachment.initial_layout = Image::Layout::kUndefined;
    color_attachment.final_layout = Image::Layout::kPresentSource;
    RenderPass::SubpassDescription subpass;
    subpass.color_attachments.push_back(
        {0, Image::Layout::kColorAttachmentOptimal});
    render_pass_ =
        context_->CreateRenderPass({color_attachment}, {subpass}, {});
    if (!render_pass_) {
      LOG(ERROR) << "Unable to create render pass";
      return false;
    }

    // Prepare render state.
    RenderState render_state;
    render_state.vertex_input_state.vertex_bindings.push_back(
        {0, sizeof(float) * 9});
    render_state.vertex_input_state.vertex_attributes.push_back(
        {0, 0, 0, gfx::VertexFormats::kX32Y32Z32SFloat});
    render_state.vertex_input_state.vertex_attributes.push_back(
        {1, 0, sizeof(float) * 3, gfx::VertexFormats::kX32Y32SFloat});
    render_state.vertex_input_state.vertex_attributes.push_back(
        {2, 0, sizeof(float) * 5, gfx::VertexFormats::kX32Y32Z32W32SFloat});
    render_state.input_assembly_state.set_primitive_topology(
        gfx::PrimitiveTopology::kTriangleList);
    render_state.viewport_state.set_count(1);

    // Compile a shader module from GLSL. Real applications would want to do
    // this offline.
    auto vert_shader_compiler =
        make_unique<ShaderCompiler>(ShaderCompiler::SourceLanguage::kGlsl,
                                    ShaderCompiler::ShaderStage::kVertex);
    vert_shader_compiler->AddSource(R"""(#version 310 es
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color;
layout(location = 0) out vec2 v_uv;
layout(location = 1) out vec4 v_color;
void main() {
  gl_Position = vec4(a_position, 1.0);
  v_uv = a_uv;
  v_color = a_color;
}
)""");
    std::vector<uint32_t> vert_shader_data;
    if (!vert_shader_compiler->Compile(&vert_shader_data)) {
      LOG(FATAL) << "Could not compile vertex shader: " << std::endl
                 << vert_shader_compiler->compile_log();
    }
    auto frag_shader_compiler =
        make_unique<ShaderCompiler>(ShaderCompiler::SourceLanguage::kGlsl,
                                    ShaderCompiler::ShaderStage::kFragment);
    frag_shader_compiler->AddSource(R"""(#version 310 es
precision highp float;
layout(push_constant, std140) uniform PushConstants {
  float mix_value;
} push_constants;
layout(location = 0) in vec2 v_uv;
layout(location = 1) in vec4 v_color;
layout(location = 0) out vec4 out_color;
layout(set = 0, binding = 0) uniform sampler2D image_sampler;
layout(set = 0, binding = 1, std140) uniform UniformBlock {
  float mix_base;
} uniform_block;
void main() {
  float mix_value = push_constants.mix_value * uniform_block.mix_base;
  vec4 tex_sample = texture(image_sampler, v_uv);
  out_color = vec4(mix(v_color.rgb, tex_sample.rgb, v_color.a * mix_value),
                   1.0);
}
)""");
    std::vector<uint32_t> frag_shader_data;
    if (!frag_shader_compiler->Compile(&frag_shader_data)) {
      LOG(FATAL) << "Could not compile fragment shader: " << std::endl
                 << frag_shader_compiler->compile_log();
    }

    // Load the shader module binaries.
    auto vertex_shader_module = context_->CreateShaderModule(
        ShaderModule::DataFormat::kSpirV, vert_shader_data);
    if (!vertex_shader_module) {
      LOG(ERROR) << "Unable to load vertex shader module";
      return false;
    }
    auto fragment_shader_module = context_->CreateShaderModule(
        ShaderModule::DataFormat::kSpirV, frag_shader_data);
    if (!fragment_shader_module) {
      LOG(ERROR) << "Unable to load fragment shader module";
      return false;
    }

    // Create shader modules.
    RenderPipeline::ShaderStages shader_stages;
    shader_stages.vertex_shader_module = vertex_shader_module;
    shader_stages.vertex_entry_point = "main";
    shader_stages.fragment_shader_module = fragment_shader_module;
    shader_stages.fragment_entry_point = "main";

    // Pipeline layout.
    auto resource_set_layout = context_->CreateResourceSetLayout({
        {0, ResourceSetLayout::BindingSlot::Type::kCombinedImageSampler},
        {1, ResourceSetLayout::BindingSlot::Type::kUniformBuffer},
    });
    auto pipeline_layout = context_->CreatePipelineLayout(
        {
            resource_set_layout,
        },
        {
            {offsetof(PushConstants, mix_value),
             sizeof(PushConstants::mix_value)},
        });
    if (!pipeline_layout) {
      LOG(ERROR) << "Unable to create pipeline layout";
      return false;
    }

    // Create the pipeline.
    render_pipeline_ =
        context_->CreateRenderPipeline(pipeline_layout, render_pass_, 0,
                                       render_state, std::move(shader_stages));
    if (!render_pipeline_) {
      LOG(ERROR) << "Unable to create render pipeline";
      return false;
    }

    // Allocate the uniform buffer.
    auto allocation_result = memory_heap_->AllocateBuffer(
        sizeof(UniformBlock), Buffer::Usage::kUniformBuffer, &uniform_buffer_);
    switch (allocation_result) {
      case MemoryHeap::AllocationResult::kSuccess:
        break;
      default:
        LOG(ERROR) << "Failed to allocate uniform buffer";
        return false;
    }

    // Create the resource set we'll use for the triangle.
    resource_set_ = context_->CreateResourceSet(
        resource_set_layout,
        {
            {grid_image_view_, Image::Layout::kGeneral, nearest_sampler_},
            {uniform_buffer_},
        });
    if (!resource_set_) {
      LOG(ERROR) << "Unable to create resource set";
      return false;
    }

    return true;
  }

  // Draws a single frame and presents it to the screen.
  bool DrawFrame(std::chrono::microseconds timestamp_utc_micros) {
    // Create a command buffer for the render commands.
    auto command_buffer = context_->CreateCommandBuffer();
    if (!command_buffer) {
      LOG(ERROR) << "Unable to create command buffer";
      return false;
    }

    // Acquire a framebuffer to render into.
    auto framebuffer_ready_fence = context_->CreateQueueFence();
    ref_ptr<ImageView> framebuffer_image_view;
    auto acquire_result = swap_chain_->AcquireNextImage(
        std::chrono::milliseconds(16), framebuffer_ready_fence,
        &framebuffer_image_view);
    switch (acquire_result) {
      case SwapChain::AcquireResult::kSuccess:
        break;
      case SwapChain::AcquireResult::kResizeRequired:
        LOG(WARNING) << "Swap chain resize required";
        break;
      case SwapChain::AcquireResult::kTimeout:
        // TODO(benvanik): render thread so we don't block the message loop.
        LOG(WARNING) << "Swap chain acquire timeout; running too slow and "
                        "skipping frame";
        return true;
      default:
        LOG(ERROR) << "Failed to acquire framebuffer";
        return false;
    }

    // TODO(benvanik): cache framebuffers for each image view.
    auto framebuffer = context_->CreateFramebuffer(
        render_pass_, framebuffer_image_view->size(), {framebuffer_image_view});
    if (!framebuffer) {
      LOG(ERROR) << "Unable to create framebuffer";
      return false;
    }

    // Update uniform buffer data.
    auto rce = command_buffer->BeginRenderCommands();
    UniformBlock uniform_block;
    uniform_block.mix_base = 0.75f;
    rce->UpdateBuffer(uniform_buffer_, 0, &uniform_block,
                      sizeof(uniform_block));
    command_buffer->EndRenderCommands(std::move(rce));

    // Draw triangle.
    auto rpe = command_buffer->BeginRenderPass(
        render_pass_, framebuffer, {gfx::ClearColor(1.0f, 0.0f, 1.0f, 1.0f)});
    rpe->SetViewport({framebuffer_image_view->size().width,
                      framebuffer_image_view->size().height});
    rpe->BindPipeline(render_pipeline_);
    rpe->BindResourceSet(0, resource_set_);
    rpe->BindVertexBuffers(0, {triangle_buffer_});
    PushConstants push_constants;
    push_constants.mix_value =
        (SystemClock::default_clock()->now_millis().count() % 1000) / 1000.0f;
    rpe->PushConstants(
        render_pipeline_->pipeline_layout(), gfx::ShaderStageFlag::kFragment, 0,
        &push_constants.mix_value, sizeof(push_constants.mix_value));
    rpe->Draw(3);
    command_buffer->EndRenderPass(std::move(rpe));

    // Submit command buffer for drawing the triangle. We wait until the
    // framebuffer is ready for rendering.
    auto render_complete_fence = context_->CreateQueueFence();
    auto submit_result =
        context_->Submit(std::move(framebuffer_ready_fence),
                         std::move(command_buffer), render_complete_fence);
    switch (submit_result) {
      case Context::SubmitResult::kSuccess:
        break;
      default:
        LOG(ERROR) << "Failed to submit rendering commands";
        return false;
    }

    // Submit the framebuffer for presentation.
    auto present_result = swap_chain_->PresentImage(
        std::move(render_complete_fence), framebuffer_image_view);
    switch (present_result) {
      case SwapChain::PresentResult::kSuccess:
        break;
      case SwapChain::PresentResult::kResizeRequired:
        LOG(WARNING) << "Swap chain resize required; resizing now";
        context_->WaitUntilQueuesIdle();
        if (swap_chain_->Resize(window_->root_control()->size()) !=
            SwapChain::ResizeResult::kSuccess) {
          LOG(ERROR) << "Failed to resize swap chain";
          return false;
        }
        break;
      default:
        LOG(ERROR) << "Failed to present framebuffer";
        return false;
    }

    return true;
  }

  void OnError(ref_ptr<Control> target) override {
    LOG(INFO) << "OnError";
    done_event_->Set();
  }

  void OnCreating(ref_ptr<Control> target) override {
    LOG(INFO) << "OnCreating";
  }

  void OnCreated(ref_ptr<Control> target) override {
    LOG(INFO) << "OnCreated";

    // Setup everything for rendering.
    if (!CreateContext() || !CreateGeometry() || !CreateGridTexture() ||
        !CreateRenderPipeline()) {
      LOG(ERROR) << "Failed to initialize graphics resources";
      done_event_->Set();
      return;
    }

    // Start the frame loop.
    target->display_link()->Start(
        [this](std::chrono::microseconds timestamp_utc_micros) {
          // NOTE: this may be called back from an arbitrary thread!
          DrawFrame(timestamp_utc_micros);
        });
  }

  void OnDestroying(ref_ptr<Control> target) override {
    LOG(INFO) << "OnDestroying";

    target->display_link()->Stop();
    if (swap_chain_) {
      swap_chain_->DiscardPendingPresents();
    }
    if (context_) {
      context_->WaitUntilQueuesIdle();
    }
    uniform_buffer_.reset();
    nearest_sampler_.reset();
    grid_image_view_.reset();
    grid_image_.reset();
    triangle_buffer_.reset();
    resource_set_.reset();
    render_pipeline_.reset();
    render_pass_.reset();
    memory_heap_.reset();
    swap_chain_.reset();
    context_.reset();
  }

  void OnDestroyed(ref_ptr<Control> target) override {
    LOG(INFO) << "OnDestroyed";
    done_event_->Set();
  }

  void OnSystemThemeChanged(ref_ptr<Control> target) override {
    LOG(INFO) << "OnSystemThemeChanged";
  }

  void OnSuspendChanged(ref_ptr<Control> target, bool is_suspended) override {
    LOG(INFO) << "OnSuspendChanged: " << is_suspended;
  }

  void OnFocusChanged(ref_ptr<Control> target, bool is_focused) override {
    LOG(INFO) << "OnFocusChanged: " << is_focused;
  }

  void OnResized(ref_ptr<Control> target, Rect2D bounds) override {
    LOG(INFO) << "OnResized: " << bounds.origin.x << "," << bounds.origin.y
              << " " << bounds.size.width << "x" << bounds.size.height;
  }

  ref_ptr<MessageLoop> message_loop_;
  ref_ptr<Window> window_;
  ref_ptr<Event> done_event_;

  ref_ptr<Context> context_;
  ref_ptr<SwapChain> swap_chain_;

  ref_ptr<RenderPass> render_pass_;
  ref_ptr<RenderPipeline> render_pipeline_;
  ref_ptr<ResourceSet> resource_set_;

  ref_ptr<MemoryHeap> memory_heap_;
  ref_ptr<Buffer> triangle_buffer_;
  ref_ptr<Image> grid_image_;
  ref_ptr<ImageView> grid_image_view_;
  ref_ptr<Sampler> nearest_sampler_;
  ref_ptr<Buffer> uniform_buffer_;
};

int TriangleFullEntry(int argc, char** argv) {
  auto demo = make_unique<TriangleFullExample>();
  Thread::Wait(demo->Run());
  demo.reset();
  LOG(INFO) << "Clean exit!";
  return 0;
}

}  // namespace
}  // namespace examples
}  // namespace xrtl

DECLARE_ENTRY_POINT(xrtl::examples::TriangleFullEntry);
