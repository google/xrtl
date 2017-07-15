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

#ifndef XRTL_UI_IMGUI_OVERLAY_H_
#define XRTL_UI_IMGUI_OVERLAY_H_

#include <chrono>
#include <memory>

#include "third_party/imgui/imgui.h"
#include "xrtl/gfx/context.h"
#include "xrtl/ui/control.h"

namespace xrtl {
namespace ui {

// ImGui (immediate mode GUI) overlay.
// This can be added to scenes to draw debug HUDs, editors, etc.
//
// Find imgui documentation and examples here:
//   https://github.com/ocornut/imgui
//
// Usage:
//   OnCreated:
//     imgui_overlay_.reset(new ImGuiOverlay());
//     imgui_overlay_->Initialize(graphics_context);
//     control->set_input_listener(imgui_overlay_.get());
//   OnFrame:
//     // Get framebuffer from swapchain/etc.
//     auto framebuffer = ...;
//     // Begin the ImGui frame. ImGui::* calls are now valid.
//     imgui_overlay_->BeginFrame(framebuffer);
//     // Draw the scene under the GUI. This may call ImGui functions to
//     // build UI.
//     DrawScene(command_buffer);
//     // Submit the scene command buffer(s) and get back a fence indicating
//     // command buffer completion.
//     auto scene_wait_fence = ...;
//     // End the ImGui frame. The GUI command buffer will execute after the
//     // wait fence is signaled.
//     auto imgui_wait_fence = imgui_overlay_->EndFrame(scene_wait_fence);
//     // Present the framebuffer. Must wait on the imgui commands with the
//     // returned fence.
//     swap_chain_->PresentImage(imgui_wait_fence, ...);
class ImGuiOverlay : public Control::InputListener {
 public:
  ImGuiOverlay();
  ~ImGuiOverlay();

  // Initializes ImGui for the given graphics context.
  // This must be called on startup.
  // Returns true if the layer was initialized and is ready for use.
  bool Initialize(ref_ptr<gfx::Context> context);

  // Begins an imgui frame targeting the given framebuffer.
  // The ImGui context owned by this layer will be made active on the current
  // thread.
  void BeginFrame(ref_ptr<gfx::Framebuffer> framebuffer);

  // Ends the imgui frame and flushes rendering commands.
  // The provided wait fence must be used to ensure that the framebuffer is
  // available for use.
  // Returns a queue fence that should be waited on before continuing to use the
  // frame buffer.
  ref_ptr<gfx::QueueFence> EndFrame(ref_ptr<gfx::QueueFence> wait_fence);

 private:
  // Maximum vertex buffer capacity in vertices.
  static constexpr int kMaxVertexCount = 64 * 1024;
  // Maximum index buffer capacity in indices.
  static constexpr int kMaxIndexCount = 64 * 1024;

  bool InitializeImGui(ImGuiIO* io);
  bool InitializeFont(ImGuiIO* io);
  bool InitializePipeline();

  static void RenderDrawListsThunk(ImDrawData* data);
  void RenderDrawLists(ImDrawData* data);

  void OnKeyDown(ref_ptr<Control> target, const KeyboardEvent& ev) override;
  void OnKeyUp(ref_ptr<Control> target, const KeyboardEvent& ev) override;
  void OnKeyPress(ref_ptr<Control> target, const KeyboardEvent& ev) override;

  void OnMouseDown(ref_ptr<Control> target, const MouseEvent& ev) override;
  void OnMouseUp(ref_ptr<Control> target, const MouseEvent& ev) override;
  void OnMouseOut(ref_ptr<Control> target, const MouseEvent& ev) override;
  void OnMouseMove(ref_ptr<Control> target, const MouseEvent& ev) override;
  void OnMouseWheel(ref_ptr<Control> target, const MouseEvent& ev) override;

  ref_ptr<gfx::Context> context_;
  ref_ptr<gfx::MemoryHeap> memory_heap_;

  ref_ptr<gfx::Sampler> nearest_sampler_;
  ref_ptr<gfx::Sampler> linear_sampler_;

  // Scratch buffers used for vertex/index data.
  // TODO(benvanik): double buffer/etc.
  ref_ptr<gfx::Buffer> vertex_buffer_;
  ref_ptr<gfx::Buffer> index_buffer_;

  // Font and the gfx resources for it.
  std::unique_ptr<ImFontAtlas> font_atlas_;
  ref_ptr<gfx::ImageView> font_image_view_;

  // Render pipeline for normal imgui drawing.
  ref_ptr<gfx::RenderPass> render_pass_;
  ref_ptr<gfx::ResourceSetLayout> resource_set_layout_;
  ref_ptr<gfx::PipelineLayout> render_pipeline_layout_;
  ref_ptr<gfx::RenderPipeline> render_pipeline_;

  // Target framebuffer for the current batch.
  ref_ptr<gfx::Framebuffer> framebuffer_;
  // Command buffer in use by the current batch.
  ref_ptr<gfx::CommandBuffer> command_buffer_;

  // ImGui context, owned by us.
  ImGuiContext* imgui_context_ = nullptr;

  // Previous timestamp of the last frame start.
  std::chrono::microseconds last_frame_now_micros_{0};
};

}  // namespace ui
}  // namespace xrtl

#endif  // XRTL_UI_IMGUI_OVERLAY_H_
