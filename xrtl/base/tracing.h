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

#ifndef XRTL_BASE_TRACING_H_
#define XRTL_BASE_TRACING_H_

#include <string>

#if !defined(WTF_ENABLE)
#define WTF_ENABLE 0
#endif  // !WTF_ENABLE

#if WTF_ENABLE
#include <wtf/event.h>
#include <wtf/macros.h>
#include <wtf/platform.h>
#include <wtf/runtime.h>
#endif  // WTF_ENABLE

namespace xrtl {
namespace tracing {

#if WTF_ENABLE

// Marks a frame start/end event with a monotonically increasing frame number.
void EmitFrameStart();
void EmitFrameEnd();

// Saves the current trace buffer to the given file path, if enabled.
void SaveToFile(std::string file_path);

#else

// Empty functions to prevent compilation errors.
inline void EmitFrameStart() {}
inline void EmitFrameEnd() {}
inline void SaveToFile(std::string file_path) {}

// No-op macros.
#define __WTF_IGNORED(...)
#define WTF_EVENT(...) __WTF_IGNORED
#define WTF_EVENT0(...)
#define WTF_SCOPE(...) __WTF_IGNORED
#define WTF_SCOPE0(...)

#endif  // WTF_ENABLE

}  // namespace tracing
}  // namespace xrtl

#endif  // XRTL_BASE_TRACING_H_
