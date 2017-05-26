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
using gfx::Framebuffer;
using gfx::Image;
using gfx::ImageView;
using gfx::MemoryPool;
using gfx::Pipeline;
using gfx::PipelineLayout;
using gfx::RenderPass;
using gfx::RenderPipeline;
using gfx::RenderState;
using gfx::ShaderModule;
using gfx::SwapChain;
using gfx::spirv::ShaderCompiler;
using ui::Control;
using ui::Window;

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
    auto create_result =
        context_factory->CreateContext(context_factory->default_device(),
                                       std::move(required_features), &context_);
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
        {0, sizeof(float) * 6});
    render_state.vertex_input_state.vertex_attributes.push_back(
        {0, 0, 0, gfx::VertexFormats::kX32Y32Z32SFloat});
    render_state.vertex_input_state.vertex_attributes.push_back(
        {1, 0, sizeof(float) * 3, gfx::VertexFormats::kX32Y32Z32SFloat});
    render_state.input_assembly_state.set_primitive_topology(
        gfx::PrimitiveTopology::kTriangleList);
    render_state.viewport_state.set_count(1);

    // Compile a shader module from GLSL. Real applications would want to do
    // this offline.
    auto vert_shader_compiler =
        make_unique<ShaderCompiler>(ShaderCompiler::SourceLanguage::kGlsl,
                                    ShaderCompiler::ShaderStage::kVertex);
    vert_shader_compiler->AddSource(R"""(#version 310 es
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_color;
layout(location = 0) out vec4 v_color;
void main() {
  gl_Position = vec4(a_position.xyz, 1.0);
  v_color = vec4(a_color, 1.0);
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
layout(location = 0) in vec4 v_color;
layout(location = 0) out vec4 out_color;
void main() {
  out_color = v_color;
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
    auto pipeline_layout = context_->CreatePipelineLayout({}, {});

    // Create the pipeline.
    render_pipeline_ = context_->CreateRenderPipeline(
        pipeline_layout, render_pass_, 0, std::move(render_state),
        std::move(shader_stages));
    if (!render_pipeline_) {
      LOG(ERROR) << "Unable to create render pipeline";
      return false;
    }

    return true;
  }

  // Creates the input geometry for the triangle.
  // Returns false if the geometry could not be prepared.
  bool CreateGeometry() {
    // Allocate a memory pool to allocate the buffer.
    memory_pool_ = context_->CreateMemoryPool(
        gfx::MemoryType::kHostVisible | gfx::MemoryType::kHostCoherent,
        1 * 1024 * 1024);
    if (!memory_pool_) {
      LOG(ERROR) << "Unable to create memory pool";
      return false;
    }

    const float kVertexData[] = {
        1.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f,  // v0
        -1.0f, 1.0f,  0.0f, 0.0f, 1.0f, 0.0f,  // v1
        0.0f,  -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // v2
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

    // Draw triangle.
    auto rpe = command_buffer->BeginRenderPass(
        render_pass_, framebuffer, {gfx::ClearColor(1.0f, 0.0f, 1.0f, 1.0f)});
    rpe->SetViewport({framebuffer_image_view->size().width,
                      framebuffer_image_view->size().height});
    rpe->BindPipeline(render_pipeline_);
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
    if (!CreateContext() || !CreateRenderPipeline() || !CreateGeometry()) {
      LOG(FATAL) << "Failed to launch example";
      done_event_->Set();
    }
  }

  void OnDestroying(ref_ptr<Control> target) override {
    LOG(INFO) << "OnDestroying";

    triangle_buffer_.reset();
    memory_pool_.reset();
    render_pipeline_.reset();
    render_pass_.reset();
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

  ref_ptr<MemoryPool> memory_pool_;
  ref_ptr<Buffer> triangle_buffer_;
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
