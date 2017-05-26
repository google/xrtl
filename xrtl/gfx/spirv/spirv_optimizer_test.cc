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

#include "xrtl/gfx/spirv/spirv_optimizer.h"

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace gfx {
namespace spirv {
namespace {

/*
$ glslangValidator --vn kValidVertexShader -q -V shader.vert

#version 310 es
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_color;
layout(location = 0) out vec4 v_color;
void main() {
  gl_Position = vec4(a_position.xyz, 1.0);
  v_color = vec4(a_color, 1.0);
}
*/
const uint32_t kValidVertexShader[] = {
    0x07230203, 0x00010000, 0x00080001, 0x00000021, 0x00000000, 0x00020011,
    0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0009000f, 0x00000000,
    0x00000004, 0x6e69616d, 0x00000000, 0x0000000a, 0x0000000e, 0x00000019,
    0x0000001b, 0x00030003, 0x00000001, 0x00000136, 0x00040005, 0x00000004,
    0x6e69616d, 0x00000000, 0x00060005, 0x00000008, 0x505f6c67, 0x65567265,
    0x78657472, 0x00000000, 0x00060006, 0x00000008, 0x00000000, 0x505f6c67,
    0x7469736f, 0x006e6f69, 0x00070006, 0x00000008, 0x00000001, 0x505f6c67,
    0x746e696f, 0x657a6953, 0x00000000, 0x00030005, 0x0000000a, 0x00000000,
    0x00050005, 0x0000000e, 0x6f705f61, 0x69746973, 0x00006e6f, 0x00040005,
    0x00000019, 0x6f635f76, 0x00726f6c, 0x00040005, 0x0000001b, 0x6f635f61,
    0x00726f6c, 0x00050048, 0x00000008, 0x00000000, 0x0000000b, 0x00000000,
    0x00050048, 0x00000008, 0x00000001, 0x0000000b, 0x00000001, 0x00030047,
    0x00000008, 0x00000002, 0x00040047, 0x0000000e, 0x0000001e, 0x00000000,
    0x00040047, 0x00000019, 0x0000001e, 0x00000000, 0x00040047, 0x0000001b,
    0x0000001e, 0x00000001, 0x00020013, 0x00000002, 0x00030021, 0x00000003,
    0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007,
    0x00000006, 0x00000004, 0x0004001e, 0x00000008, 0x00000007, 0x00000006,
    0x00040020, 0x00000009, 0x00000003, 0x00000008, 0x0004003b, 0x00000009,
    0x0000000a, 0x00000003, 0x00040015, 0x0000000b, 0x00000020, 0x00000001,
    0x0004002b, 0x0000000b, 0x0000000c, 0x00000000, 0x00040020, 0x0000000d,
    0x00000001, 0x00000007, 0x0004003b, 0x0000000d, 0x0000000e, 0x00000001,
    0x00040017, 0x0000000f, 0x00000006, 0x00000003, 0x0004002b, 0x00000006,
    0x00000012, 0x3f800000, 0x00040020, 0x00000017, 0x00000003, 0x00000007,
    0x0004003b, 0x00000017, 0x00000019, 0x00000003, 0x00040020, 0x0000001a,
    0x00000001, 0x0000000f, 0x0004003b, 0x0000001a, 0x0000001b, 0x00000001,
    0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8,
    0x00000005, 0x0004003d, 0x00000007, 0x00000010, 0x0000000e, 0x0008004f,
    0x0000000f, 0x00000011, 0x00000010, 0x00000010, 0x00000000, 0x00000001,
    0x00000002, 0x00050051, 0x00000006, 0x00000013, 0x00000011, 0x00000000,
    0x00050051, 0x00000006, 0x00000014, 0x00000011, 0x00000001, 0x00050051,
    0x00000006, 0x00000015, 0x00000011, 0x00000002, 0x00070050, 0x00000007,
    0x00000016, 0x00000013, 0x00000014, 0x00000015, 0x00000012, 0x00050041,
    0x00000017, 0x00000018, 0x0000000a, 0x0000000c, 0x0003003e, 0x00000018,
    0x00000016, 0x0004003d, 0x0000000f, 0x0000001c, 0x0000001b, 0x00050051,
    0x00000006, 0x0000001d, 0x0000001c, 0x00000000, 0x00050051, 0x00000006,
    0x0000001e, 0x0000001c, 0x00000001, 0x00050051, 0x00000006, 0x0000001f,
    0x0000001c, 0x00000002, 0x00070050, 0x00000007, 0x00000020, 0x0000001d,
    0x0000001e, 0x0000001f, 0x00000012, 0x0003003e, 0x00000019, 0x00000020,
    0x000100fd, 0x00010038};

/*
$ glslangValidator --vn kValidFragmentShader -q -V shader.frag

#version 310 es
precision highp float;
layout(location = 0) in vec4 v_color;
layout(location = 0) out vec4 out_color;
void main() {
  out_color = v_color;
}
*/
const uint32_t kValidFragmentShader[] = {
    0x07230203, 0x00010000, 0x00080001, 0x0000000d, 0x00000000, 0x00020011,
    0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000004,
    0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000b, 0x00030010,
    0x00000004, 0x00000007, 0x00030003, 0x00000001, 0x00000136, 0x00040005,
    0x00000004, 0x6e69616d, 0x00000000, 0x00050005, 0x00000009, 0x5f74756f,
    0x6f6c6f63, 0x00000072, 0x00040005, 0x0000000b, 0x6f635f76, 0x00726f6c,
    0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047, 0x0000000b,
    0x0000001e, 0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003,
    0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007,
    0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003, 0x00000007,
    0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040020, 0x0000000a,
    0x00000001, 0x00000007, 0x0004003b, 0x0000000a, 0x0000000b, 0x00000001,
    0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8,
    0x00000005, 0x0004003d, 0x00000007, 0x0000000c, 0x0000000b, 0x0003003e,
    0x00000009, 0x0000000c, 0x000100fd, 0x00010038};

// Same as kValidFragmentShader but with a bunch of words truncated.
const uint32_t kInvalidFragmentShader[] = {
    0x07230203, 0x00010000, 0x00080001, 0x0000000d, 0x00000000, 0x00020011,
    0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000004,
    0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000b, 0x00030010,
    0x00000004, 0x00000007, 0x00030003, 0x00000001, 0x00000136, 0x00040005,
    0x00000004, 0x6e69616d, 0x00000000, 0x00050005};

// Tests that the optimizer can be created and deleted with no leaks.
TEST(SpirVOptimizerTest, Unused) { SpirVOptimizer optimizer({}); }

// Tests optimizing an empty SPIR-V data blob.
TEST(SpirVOptimizerTest, Empty) {
  SpirVOptimizer optimizer({});
  std::vector<uint32_t> target_spirv;
  // To test that the provided output is cleared even on failure:
  target_spirv.push_back(1);
  EXPECT_FALSE(optimizer.Optimize({}, &target_spirv));
  EXPECT_TRUE(target_spirv.empty());
}

// Tests optimizing various bad inputs that should cause graceful failures.
TEST(SpirVOptimizerTest, InvalidInput) {
  std::vector<uint32_t> source_spirv;
  source_spirv.resize(count_of(kInvalidFragmentShader));
  std::memcpy(source_spirv.data(), kInvalidFragmentShader,
              sizeof(kInvalidFragmentShader));

  SpirVOptimizer::Options options;
  options.strip_debug_info = true;
  SpirVOptimizer optimizer(options);
  std::vector<uint32_t> target_spirv;
  EXPECT_FALSE(optimizer.Optimize(source_spirv, &target_spirv));
  EXPECT_TRUE(target_spirv.empty());
}

// Tests valid optimization.
TEST(SpirVOptimizerTest, Optimization) {
  std::vector<uint32_t> source_spirv;
  source_spirv.resize(count_of(kValidVertexShader));
  std::memcpy(source_spirv.data(), kValidVertexShader,
              sizeof(kValidVertexShader));

  SpirVOptimizer::Options options;
  options.strip_debug_info = true;
  SpirVOptimizer optimizer(options);
  std::vector<uint32_t> target_spirv;
  EXPECT_TRUE(optimizer.Optimize(source_spirv, &target_spirv));
  EXPECT_EQ(194, target_spirv.size());
}

// Tests reusing the optimizer for multiple optimizations.
TEST(SpirVOptimizerTest, Reuse) {
  SpirVOptimizer::Options options;
  options.strip_debug_info = true;
  SpirVOptimizer optimizer(options);

  std::vector<uint32_t> vertex_source_spirv;
  vertex_source_spirv.resize(count_of(kValidVertexShader));
  std::memcpy(vertex_source_spirv.data(), kValidVertexShader,
              sizeof(kValidVertexShader));

  std::vector<uint32_t> vertex_target_spirv;
  EXPECT_TRUE(optimizer.Optimize(vertex_source_spirv, &vertex_target_spirv));
  EXPECT_EQ(194, vertex_target_spirv.size());

  std::vector<uint32_t> fragment_source_spirv;
  fragment_source_spirv.resize(count_of(kValidFragmentShader));
  std::memcpy(fragment_source_spirv.data(), kValidFragmentShader,
              sizeof(kValidFragmentShader));

  std::vector<uint32_t> fragment_target_spirv;
  EXPECT_TRUE(
      optimizer.Optimize(fragment_source_spirv, &fragment_target_spirv));
  EXPECT_EQ(78, fragment_target_spirv.size());
}

}  // namespace
}  // namespace spirv
}  // namespace gfx
}  // namespace xrtl
