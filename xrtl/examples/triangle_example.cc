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
using gfx::ShaderModule;
using gfx::SwapChain;
using gfx::spirv::ShaderCompiler;
using ui::Control;
using ui::Window;

// This example is a simple colored triangle. Though it demonstrates a lot of
// the API if you are interested in textures, uniform buffers/push constants,
// and how to properly handle errors see triangle_full_example.cc.
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
  void CreateContext() {
    // Get a context factory for the desired context type.
    // The chosen factory will be based on the --gfx= flag or the provided
    // value.
    auto context_factory = ContextFactory::Create();
    CHECK(context_factory);
    CHECK(context_factory->default_device());

    // Set required features/extensions.
    // TODO(benvanik): something sensible.
    gfx::Device::Features required_features;

    // Attempt to create the context.
    auto create_result = context_factory->CreateContext(
        context_factory->default_device(), required_features, &context_);
    CHECK_EQ(ContextFactory::CreateResult::kSuccess, create_result);

    // Create the swap chain used for presentation.
    swap_chain_ = context_->CreateSwapChain(
        window_->root_control(), SwapChain::PresentMode::kLowLatency, 1,
        {gfx::PixelFormats::kB8G8R8A8UNorm});
    CHECK(swap_chain_);

    // Allocate a memory heap to allocate buffers and textures.
    memory_heap_ = context_->CreateMemoryHeap(
        gfx::MemoryType::kHostVisible | gfx::MemoryType::kHostCoherent,
        16 * 1024 * 1024);
    CHECK(memory_heap_);
  }

  // Creates the input geometry for the triangle.
  void CreateGeometry() {
    struct Vertex {
      float x, y, z;
      float r, g, b, a;
    };
    const Vertex kVertexData[] = {
        {1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},   // v0
        {-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},  // v1
        {0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f},  // v2
    };

    // Allocate a buffer for the geometry.
    auto allocation_result = memory_heap_->AllocateBuffer(
        sizeof(kVertexData), Buffer::Usage::kVertexBuffer, &triangle_buffer_);
    CHECK_EQ(MemoryHeap::AllocationResult::kSuccess, allocation_result);

    // Write data directly into the buffer.
    // A real app would want to use a staging buffer.
    bool did_write =
        triangle_buffer_->WriteData(0, kVertexData, sizeof(kVertexData));
    CHECK(did_write);
  }

  // Creates a render pipeline used to render our triangle.
  void CreateRenderPipeline() {
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
    CHECK(render_pass_);

    // Prepare render state.
    RenderState render_state;
    render_state.vertex_input_state.vertex_bindings.push_back(
        {0, sizeof(float) * 7});
    render_state.vertex_input_state.vertex_attributes.push_back(
        {0, 0, 0, gfx::VertexFormats::kX32Y32Z32SFloat});
    render_state.vertex_input_state.vertex_attributes.push_back(
        {1, 0, sizeof(float) * 3, gfx::VertexFormats::kX32Y32Z32W32SFloat});
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
layout(location = 1) in vec4 a_color;
layout(location = 0) out vec4 v_color;
void main() {
  gl_Position = vec4(a_position, 1.0);
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
    CHECK(vertex_shader_module);
    auto fragment_shader_module = context_->CreateShaderModule(
        ShaderModule::DataFormat::kSpirV, frag_shader_data);
    CHECK(fragment_shader_module);

    // Create shader modules.
    RenderPipeline::ShaderStages shader_stages;
    shader_stages.vertex_shader_module = vertex_shader_module;
    shader_stages.vertex_entry_point = "main";
    shader_stages.fragment_shader_module = fragment_shader_module;
    shader_stages.fragment_entry_point = "main";

    // Create the pipeline.
    auto pipeline_layout = context_->CreatePipelineLayout({}, {});
    render_pipeline_ =
        context_->CreateRenderPipeline(pipeline_layout, render_pass_, 0,
                                       render_state, std::move(shader_stages));
    CHECK(render_pipeline_);
  }

  // Draws a single frame and presents it to the screen.
  void DrawFrame(std::chrono::microseconds timestamp_utc_micros) {
    // Create a command buffer for the render commands.
    auto command_buffer = context_->CreateCommandBuffer();
    CHECK(command_buffer);

    // Acquire a framebuffer to render into.
    auto framebuffer_ready_fence = context_->CreateQueueFence();
    ref_ptr<ImageView> framebuffer_image_view;
    auto acquire_result = swap_chain_->AcquireNextImage(
        std::chrono::milliseconds(100), framebuffer_ready_fence,
        &framebuffer_image_view);
    CHECK(acquire_result == SwapChain::AcquireResult::kSuccess ||
          acquire_result == SwapChain::AcquireResult::kResizeRequired);

    // TODO(benvanik): cache framebuffers for each image view.
    auto framebuffer = context_->CreateFramebuffer(
        render_pass_, framebuffer_image_view->size(), {framebuffer_image_view});
    CHECK(framebuffer);

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
    CHECK_EQ(Context::SubmitResult::kSuccess, submit_result);

    // Submit the framebuffer for presentation.
    auto present_result = swap_chain_->PresentImage(
        std::move(render_complete_fence), framebuffer_image_view);
    CHECK(present_result == SwapChain::PresentResult::kSuccess ||
          present_result == SwapChain::PresentResult::kResizeRequired);
  }

  void OnError(ref_ptr<Control> target) override {
    LOG(INFO) << "OnError";
    done_event_->Set();
  }

  void OnCreated(ref_ptr<Control> target) override {
    // Setup everything for rendering.
    CreateContext();
    CreateGeometry();
    CreateRenderPipeline();

    // Start the frame loop.
    target->display_link()->Start(
        [this](std::chrono::microseconds timestamp_utc_micros) {
          // NOTE: this may be called back from an arbitrary thread!
          DrawFrame(timestamp_utc_micros);
        });
  }

  void OnDestroying(ref_ptr<Control> target) override {
    target->display_link()->Stop();
    if (swap_chain_) {
      swap_chain_->DiscardPendingPresents();
    }
    if (context_) {
      context_->WaitUntilQueuesIdle();
    }
    triangle_buffer_.reset();
    render_pipeline_.reset();
    render_pass_.reset();
    memory_heap_.reset();
    swap_chain_.reset();
    context_.reset();
  }

  void OnDestroyed(ref_ptr<Control> target) override { done_event_->Set(); }

  ref_ptr<MessageLoop> message_loop_;
  ref_ptr<Window> window_;
  ref_ptr<Event> done_event_;

  ref_ptr<Context> context_;
  ref_ptr<SwapChain> swap_chain_;

  ref_ptr<RenderPass> render_pass_;
  ref_ptr<RenderPipeline> render_pipeline_;

  ref_ptr<MemoryHeap> memory_heap_;
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
