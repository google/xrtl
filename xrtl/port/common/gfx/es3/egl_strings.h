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

#ifndef XRTL_PORT_COMMON_GFX_ES3_EGL_STRINGS_H_
#define XRTL_PORT_COMMON_GFX_ES3_EGL_STRINGS_H_

namespace xrtl {
namespace gfx {
namespace es3 {

// Returns a string representing the given EGL error enum value name.
const char* GetEglErrorName(int error);

// Returns a string representing the given EGL error enum value description.
const char* GetEglErrorDescription(int error);

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_PORT_COMMON_GFX_ES3_EGL_STRINGS_H_
