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
#include "xrtl/testing/demo_main.h"
#include "xrtl/ui/display_link.h"
#include "xrtl/ui/imgui_overlay.h"
#include "xrtl/ui/window.h"

namespace xrtl {
namespace ui {
namespace {

using gfx::Context;
using gfx::ContextFactory;
using gfx::Image;
using gfx::ImageView;
using gfx::RenderPass;
using gfx::SwapChain;

class ImGuiOverlayDemo : private Control::Listener {
 public:
  ImGuiOverlayDemo() {
    message_loop_ = MessageLoop::Create();
    done_event_ = Event::CreateFence();
  }

  ~ImGuiOverlayDemo() { Thread::Wait(message_loop_->Exit()); }

  ref_ptr<WaitHandle> Run() {
    window_ = Window::Create(message_loop_);
    window_->set_title("ImGui Overlay Demo");
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

    // Create render pass to clear the screen.
    RenderPass::AttachmentDescription color_attachment;
    color_attachment.format = gfx::PixelFormats::kB8G8R8A8UNorm;
    color_attachment.load_op = RenderPass::LoadOp::kClear;
    color_attachment.store_op = RenderPass::StoreOp::kStore;
    color_attachment.initial_layout = Image::Layout::kUndefined;
    color_attachment.final_layout = Image::Layout::kColorAttachmentOptimal;
    RenderPass::SubpassDescription subpass;
    subpass.color_attachments.push_back(
        {0, Image::Layout::kColorAttachmentOptimal});
    render_pass_ =
        context_->CreateRenderPass({color_attachment}, {subpass}, {});
    if (!render_pass_) {
      LOG(ERROR) << "Unable to create render pass";
      return false;
    }

    return true;
  }

  // Creates the imgui layer for the window.
  bool CreateImGuiOverlay() {
    // Create the imgui layer (and allocate resources/etc).
    imgui_overlay_.reset(new ImGuiOverlay());
    if (!imgui_overlay_->Initialize(context_)) {
      LOG(ERROR) << "Failed to initialize imgui layer";
      return false;
    }

    // Route control input to the imgui layer.
    window_->root_control()->set_input_listener(imgui_overlay_.get());

    return true;
  }

  // Draws a single frame and presents it to the screen.
  bool DrawFrame(std::chrono::microseconds timestamp_utc_micros) {
    // Create a command buffer for the render commands.
    auto scene_command_buffer = context_->CreateCommandBuffer();
    if (!scene_command_buffer) {
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

    // Begin the imgui frame now. It'll record but not issue any commands until
    // EndFrame is called below.
    imgui_overlay_->BeginFrame(framebuffer);

    // Issue a small render pass that clears the screen.
    // Normal apps would draw their scene here.
    auto rpe = scene_command_buffer->BeginRenderPass(
        render_pass_, framebuffer, {gfx::ClearColor(1.0f, 0.0f, 1.0f, 1.0f)});
    scene_command_buffer->EndRenderPass(std::move(rpe));

    // Draw some UI.
    ImGui::ShowUserGuide();
    ImGui::ShowStyleEditor();
    ImGui::ShowTestWindow();
    ImGui::ShowMetricsWindow();

    // Submit the command buffer for the scene. It should go first so imgui
    // draws on top.
    auto scene_complete_fence = context_->CreateQueueFence();
    auto submit_result =
        context_->Submit(std::move(framebuffer_ready_fence),
                         std::move(scene_command_buffer), scene_complete_fence);
    switch (submit_result) {
      case Context::SubmitResult::kSuccess:
        break;
      default:
        LOG(ERROR) << "Failed to submit rendering commands";
        return false;
    }

    // End the imgui frame and flush its commands.
    // We'll wait on the scene fence so that imgui is correctly composited.
    auto imgui_complete_fence = imgui_overlay_->EndFrame(scene_complete_fence);

    // Submit the framebuffer for presentation.
    auto present_result = swap_chain_->PresentImage(
        std::move(imgui_complete_fence), framebuffer_image_view);
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
    if (!CreateContext() || !CreateImGuiOverlay()) {
      LOG(ERROR) << "Failed to initialize graphics resources";
      done_event_->Set();
      return;
    }

    // Start the frame loop.
    target->display_link()->Start(
        [this](std::chrono::microseconds timestamp_utc_micros) {
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
    imgui_overlay_.reset();
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
  }

  ref_ptr<MessageLoop> message_loop_;
  MessageLoop::TaskList pending_task_list_;
  ref_ptr<Window> window_;
  ref_ptr<Event> done_event_;

  ref_ptr<Context> context_;
  ref_ptr<SwapChain> swap_chain_;
  ref_ptr<RenderPass> render_pass_;

  std::unique_ptr<ImGuiOverlay> imgui_overlay_;
};

int MyEntry(int argc, char** argv) {
  auto demo = make_unique<ImGuiOverlayDemo>();
  Thread::Wait(demo->Run());
  demo.reset();
  LOG(INFO) << "Clean exit!";
  return 0;
}

}  // namespace
}  // namespace ui
}  // namespace xrtl

DECLARE_ENTRY_POINT(xrtl::ui::MyEntry);
