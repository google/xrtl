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

precision highp float;

layout(push_constant) uniform PushConstants {
  mat4 proj_matrix;
} push_constants;

layout(set = 0, binding = 0) uniform sampler2D image_sampler;

layout(location = 0) in vec2 vtx_uv;
layout(location = 1) in vec4 vtx_color;

layout(location = 0) out vec4 out_color;

void main() {
  vec4 tex_sample = texture(image_sampler, vtx_uv);
  out_color = vtx_color * tex_sample;
}
