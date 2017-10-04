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

#include "xrtl/base/uri.h"

namespace xrtl {
namespace uri {

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  absl::string_view input_uri(reinterpret_cast<const char*>(data), size);
  IsSchemeless(input_uri);
  GetScheme(input_uri);
  GetHost(input_uri);
  GetOrigin(input_uri);
  GetPath(input_uri);
  IsPathAbsolute(input_uri);
  GetBasePath(input_uri);
  JoinParts(input_uri, "a");
  JoinParts("a", input_uri);
  JoinParts(input_uri, input_uri);
  return 0;
}

}  // namespace uri
}  // namespace xrtl
