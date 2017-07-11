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

layout(push_constant, std140) uniform PushConstants {
  mat4 proj_matrix;
} push_constants;

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_color;

layout(location = 0) out vec2 vtx_uv;
layout(location = 1) out vec4 vtx_color;

void main() {
  gl_Position = push_constants.proj_matrix * vec4(in_pos.xy, 0.0, 1.0);
  vtx_uv = in_uv;
  vtx_color = in_color;
}
