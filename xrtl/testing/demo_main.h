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

#ifndef XRTL_TESTING_DEMO_MAIN_H_
#define XRTL_TESTING_DEMO_MAIN_H_

namespace xrtl {
namespace testing {

using EntryPointFn = int (*)(int argc, char** argv);

// Returns a function pointer to the entry point of the application.
// DemoMain will call this once the environment has been setup.
//
// Usage:
//  my_demo.cc:
//   int MyEntry(int argc, char** argv) { return 55; }
//   DECLARE_ENTRY_POINT(MyEntry);
EntryPointFn GetEntryPoint();

#define DECLARE_ENTRY_POINT(fn) \
  xrtl::testing::EntryPointFn xrtl::testing::GetEntryPoint() { return fn; }

// Application entry point for demos.
// This follows the standard C main() function convention.
int DemoMain(int argc, char** argv);

}  // namespace testing
}  // namespace xrtl

#endif  // XRTL_TESTING_DEMO_MAIN_H_
