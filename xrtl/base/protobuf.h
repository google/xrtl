// Copyright 2018 Google Inc.
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

#ifndef XRTL_BASE_PROTOBUF_H_
#define XRTL_BASE_PROTOBUF_H_

#include "xrtl/base/macros.h"

// Import the platform-specific protobuf implementation into the xrtl::protobuf
// namespace.

#ifdef XRTL_CONFIG_GOOGLE_INTERNAL
#include "xrtl/port/google/base/protobuf.h"
#else
#include "xrtl/port/common/base/protobuf.h"
#endif  // XRTL_CONFIG_GOOGLE_INTERNAL

#endif  // XRTL_BASE_PROTOBUF_H_
