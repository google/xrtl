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

#include "xrtl/gfx/util/memory_command_buffer.h"

#include "xrtl/gfx/testing/partial_command_encoder.h"
#include "xrtl/gfx/util/memory_command_decoder.h"
#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace gfx {
namespace util {
namespace {

class MyCommandFence : public CommandFence {
 public:
  MyCommandFence() { ++alloc_count; }
  ~MyCommandFence() override { --alloc_count; }
  static int alloc_count;
};
int MyCommandFence::alloc_count = 0;

using PipelineBarrierCallArgs =
    std::tuple<PipelineStageFlag, PipelineStageFlag, PipelineDependencyFlag>;

class MyTransferCommandEncoder
    : public gfx::testing::PartialTransferCommandEncoder {
 public:
  explicit MyTransferCommandEncoder(CommandBuffer* command_buffer)
      : gfx::testing::PartialTransferCommandEncoder(command_buffer) {}

  void PipelineBarrier(PipelineStageFlag source_stage_mask,
                       PipelineStageFlag target_stage_mask,
                       PipelineDependencyFlag dependency_flags) override {
    pipeline_barrier_calls.push_back(PipelineBarrierCallArgs{
        source_stage_mask, target_stage_mask, dependency_flags});
  }

  std::vector<PipelineBarrierCallArgs> pipeline_barrier_calls;
};

using WaitFencesCallArgs = ArrayView<ref_ptr<CommandFence>>;

class MyComputeCommandEncoder
    : public gfx::testing::PartialComputeCommandEncoder {
 public:
  explicit MyComputeCommandEncoder(CommandBuffer* command_buffer)
      : gfx::testing::PartialComputeCommandEncoder(command_buffer) {}

  void WaitFences(ArrayView<ref_ptr<CommandFence>> fences) override {
    wait_fences_calls.push_back(WaitFencesCallArgs{fences});
  }

  std::vector<WaitFencesCallArgs> wait_fences_calls;
};

class MyCommandBuffer : public CommandBuffer {
 public:
  TransferCommandEncoderPtr BeginTransferCommands() override {
    queue_mask_ |= OperationQueueMask::kTransfer;
    return {&transfer_encoder, [](TransferCommandEncoder*) {}};
  }

  void EndTransferCommands(TransferCommandEncoderPtr encoder) override {
    EXPECT_EQ(encoder.get(), &transfer_encoder);
  }

  ComputeCommandEncoderPtr BeginComputeCommands() override {
    queue_mask_ |= OperationQueueMask::kCompute;
    return {&compute_encoder, [](ComputeCommandEncoder*) {}};
  }
  void EndComputeCommands(ComputeCommandEncoderPtr encoder) override {
    EXPECT_EQ(encoder.get(), &compute_encoder);
  }

  RenderCommandEncoderPtr BeginRenderCommands() override {
    return {nullptr, nullptr};
  }
  void EndRenderCommands(RenderCommandEncoderPtr encoder) override {}

  RenderPassCommandEncoderPtr BeginRenderPass(
      ref_ptr<RenderPass> render_pass, ref_ptr<Framebuffer> framebuffer,
      ArrayView<ClearColor> clear_colors) override {
    return {nullptr, nullptr};
  }
  void EndRenderPass(RenderPassCommandEncoderPtr encoder) override {}

  MyTransferCommandEncoder transfer_encoder{this};
  MyComputeCommandEncoder compute_encoder{this};
};

// Tests an empty command buffer (nothing recorded).
TEST(MemoryCommandBufferTest, Empty) {
  MemoryCommandBuffer command_buffer;

  // Nothing recorded should have no queue pinned and no commands.
  EXPECT_EQ(OperationQueueMask::kNone, command_buffer.queue_mask());
  auto reader = command_buffer.GetReader();
  EXPECT_TRUE(reader.empty());

  // A no-op encoder usage should mark the queue.
  // No real commands get encoded but the pseudo commands will be there so
  // the reader won't be empty.
  command_buffer.EndTransferCommands(command_buffer.BeginTransferCommands());
  EXPECT_EQ(OperationQueueMask::kTransfer, command_buffer.queue_mask());
  reader = command_buffer.GetReader();
  EXPECT_FALSE(reader.empty());

  // Test operation queue masking.
  command_buffer.EndComputeCommands(command_buffer.BeginComputeCommands());
  EXPECT_EQ(OperationQueueMask::kCompute | OperationQueueMask::kTransfer,
            command_buffer.queue_mask());
  reader = command_buffer.GetReader();
  EXPECT_FALSE(reader.empty());
}

// Tests encoding simple commands and reading them back.
TEST(MemoryCommandBufferTest, SimpleCommands) {
  MemoryCommandBuffer command_buffer;

  EXPECT_EQ(OperationQueueMask::kNone, command_buffer.queue_mask());
  auto reader = command_buffer.GetReader();
  EXPECT_TRUE(reader.empty());

  // Encode two commands.
  auto encoder = command_buffer.BeginTransferCommands();
  EXPECT_EQ(&command_buffer, encoder->command_buffer());
  encoder->PipelineBarrier(PipelineStageFlag::kTopOfPipe,
                           PipelineStageFlag::kBottomOfPipe,
                           PipelineDependencyFlag::kFramebufferLocal);
  encoder->PipelineBarrier(PipelineStageFlag::kBottomOfPipe,
                           PipelineStageFlag::kTopOfPipe,
                           PipelineDependencyFlag::kNone);
  command_buffer.EndTransferCommands(std::move(encoder));

  EXPECT_EQ(OperationQueueMask::kTransfer, command_buffer.queue_mask());
  reader = command_buffer.GetReader();
  EXPECT_FALSE(reader.empty());

  // Create a new command buffer to replay the commands against.
  MyCommandBuffer my_command_buffer;

  // Decode commands against the command buffer.
  MemoryCommandDecoder::Decode(&reader, &my_command_buffer);

  // Ensure the commands made it.
  EXPECT_EQ(OperationQueueMask::kTransfer, my_command_buffer.queue_mask());
  const auto& pipeline_barrier_calls =
      my_command_buffer.transfer_encoder.pipeline_barrier_calls;
  EXPECT_EQ(2, pipeline_barrier_calls.size());
  EXPECT_EQ(PipelineBarrierCallArgs(PipelineStageFlag::kTopOfPipe,
                                    PipelineStageFlag::kBottomOfPipe,
                                    PipelineDependencyFlag::kFramebufferLocal),
            pipeline_barrier_calls[0]);
  EXPECT_EQ(PipelineBarrierCallArgs(PipelineStageFlag::kBottomOfPipe,
                                    PipelineStageFlag::kTopOfPipe,
                                    PipelineDependencyFlag::kNone),
            pipeline_barrier_calls[1]);
}

// Tests encoding a command that requires ref counted objects.
TEST(MemoryCommandBufferTest, RefCountedObjects) {
  auto command_buffer = absl::make_unique<MemoryCommandBuffer>();

  // Allocate dummy fences.
  EXPECT_EQ(0, MyCommandFence::alloc_count);
  auto fence_1 = make_ref<MyCommandFence>();
  auto fence_2 = make_ref<MyCommandFence>();
  EXPECT_EQ(2, MyCommandFence::alloc_count);

  // Encode command.
  auto encoder = command_buffer->BeginComputeCommands();
  EXPECT_EQ(command_buffer.get(), encoder->command_buffer());
  encoder->WaitFences({fence_1, fence_2});
  command_buffer->EndComputeCommands(std::move(encoder));

  // Drop fences here so the command buffer is the only thing holding on to
  // them.
  auto fence_1_ptr = fence_1.get();
  fence_1.reset();
  auto fence_2_ptr = fence_2.get();
  fence_2.reset();

  MyCommandBuffer my_command_buffer;

  // Decode commands.
  auto reader = command_buffer->GetReader();
  EXPECT_FALSE(reader.empty());
  MemoryCommandDecoder::Decode(&reader, &my_command_buffer);

  // Ensure the fences made it ok.
  EXPECT_EQ(OperationQueueMask::kCompute, my_command_buffer.queue_mask());
  const auto& wait_fences_calls =
      my_command_buffer.compute_encoder.wait_fences_calls;
  EXPECT_EQ(1, wait_fences_calls.size());
  EXPECT_EQ(2, wait_fences_calls[0].size());
  EXPECT_EQ(fence_1_ptr, wait_fences_calls[0][0]);
  EXPECT_EQ(fence_2_ptr, wait_fences_calls[0][1]);

  // Drop the command buffer; it should deallocate the fences.
  EXPECT_EQ(2, MyCommandFence::alloc_count);
  command_buffer.reset();
  EXPECT_EQ(0, MyCommandFence::alloc_count);
}

}  // namespace
}  // namespace util
}  // namespace gfx
}  // namespace xrtl
