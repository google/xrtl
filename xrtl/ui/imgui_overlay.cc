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

#include "xrtl/ui/imgui_overlay.h"

#include <glm/matrix.hpp>

#include <utility>
#include <vector>

#include "third_party/proggy_fonts/proggy_tiny.h"
#include "xrtl/base/macros.h"
#include "xrtl/base/system_clock.h"
#include "xrtl/ui/shaders/imgui_overlay_frag.h"
#include "xrtl/ui/shaders/imgui_overlay_vert.h"

namespace xrtl {
namespace ui {

namespace {

using gfx::Buffer;
using gfx::Image;
using gfx::MemoryHeap;
using gfx::RenderPass;
using gfx::RenderPipeline;
using gfx::RenderState;
using gfx::ResourceSetLayout;
using gfx::Sampler;
using gfx::ShaderModule;

class ImGuiLock {
 public:
  ImGuiLock() = default;
  explicit ImGuiLock(ImGuiContext* imgui_context)
      : imgui_context_(imgui_context) {
    DCHECK(imgui_context_ != nullptr);
    previous_context_ = ImGui::GetCurrentContext();
    ImGui::SetCurrentContext(imgui_context_);
  }
  ~ImGuiLock() { ImGui::SetCurrentContext(previous_context_); }

  ImGuiIO* io() const { return &ImGui::GetIO(); }

 private:
  ImGuiContext* previous_context_ = nullptr;
  ImGuiContext* imgui_context_ = nullptr;
};

struct PushConstants {
  glm::mat4 proj_matrix;
};

}  // namespace

ImGuiOverlay::ImGuiOverlay() = default;

ImGuiOverlay::~ImGuiOverlay() {
  DCHECK(!command_buffer_);

  if (imgui_context_) {
    DCHECK_NE(ImGui::GetCurrentContext(), imgui_context_);
    ImGui::SetCurrentContext(imgui_context_);
    ImGui::Shutdown();
    ImGui::DestroyContext(imgui_context_);
    imgui_context_ = nullptr;
  }

  render_pipeline_.reset();
  render_pass_.reset();

  font_image_view_.reset();
  font_atlas_.reset();

  index_buffer_.reset();
  vertex_buffer_.reset();

  linear_sampler_.reset();
  nearest_sampler_.reset();

  memory_heap_.reset();
  context_.reset();
}

bool ImGuiOverlay::Initialize(ref_ptr<gfx::Context> context) {
  context_ = std::move(context);

  // Allocate a memory heap to allocate buffers and textures.
  // TODO(benvanik): accept one to share.
  memory_heap_ = context_->CreateMemoryHeap(
      gfx::MemoryType::kHostVisible | gfx::MemoryType::kHostCoherent,
      8 * 1024 * 1024);
  if (!memory_heap_) {
    LOG(ERROR) << "Unable to create memory heap";
    return false;
  }

  // Allocate the imgui context.
  DCHECK(!imgui_context_);
  imgui_context_ = ImGui::CreateContext();
  if (!imgui_context_) {
    LOG(ERROR) << "Failed to create imgui context";
    return false;
  }

  ImGuiLock imgui(imgui_context_);
  imgui.io()->UserData = this;

  // Setup imgui for use.
  if (!InitializeImGui(imgui.io())) {
    return false;
  }
  if (!InitializeFont(imgui.io())) {
    return false;
  }

  // Setup our render pass/pipeline for use.
  if (!InitializePipeline()) {
    return false;
  }

  return true;
}

bool ImGuiOverlay::InitializeImGui(ImGuiIO* io) {
  // Setup custom rendering function.
  io->RenderDrawListsFn = &ImGuiOverlay::RenderDrawListsThunk;

  // Setup keymapping.
  // TODO(benvanik): platform mapping.
  io->KeyMap[ImGuiKey_Tab] = 0x09;         // VK_TAB
  io->KeyMap[ImGuiKey_LeftArrow] = 0x25;   // VK_LEFT
  io->KeyMap[ImGuiKey_RightArrow] = 0x27;  // VK_RIGHT
  io->KeyMap[ImGuiKey_UpArrow] = 0x26;     // VK_UP
  io->KeyMap[ImGuiKey_DownArrow] = 0x28;   // VK_DOWN
  io->KeyMap[ImGuiKey_PageUp] = 0x21;      // VK_PRIOR
  io->KeyMap[ImGuiKey_PageDown] = 0x22;    // VK_NEXT
  io->KeyMap[ImGuiKey_Home] = 0x24;        // VK_HOME
  io->KeyMap[ImGuiKey_End] = 0x23;         // VK_END
  io->KeyMap[ImGuiKey_Delete] = 0x2E;      // VK_DELETE
  io->KeyMap[ImGuiKey_Backspace] = 0x08;   // VK_BACK
  io->KeyMap[ImGuiKey_Enter] = 0x0D;       // VK_RETURN
  io->KeyMap[ImGuiKey_Escape] = 0x1B;      // VK_ESCAPE
  io->KeyMap[ImGuiKey_A] = 'A';
  io->KeyMap[ImGuiKey_C] = 'C';
  io->KeyMap[ImGuiKey_V] = 'V';
  io->KeyMap[ImGuiKey_X] = 'X';
  io->KeyMap[ImGuiKey_Y] = 'Y';
  io->KeyMap[ImGuiKey_Z] = 'Z';

  // Configure our default style.
  ImGuiStyle& style = ImGui::GetStyle();
  style.ScrollbarRounding = 0;
  style.WindowRounding = 0;

  return true;
}

bool ImGuiOverlay::InitializeFont(ImGuiIO* io) {
  // Setup the font atlas that imgui will use to stash characters.
  font_atlas_ = make_unique<ImFontAtlas>();
  io->Fonts = font_atlas_.get();

  // Add ProggyTiny font, which covers basic latin characters. That's enough for
  // our debugging UI.
  // If we want to render strings from any language we'll need to add a system
  // TTF with tons of characters.
  ImFontConfig font_config;
  font_config.OversampleH = font_config.OversampleV = 1;
  font_config.PixelSnapH = true;
  static const ImWchar font_glyph_ranges[] = {
      0x0020, 0x00FF,  // Basic Latin + Latin Supplement.
      0,               // End of list.
  };
  io->Fonts->AddFontFromMemoryCompressedBase85TTF(
      proggy_tiny_compressed_data_base85, 10.0f, &font_config,
      font_glyph_ranges);

  // Extract atlas data as a 1 byte per pixel texture.
  // Pixel data is owned by imgui so no need to free.
  unsigned char* pixel_data = nullptr;
  int width = 0;
  int height = 0;
  io->Fonts->GetTexDataAsRGBA32(&pixel_data, &width, &height);

  // Allocate font atlas image.
  Image::CreateParams image_params;
  image_params.format = gfx::PixelFormats::kR8G8B8A8UNorm;
  image_params.tiling_mode = Image::TilingMode::kLinear;
  image_params.size = {width, height};
  image_params.initial_layout = Image::Layout::kPreinitialized;
  ref_ptr<Image> font_image;
  auto allocation_result = memory_heap_->AllocateImage(
      image_params, Image::Usage::kSampled, &font_image);
  switch (allocation_result) {
    case MemoryHeap::AllocationResult::kSuccess:
      break;
    default:
      LOG(ERROR) << "Failed to allocate font atlas image";
      return false;
  }

  // Upload initial font altas data.
  // TODO(benvanik): is invalidation possible?
  // TODO(benvanik): probably worth transitioning to optimal.
  if (!font_image->WriteData(font_image->entire_range(), pixel_data,
                             static_cast<size_t>(width * height * 4))) {
    LOG(ERROR) << "Failed to write data into font image";
    return false;
  }

  // Create simple view into the image.
  font_image_view_ = font_image->CreateView();
  if (!font_image_view_) {
    LOG(ERROR) << "Failed to create font atlas image view";
    return false;
  }

  // Let imgui know of the font texture so it'll use it when drawing.
  io->Fonts->TexID = reinterpret_cast<ImTextureID>(font_image_view_.get());

  return true;
}

bool ImGuiOverlay::InitializePipeline() {
  // Allocate common samplers.
  Sampler::Params sampler_params;
  nearest_sampler_ = context_->CreateSampler(sampler_params);
  if (!nearest_sampler_) {
    LOG(ERROR) << "Failed to create nearest sampler";
    return false;
  }
  sampler_params.min_filter = Sampler::Filter::kLinear;
  sampler_params.mag_filter = Sampler::Filter::kLinear;
  linear_sampler_ = context_->CreateSampler(sampler_params);
  if (!linear_sampler_) {
    LOG(ERROR) << "Failed to create linear sampler";
    return false;
  }

  // Allocate the buffers we'll use to stash geometry from imgui.
  auto allocation_result = memory_heap_->AllocateBuffer(
      kMaxVertexCount * sizeof(ImDrawVert), Buffer::Usage::kVertexBuffer,
      &vertex_buffer_);
  switch (allocation_result) {
    case MemoryHeap::AllocationResult::kSuccess:
      break;
    default:
      LOG(ERROR) << "Failed to allocate geometry vertex buffer";
      return false;
  }
  allocation_result =
      memory_heap_->AllocateBuffer(kMaxIndexCount * sizeof(ImDrawIdx),
                                   Buffer::Usage::kIndexBuffer, &index_buffer_);
  switch (allocation_result) {
    case MemoryHeap::AllocationResult::kSuccess:
      break;
    default:
      LOG(ERROR) << "Failed to allocate geometry index buffer";
      return false;
  }

  // Create render pass.
  RenderPass::AttachmentDescription color_attachment;
  color_attachment.format = gfx::PixelFormats::kB8G8R8A8UNorm;
  color_attachment.load_op = RenderPass::LoadOp::kLoad;
  color_attachment.store_op = RenderPass::StoreOp::kStore;
  color_attachment.initial_layout = Image::Layout::kColorAttachmentOptimal;
  color_attachment.final_layout = Image::Layout::kPresentSource;
  RenderPass::SubpassDescription subpass;
  subpass.color_attachments.push_back(
      {0, Image::Layout::kColorAttachmentOptimal});
  render_pass_ = context_->CreateRenderPass({color_attachment}, {subpass}, {});
  if (!render_pass_) {
    LOG(ERROR) << "Unable to create render pass";
    return false;
  }

  // Prepare render state.
  RenderState render_state;
  render_state.vertex_input_state.vertex_bindings.push_back(
      {0, sizeof(ImDrawVert)});
  render_state.vertex_input_state.vertex_attributes.push_back(
      {0, 0, 0, gfx::VertexFormats::kX32Y32SFloat});
  render_state.vertex_input_state.vertex_attributes.push_back(
      {1, 0, sizeof(float) * 2, gfx::VertexFormats::kX32Y32SFloat});
  render_state.vertex_input_state.vertex_attributes.push_back(
      {2, 0, sizeof(float) * 4, gfx::VertexFormats::kX8Y8Z8W8UNorm});
  render_state.input_assembly_state.set_primitive_topology(
      gfx::PrimitiveTopology::kTriangleList);
  render_state.viewport_state.set_count(1);
  render_state.color_blend_state.attachments.resize(1);
  auto& blend_state = render_state.color_blend_state.attachments[0];
  blend_state.set_blend_enabled(true);
  blend_state.set_blend_op(gfx::BlendOp::kAdd);
  blend_state.set_src_blend_factor(gfx::BlendFactor::kSrcAlpha);
  blend_state.set_dst_blend_factor(gfx::BlendFactor::kOneMinusSrcAlpha);

  // Load the shader module binaries.
  auto vertex_shader_module = context_->CreateShaderModule(
      ShaderModule::DataFormat::kSpirV, shaders::kImGuiOverlayVertSpirV,
      sizeof(shaders::kImGuiOverlayVertSpirV));
  if (!vertex_shader_module) {
    LOG(ERROR) << "Unable to load vertex shader module";
    return false;
  }
  auto fragment_shader_module = context_->CreateShaderModule(
      ShaderModule::DataFormat::kSpirV, shaders::kImGuiOverlayFragSpirV,
      sizeof(shaders::kImGuiOverlayFragSpirV));
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
  resource_set_layout_ = context_->CreateResourceSetLayout({
      {0, ResourceSetLayout::BindingSlot::Type::kCombinedImageSampler},
  });
  render_pipeline_layout_ = context_->CreatePipelineLayout(
      {
          resource_set_layout_,
      },
      {
          {offsetof(PushConstants, proj_matrix),
           sizeof(PushConstants::proj_matrix)},
      });
  if (!render_pipeline_layout_) {
    LOG(ERROR) << "Unable to create pipeline layout";
    return false;
  }

  // Create the pipeline.
  render_pipeline_ =
      context_->CreateRenderPipeline(render_pipeline_layout_, render_pass_, 0,
                                     render_state, std::move(shader_stages));
  if (!render_pipeline_) {
    LOG(ERROR) << "Unable to create render pipeline";
    return false;
  }

  return true;
}

void ImGuiOverlay::BeginFrame(ref_ptr<gfx::Framebuffer> framebuffer) {
  // Set our context active. It'll remain active on this thread until the frame
  // end.
  DCHECK(ImGui::GetCurrentContext() == nullptr);
  ImGui::SetCurrentContext(imgui_context_);

  // Configure the imgui context for this frame.
  ImGuiIO* io = &ImGui::GetIO();
  io->DisplaySize = ImVec2(static_cast<float>(framebuffer->size().width),
                           static_cast<float>(framebuffer->size().height));
  // TODO(benvanik): set DPI scaling.
  io->DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

  // Calculate time delta between this frame and the last.
  std::chrono::microseconds now_micros =
      SystemClock::default_clock()->now_micros();
  io->DeltaTime =
      (now_micros.count() - last_frame_now_micros_.count()) / 1000000.0f;
  last_frame_now_micros_ = now_micros;

  // Start the frame. Any imgui operations will go into our context.
  ImGui::NewFrame();

  // Stash framebuffer for when we render.
  framebuffer_ = std::move(framebuffer);
}

void ImGuiOverlay::RenderDrawListsThunk(ImDrawData* data) {
  // NOTE: this is only ever called from within ImGui::Render so we know a
  //       context is valid.
  ImGuiIO* io = &ImGui::GetIO();
  DCHECK(io->UserData != nullptr);
  auto overlay = reinterpret_cast<ImGuiOverlay*>(io->UserData);
  overlay->RenderDrawLists(data);
}

void ImGuiOverlay::RenderDrawLists(ImDrawData* data) {
  ImGuiIO* io = &ImGui::GetIO();
  if (!data->Valid) {
    return;
  }

  // Scale commands when the framebuffer size does not match the display size
  // (such as when resizing or rendering on a high DPI display).
  data->ScaleClipRects(io->DisplayFramebufferScale);

  // Begin the render pass for all our drawing.
  auto rpe = command_buffer_->BeginRenderPass(render_pass_, framebuffer_, {});
  rpe->SetViewport({framebuffer_->size().width, framebuffer_->size().height});
  rpe->BindPipeline(render_pipeline_);

  // Prepare projection matrix push constant.
  const float ortho_projection[4][4] = {
      {2.0f / io->DisplaySize.x, 0.0f, 0.0f, 0.0f},
      {0.0f, 2.0f / -io->DisplaySize.y, 0.0f, 0.0f},
      {0.0f, 0.0f, -1.0f, 0.0f},
      {-1.0f, 1.0f, 0.0f, 1.0f},
  };
  rpe->PushConstants(render_pipeline_layout_, gfx::ShaderStageFlag::kVertex, 0,
                     ortho_projection, sizeof(ortho_projection));

  // Map buffers to store vertex/index data.
  // We'll fill the data as we walk the command list.
  // TODO(benvanik): grow as needed.
  if (data->TotalVtxCount > kMaxVertexCount) {
    LOG(WARNING) << "Exceeded max imgui vertex count: " << data->TotalVtxCount;
    return;
  }
  if (data->TotalIdxCount > kMaxIndexCount) {
    LOG(WARNING) << "Exceeded max imgui index count: " << data->TotalIdxCount;
    return;
  }
  auto vertex_mapping = vertex_buffer_->MapMemory<uint8_t>(
      gfx::MemoryAccess::kWriteDiscard, 0,
      data->TotalVtxCount * sizeof(ImDrawVert));
  if (!vertex_mapping) {
    return;
  }
  uint8_t* vertex_data = vertex_mapping.data();
  size_t vertex_data_offset = 0;
  auto index_mapping = index_buffer_->MapMemory<uint8_t>(
      gfx::MemoryAccess::kWriteDiscard, 0,
      data->TotalIdxCount * sizeof(ImDrawIdx));
  if (!index_mapping) {
    return;
  }
  uint8_t* index_data = index_mapping.data();
  size_t index_data_offset = 0;

  // Iterate over each command list, which contains its own vb/ib data.
  Rect2D previous_scissor_rect;
  for (int i = 0; i < data->CmdListsCount; ++i) {
    const ImDrawList* draw_list = data->CmdLists[i];

    // Bind the buffers at the current offset.
    rpe->BindVertexBuffers(0, {vertex_buffer_}, {vertex_data_offset});
    rpe->BindIndexBuffer(index_buffer_, index_data_offset,
                         gfx::IndexElementType::kUint16);

    // Upload data for the command list.
    std::memcpy(vertex_data + vertex_data_offset, draw_list->VtxBuffer.Data,
                sizeof(ImDrawVert) * draw_list->VtxBuffer.size());
    vertex_data_offset += sizeof(ImDrawVert) * draw_list->VtxBuffer.size();
    std::memcpy(index_data + index_data_offset, draw_list->IdxBuffer.Data,
                sizeof(ImDrawIdx) * draw_list->IdxBuffer.size());
    index_data_offset += sizeof(ImDrawIdx) * draw_list->IdxBuffer.size();

    // Iterate commands using the data in the list.
    int index_offset = 0;
    for (const ImDrawCmd& draw_cmd : draw_list->CmdBuffer) {
      if (draw_cmd.UserCallback) {
        draw_cmd.UserCallback(draw_list, &draw_cmd);
      } else {
        // Scissor - often full screen.
        Rect2D scissor_rect{
            static_cast<int>(draw_cmd.ClipRect.x),
            static_cast<int>(io->DisplaySize.y - draw_cmd.ClipRect.w),
            static_cast<int>(draw_cmd.ClipRect.z - draw_cmd.ClipRect.x),
            static_cast<int>(draw_cmd.ClipRect.w - draw_cmd.ClipRect.y)};
        if (scissor_rect != previous_scissor_rect) {
          rpe->SetScissor(scissor_rect);
          previous_scissor_rect = scissor_rect;
        }

        // Bind the texture resource used (if any).
        if (draw_cmd.TextureId) {
          // TODO(benvanik): cache resources (especially for font).
          // TODO(benvanik): combined image sampler resource to simplify?
          ref_ptr<gfx::ImageView> image_view{
              reinterpret_cast<gfx::ImageView*>(draw_cmd.TextureId)};
          auto resource_set = context_->CreateResourceSet(
              resource_set_layout_,
              {
                  {image_view, Image::Layout::kGeneral, nearest_sampler_},
              });
          if (!resource_set) {
            LOG(ERROR) << "Unable to create resource set";
            return;
          }
          rpe->BindResourceSet(0, resource_set);
        } else {
          // TODO(benvanik): a default set with a white pixel texture?
          rpe->BindResourceSet(0, nullptr);
        }

        // Issue draw for the buffer range.
        rpe->DrawIndexed(draw_cmd.ElemCount, 1, index_offset, 0, 0);
      }
      index_offset += draw_cmd.ElemCount;
    }
  }

  command_buffer_->EndRenderPass(std::move(rpe));

  // Flush and unmap the buffers before we use them.
  vertex_mapping.reset();
  index_mapping.reset();
}

ref_ptr<gfx::QueueFence> ImGuiOverlay::EndFrame(
    ref_ptr<gfx::QueueFence> wait_fence) {
  DCHECK_EQ(ImGui::GetCurrentContext(), imgui_context_);

  // Prepare the command buffer that RenderDrawLists will write into.
  command_buffer_ = context_->CreateCommandBuffer();

  // Flush all imgui commands. This will call our RenderDrawLists fn.
  ImGui::Render();

  // TODO(benvanik): get outputs and redirect to control/etc.
  // io->WantCaptureMouse
  // io->WantCaptureKeyboard
  // io->WantTextInput

  // Clear the current context until the next frame.
  framebuffer_.reset();
  ImGui::SetCurrentContext(nullptr);

  // Submit the command buffer.
  ref_ptr<gfx::QueueFence> signal_fence = context_->CreateQueueFence();
  auto submit_result = context_->Submit(
      std::move(wait_fence), std::move(command_buffer_), signal_fence);
  command_buffer_.reset();
  switch (submit_result) {
    case gfx::Context::SubmitResult::kSuccess:
      break;
    case gfx::Context::SubmitResult::kCommandBufferReused:
      LOG(ERROR)
          << "Unable to submit imgui render commands: command buffer reused";
      return nullptr;
    case gfx::Context::SubmitResult::kDeviceLost:
      LOG(ERROR) << "Unable to submit imgui render commands: device lost";
      return nullptr;
  }

  return signal_fence;
}

namespace {

void PopulateEventData(const KeyboardEvent& ev, bool is_down, ImGuiIO* io) {
  if (ev.virtual_key() == VirtualKey::kNone) {
    // Not a key that imgui will recognize.
    return;
  }
  io->KeysDown[static_cast<int>(ev.virtual_key())] = is_down;

  io->KeyCtrl = any(ev.modifier_key_mask() & ModifierKey::kCtrl);
  io->KeyShift = any(ev.modifier_key_mask() & ModifierKey::kShift);
  io->KeyAlt = any(ev.modifier_key_mask() & ModifierKey::kAlt);
  io->KeySuper = any(ev.modifier_key_mask() & ModifierKey::kSuper);
}

void PopulateEventData(const MouseEvent& ev, ImGuiIO* io) {
  io->MousePos = ImVec2(static_cast<float>(ev.control_offset_px().x),
                        static_cast<float>(ev.control_offset_px().y));

  // L, R, M, X1, X2
  io->MouseDown[0] = any(ev.pressed_button_mask() & MouseButton::kLeftButton);
  io->MouseDown[1] = any(ev.pressed_button_mask() & MouseButton::kRightButton);
  io->MouseDown[2] = any(ev.pressed_button_mask() & MouseButton::kMiddleButton);
  io->MouseDown[3] = any(ev.pressed_button_mask() & MouseButton::kButton4);
  io->MouseDown[4] = any(ev.pressed_button_mask() & MouseButton::kButton5);

  // TODO(benvanik): normalize range across platforms.
  io->MouseWheel += static_cast<float>(ev.wheel_delta()) / 120.0f;

  io->KeyCtrl = any(ev.modifier_key_mask() & ModifierKey::kCtrl);
  io->KeyShift = any(ev.modifier_key_mask() & ModifierKey::kShift);
  io->KeyAlt = any(ev.modifier_key_mask() & ModifierKey::kAlt);
  io->KeySuper = any(ev.modifier_key_mask() & ModifierKey::kSuper);
}

}  // namespace

void ImGuiOverlay::OnKeyDown(ref_ptr<Control> target, const KeyboardEvent& ev) {
  ImGuiLock imgui(imgui_context_);
  PopulateEventData(ev, true, imgui.io());
}

void ImGuiOverlay::OnKeyUp(ref_ptr<Control> target, const KeyboardEvent& ev) {
  ImGuiLock imgui(imgui_context_);
  PopulateEventData(ev, false, imgui.io());
}

void ImGuiOverlay::OnKeyPress(ref_ptr<Control> target,
                              const KeyboardEvent& ev) {
  ImGuiLock imgui(imgui_context_);
  if (ev.key_code() > 0 && ev.key_code() < 0x10000) {
    imgui.io()->AddInputCharacter(ev.key_code());
  }
}

void ImGuiOverlay::OnMouseDown(ref_ptr<Control> target, const MouseEvent& ev) {
  ImGuiLock imgui(imgui_context_);
  PopulateEventData(ev, imgui.io());
}

void ImGuiOverlay::OnMouseUp(ref_ptr<Control> target, const MouseEvent& ev) {
  ImGuiLock imgui(imgui_context_);
  PopulateEventData(ev, imgui.io());
}

void ImGuiOverlay::OnMouseOut(ref_ptr<Control> target, const MouseEvent& ev) {
  // TODO(benvanik): reset all mouse state?
}

void ImGuiOverlay::OnMouseMove(ref_ptr<Control> target, const MouseEvent& ev) {
  ImGuiLock imgui(imgui_context_);
  PopulateEventData(ev, imgui.io());
}

void ImGuiOverlay::OnMouseWheel(ref_ptr<Control> target, const MouseEvent& ev) {
  ImGuiLock imgui(imgui_context_);
  PopulateEventData(ev, imgui.io());
}

}  // namespace ui
}  // namespace xrtl
