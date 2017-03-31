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

#include "xrtl/base/tracing.h"

#include "xrtl/base/logging.h"

namespace xrtl {
namespace tracing {

#if WTF_ENABLE

namespace {

uint32_t current_frame_number_ = 0;

}  // namespace

void EmitFrameStart() {
  wtf::StandardEvents::FrameStart(wtf::PlatformGetThreadLocalEventBuffer(),
                                  current_frame_number_);
}

void EmitFrameEnd() {
  wtf::StandardEvents::FrameEnd(wtf::PlatformGetThreadLocalEventBuffer(),
                                current_frame_number_);
  ++current_frame_number_;
}

void SaveToFile(std::string file_path) {
  if (wtf::Runtime::GetInstance()->SaveToFile(file_path)) {
    LOG(INFO) << "Wrote trace to " << file_path;
  } else {
    LOG(ERROR) << "Unable to write trace file to " << file_path;
  }
}

#else

// Make linkers happy with our empty file.
namespace {
void Dummy() {}
}  // namespace

#endif  // WTF_ENABLE

}  // namespace tracing
}  // namespace xrtl
