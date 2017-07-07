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
#include "xrtl/ui/window.h"

namespace xrtl {
namespace examples {
namespace {

using gfx::Buffer;
using gfx::Context;
using gfx::ContextFactory;
using gfx::Image;
using gfx::ImageView;
using gfx::MemoryPool;
using gfx::PipelineLayout;
using gfx::RenderPass;
using gfx::RenderPipeline;
using gfx::RenderState;
using gfx::ResourceSet;
using gfx::Sampler;
using gfx::ShaderModule;
using gfx::SwapChain;
using gfx::spirv::ShaderCompiler;
using ui::Control;
using ui::Window;

// Matches the UB block in the shader.
// NOTE: layout is std140.
struct UniformBlock {
  float mix;
  float unused[3];
};

class TriangleExample : private Control::Listener {
 public:
  TriangleExample() {
    message_loop_ = MessageLoop::Create();
    done_event_ = Event::CreateFence();
  }

  ~TriangleExample() { Thread::Wait(message_loop_->Exit()); }

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
        window_->root_control(), SwapChain::PresentMode::kLowLatency, 2,
        {gfx::PixelFormats::kB8G8R8A8UNorm});
    if (!swap_chain_) {
      LOG(ERROR) << "Failed to create swap chain";
      return false;
    }

    // Allocate a memory pool to allocate buffers and textures.
    memory_pool_ = context_->CreateMemoryPool(
        gfx::MemoryType::kHostVisible | gfx::MemoryType::kHostCoherent,
        1 * 1024 * 1024);
    if (!memory_pool_) {
      LOG(ERROR) << "Unable to create memory pool";
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
    auto allocation_result = memory_pool_->AllocateBuffer(
        sizeof(kVertexData), Buffer::Usage::kVertexBuffer, &triangle_buffer_);
    switch (allocation_result) {
      case MemoryPool::AllocationResult::kSuccess:
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
    create_params.usage_mask = Image::Usage::kSampled;
    create_params.initial_layout = Image::Layout::kPreinitialized;

    auto allocation_result =
        memory_pool_->AllocateImage(create_params, &grid_image_);
    switch (allocation_result) {
      case MemoryPool::AllocationResult::kSuccess:
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
layout(location = 0) in vec2 v_uv;
layout(location = 1) in vec4 v_color;
layout(location = 0) out vec4 out_color;
layout(binding = 0) uniform sampler2D u_image_sampler;
layout(binding = 1, std140) uniform UniformBlock {
  float mix;
  vec3 unused;
} u_uniform_block;
void main() {
  vec4 tex_sample = texture(u_image_sampler, v_uv);
  out_color = vec4(mix(v_color.rgb, tex_sample.rgb,
                       v_color.a * u_uniform_block.mix), 1.0);
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

    // Pipeline layout (in this case, empty).
    std::vector<PipelineLayout::BindingSlot> binding_slots;
    binding_slots.push_back(
        {0, PipelineLayout::BindingSlot::Type::kCombinedImageSampler});
    binding_slots.push_back(
        {1, PipelineLayout::BindingSlot::Type::kUniformBuffer});
    auto pipeline_layout = context_->CreatePipelineLayout(binding_slots, {});
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
    auto allocation_result = memory_pool_->AllocateBuffer(
        sizeof(UniformBlock), Buffer::Usage::kUniformBuffer, &uniform_buffer_);
    switch (allocation_result) {
      case MemoryPool::AllocationResult::kSuccess:
        break;
      default:
        LOG(ERROR) << "Failed to allocate uniform buffer";
        return false;
    }

    // Create the resource set we'll use for the triangle.
    std::vector<ResourceSet::BindingValue> binding_values;
    binding_values.push_back(
        {grid_image_view_, Image::Layout::kGeneral, nearest_sampler_});
    binding_values.push_back({uniform_buffer_});
    resource_set_ =
        context_->CreateResourceSet(pipeline_layout, binding_values);
    if (!resource_set_) {
      LOG(ERROR) << "Unable to create resource set";
      return false;
    }

    return true;
  }

  // Draws a single frame and presents it to the screen.
  bool DrawFrame() {
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
        framebuffer_ready_fence, &framebuffer_image_view);
    switch (acquire_result) {
      case SwapChain::AcquireResult::kSuccess:
        break;
      case SwapChain::AcquireResult::kResizeRequired:
        LOG(WARNING) << "Swap chain resize required";
        break;
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
    uniform_block.mix =
        (SystemClock::default_clock()->now_millis().count() % 1000) / 1000.0f;
    rce->UpdateBuffer(uniform_buffer_, 0, &uniform_block,
                      sizeof(uniform_block));
    command_buffer->EndRenderCommands(std::move(rce));

    // Draw triangle.
    auto rpe = command_buffer->BeginRenderPass(
        render_pass_, framebuffer, {gfx::ClearColor(1.0f, 0.0f, 1.0f, 1.0f)});
    rpe->SetViewport({framebuffer_image_view->size().width,
                      framebuffer_image_view->size().height});
    rpe->BindPipeline(render_pipeline_);
    rpe->BindResourceSet(resource_set_);
    rpe->BindVertexBuffers(0, {triangle_buffer_});
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
        // TODO(benvanik): clearer way to force redraw.
        redraw_required_ = true;
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
      LOG(ERROR) << "Failed to launch example";
      done_event_->Set();
    }
  }

  void OnDestroying(ref_ptr<Control> target) override {
    LOG(INFO) << "OnDestroying";

    uniform_buffer_.reset();
    resource_set_.reset();
    render_pipeline_.reset();
    render_pass_.reset();
    grid_image_view_.reset();
    grid_image_.reset();
    triangle_buffer_.reset();
    memory_pool_.reset();
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

    if (context_) {
      DrawFrame();
      if (redraw_required_) {
        // Immediately redraw the frame if we actually resized the surface.
        // This will prevent (most) flickering.
        // TODO(benvanik): avoid this by instead requesting a redraw. This can
        //                 cause window manager lag during resize.
        redraw_required_ = false;
        DrawFrame();
      }
    }
  }

  ref_ptr<MessageLoop> message_loop_;
  ref_ptr<Window> window_;
  ref_ptr<Event> done_event_;
  bool redraw_required_ = true;

  ref_ptr<Context> context_;
  ref_ptr<SwapChain> swap_chain_;

  ref_ptr<RenderPass> render_pass_;
  ref_ptr<RenderPipeline> render_pipeline_;
  ref_ptr<ResourceSet> resource_set_;

  ref_ptr<MemoryPool> memory_pool_;
  ref_ptr<Buffer> triangle_buffer_;
  ref_ptr<Image> grid_image_;
  ref_ptr<ImageView> grid_image_view_;
  ref_ptr<Sampler> nearest_sampler_;
  ref_ptr<Buffer> uniform_buffer_;
};

int TriangleEntry(int argc, char** argv) {
  auto demo = make_unique<TriangleExample>();
  Thread::Wait(demo->Run());
  demo.reset();
  LOG(INFO) << "Clean exit!";
  return 0;
}

}  // namespace
}  // namespace examples
}  // namespace xrtl

DECLARE_ENTRY_POINT(xrtl::examples::TriangleEntry);
