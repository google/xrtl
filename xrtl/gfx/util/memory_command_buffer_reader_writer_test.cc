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

#include "xrtl/gfx/util/memory_command_buffer_reader.h"
#include "xrtl/gfx/util/memory_command_buffer_writer.h"

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace gfx {
namespace util {
namespace {

// Tests empty packet streams.
TEST(MemoryCommandBufferReaderWriterTest, Empty) {
  Arena arena(kMaxCommandSize);

  MemoryCommandBufferWriter writer(&arena);
  EXPECT_EQ(nullptr, writer.first_packet());

  MemoryCommandBufferReader reader(writer.first_packet());
  EXPECT_TRUE(reader.empty());
  EXPECT_EQ(nullptr, reader.PeekCommandHeader());
}

// Tests round-tripping simple commands.
TEST(MemoryCommandBufferReaderWriterTest, Commands) {
  Arena arena(kMaxCommandSize);

  // Write two commands to the buffer.
  MemoryCommandBufferWriter writer(&arena);
  EXPECT_EQ(nullptr, writer.first_packet());
  writer.WriteCommand(CommandType::kSetLineWidth, SetLineWidthCommand{16.0f});
  writer.WriteCommand(CommandType::kSetLineWidth, SetLineWidthCommand{32.0f});
  EXPECT_NE(nullptr, writer.first_packet());

  // Read back both commands.
  MemoryCommandBufferReader reader(writer.first_packet());
  EXPECT_FALSE(reader.empty());
  auto command_header = reader.PeekCommandHeader();
  EXPECT_NE(nullptr, command_header);
  EXPECT_EQ(CommandType::kSetLineWidth, command_header->command_type);
  auto command = reader.ReadCommand<SetLineWidthCommand>(command_header);
  EXPECT_EQ(16.0f, command.line_width);
  EXPECT_FALSE(reader.empty());
  command_header = reader.PeekCommandHeader();
  EXPECT_NE(nullptr, command_header);
  EXPECT_EQ(CommandType::kSetLineWidth, command_header->command_type);
  command = reader.ReadCommand<SetLineWidthCommand>(command_header);
  EXPECT_EQ(32.0f, command.line_width);

  // Should have consumed all data.
  EXPECT_TRUE(reader.empty());
  EXPECT_EQ(nullptr, reader.PeekCommandHeader());
}

// Tests reading and writing raw data.
TEST(MemoryCommandBufferReaderWriterTest, RawData) {
  Arena arena(kMaxCommandSize);

  int source_array_1[5] = {0, 1, 2, 3, 4};
  int source_array_2[1] = {5};

  // Write two arrays and an empty array to the buffer as data.
  MemoryCommandBufferWriter writer(&arena);
  EXPECT_EQ(nullptr, writer.first_packet());
  writer.WriteData(source_array_1, sizeof(source_array_1));
  writer.WriteData(source_array_1, 0);
  writer.WriteData(source_array_2, sizeof(source_array_2));
  EXPECT_NE(nullptr, writer.first_packet());

  // Read back the data.
  MemoryCommandBufferReader reader(writer.first_packet());
  EXPECT_FALSE(reader.empty());
  // source_array_1.
  auto read_array_1 = reader.ReadData(sizeof(source_array_1));
  EXPECT_EQ(0,
            std::memcmp(read_array_1, source_array_1, sizeof(source_array_1)));
  // Empty array.
  EXPECT_EQ(nullptr, reader.ReadData(0));
  // source_array_2.
  auto read_array_2 = reader.ReadData(sizeof(source_array_2));
  EXPECT_EQ(0,
            std::memcmp(read_array_2, source_array_2, sizeof(source_array_2)));

  // Should have consumed all data.
  EXPECT_TRUE(reader.empty());
  EXPECT_EQ(nullptr, reader.ReadData(1));
}

// Tests reading and writing arrays of primitives.
TEST(MemoryCommandBufferReaderWriterTest, PrimitiveArrays) {
  Arena arena(kMaxCommandSize);

  int source_array_1[5] = {0, 1, 2, 3, 4};
  int source_array_2[1] = {5};

  // Write two arrays and an empty array to the buffer.
  MemoryCommandBufferWriter writer(&arena);
  EXPECT_EQ(nullptr, writer.first_packet());
  writer.WriteArray<int>(source_array_1);
  writer.WriteArray<int>(std::vector<int>{});
  writer.WriteArray<int>(source_array_2);
  EXPECT_NE(nullptr, writer.first_packet());

  // Read back the arrays.
  MemoryCommandBufferReader reader(writer.first_packet());
  EXPECT_FALSE(reader.empty());
  // source_array_1.
  auto read_array_1 = reader.ReadArray<int>(5);
  EXPECT_EQ(5, read_array_1.size());
  EXPECT_EQ(0, std::memcmp(read_array_1.data(), source_array_1,
                           sizeof(source_array_1)));
  // Empty array.
  auto empty_array = reader.ReadArray<int>(0);
  EXPECT_EQ(0, empty_array.size());
  // source_array_2.
  auto read_array_2 = reader.ReadArray<int>(1);
  EXPECT_EQ(1, read_array_2.size());
  EXPECT_EQ(0, std::memcmp(read_array_2.data(), source_array_2,
                           sizeof(source_array_2)));

  // Should have consumed all data.
  EXPECT_TRUE(reader.empty());
}

struct SimpleObject : public RefObject<SimpleObject> {
  SimpleObject() { ++alloc_count; }
  ~SimpleObject() { --alloc_count; }
  static int alloc_count;
};
int SimpleObject::alloc_count = 0;

// Tests reading and writing arrays of ref-counted objects.
TEST(MemoryCommandBufferReaderWriterTest, RefPtrArrays) {
  Arena arena(kMaxCommandSize);

  std::vector<ref_ptr<SimpleObject>> source_objects;
  source_objects.push_back(make_ref<SimpleObject>());
  source_objects.push_back(make_ref<SimpleObject>());
  source_objects.push_back(make_ref<SimpleObject>());
  EXPECT_EQ(3, SimpleObject::alloc_count);

  // Write the array of objects to the buffer.
  MemoryCommandBufferWriter writer(&arena);
  EXPECT_EQ(nullptr, writer.first_packet());
  writer.WriteArray<SimpleObject>(source_objects);
  EXPECT_NE(nullptr, writer.first_packet());

  // Read back the object array.
  MemoryCommandBufferReader reader(writer.first_packet());
  EXPECT_FALSE(reader.empty());
  auto read_array = reader.ReadRefPtrArray<SimpleObject>(3);
  EXPECT_EQ(3, read_array.size());
  EXPECT_EQ(read_array[0].get(), source_objects[0].get());
  EXPECT_EQ(read_array[1].get(), source_objects[1].get());
  EXPECT_EQ(read_array[2].get(), source_objects[2].get());
  EXPECT_EQ(3, SimpleObject::alloc_count);

  // Should have consumed all data.
  EXPECT_TRUE(reader.empty());

  // Drop the original array. As the read_array is just a view into it all
  // objects should be deallocated.
  source_objects.clear();
  EXPECT_EQ(0, SimpleObject::alloc_count);
}

// Tests commands that span multiple packets.
TEST(MemoryCommandBufferReaderWriterTest, MultiplePackets) {
  Arena arena(64);

  struct DummyCommand {
    uint8_t data[44];
  };
  DummyCommand command_1;
  std::memset(command_1.data, 0xCC, sizeof(command_1.data));
  DummyCommand command_2;
  std::memset(command_2.data, 0xDD, sizeof(command_2.data));

  // Write two commands that should each take up most of a block.
  MemoryCommandBufferWriter writer(&arena);
  EXPECT_EQ(nullptr, writer.first_packet());
  writer.WriteCommand(CommandType::kNextSubpass, command_1);
  EXPECT_GT(100, arena.block_bytes_allocated());
  writer.WriteCommand(CommandType::kSetLineWidth, command_2);
  EXPECT_LT(100, arena.block_bytes_allocated());

  // Read back the commands. They should span two packets now but still be read.
  MemoryCommandBufferReader reader(writer.first_packet());
  EXPECT_FALSE(reader.empty());
  // command_1.
  auto command_header = reader.PeekCommandHeader();
  EXPECT_NE(nullptr, command_header);
  EXPECT_EQ(CommandType::kNextSubpass, command_header->command_type);
  auto read_command_1 = reader.ReadCommand<DummyCommand>(command_header);
  EXPECT_EQ(0, std::memcmp(read_command_1.data, command_1.data,
                           sizeof(command_1.data)));
  // command_2.
  command_header = reader.PeekCommandHeader();
  EXPECT_NE(nullptr, command_header);
  EXPECT_EQ(CommandType::kSetLineWidth, command_header->command_type);
  auto read_command_2 = reader.ReadCommand<DummyCommand>(command_header);
  EXPECT_EQ(0, std::memcmp(read_command_2.data, command_2.data,
                           sizeof(command_2.data)));

  // Should have consumed all data.
  EXPECT_TRUE(reader.empty());
}

// Tests commands with associated data that span multiple packets.
TEST(MemoryCommandBufferReaderWriterTest, CommandDataSplitPackets) {
  Arena arena(64);

  struct DummyCommand {
    uint8_t data[44];
  };
  DummyCommand command_1;
  std::memset(command_1.data, 0xCC, sizeof(command_1.data));
  uint8_t associated_data[44];
  std::memset(associated_data, 0xDD, sizeof(associated_data));

  // Write the command and data that should each take up most of a block.
  MemoryCommandBufferWriter writer(&arena);
  EXPECT_EQ(nullptr, writer.first_packet());
  writer.WriteCommand(CommandType::kNextSubpass, command_1);
  EXPECT_GT(100, arena.block_bytes_allocated());
  writer.WriteData(associated_data, sizeof(associated_data));
  EXPECT_LT(100, arena.block_bytes_allocated());

  // Read back the command. It's data should be in the second packet.
  MemoryCommandBufferReader reader(writer.first_packet());
  EXPECT_FALSE(reader.empty());
  // command_1.
  auto command_header = reader.PeekCommandHeader();
  EXPECT_NE(nullptr, command_header);
  EXPECT_EQ(CommandType::kNextSubpass, command_header->command_type);
  auto read_command_1 = reader.ReadCommand<DummyCommand>(command_header);
  EXPECT_EQ(0, std::memcmp(read_command_1.data, command_1.data,
                           sizeof(command_1.data)));
  // associated_data.
  auto read_associated_data = reader.ReadData(sizeof(associated_data));
  EXPECT_EQ(0, std::memcmp(read_associated_data, associated_data,
                           sizeof(associated_data)));

  // Should have consumed all data.
  EXPECT_TRUE(reader.empty());
}

}  // namespace
}  // namespace util
}  // namespace gfx
}  // namespace xrtl
