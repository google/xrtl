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

// Test runner shim for ios_test.
// This uses XCTest (the supported iOS testing framework) to call into our
// gtest cc_test routines.
//
// We could probably be a bit more clever with reflection and create real
// XCTest wrapper functions for test methods, but this is better than nothing.

#import <XCTest/XCTest.h>

#include <unistd.h>

#include "xrtl/base/env.h"
#include "xrtl/base/flags.h"
#include "xrtl/testing/file_util.h"
#include "xrtl/testing/gtest.h"
#include "xrtl/tools/target_platform/target_platform.h"

@interface XRTLTestRunner : XCTestCase
+ (void)setUp;
@end

@implementation XRTLTestRunner

+ (void)setUp {
  // TODO(benvanik): accept parameters somehow?
  std::vector<std::string> args;
  args.push_back("apple_test_main");  // TODO(benvanik): real name.
  args.push_back("--logtostderr");
  int argc = static_cast<int>(args.size());
  char** argv = reinterpret_cast<char**>(std::malloc(sizeof(char*) * argc));
  for (size_t i = 0; i < args.size(); ++i) {
    const std::string& arg = args[i];
    char* arg_buffer = reinterpret_cast<char*>(std::malloc(arg.size() + 1));
    std::strcpy(arg_buffer, arg.c_str());
    argv[i] = arg_buffer;
  }

  // testdata is built into the xctest bundle, which is different than
  // the mainBundle xctest_app runner.
  // We change the working directory so the generic gtest code thinks it is
  // in a runfiles path.
  NSBundle* testBundle = [NSBundle bundleForClass:[XRTLTestRunner class]];
  NSString* testRootPath = [testBundle resourcePath];
  ::chdir([testRootPath UTF8String]);

#if defined(XRTL_PLATFORM_IOS)
  // When running on the phone we need to write to the Documents folder as it's
  // the only folder we can read back on the attached computer.
  NSURL* docsPath = [[[NSFileManager defaultManager]
      URLsForDirectory:NSDocumentDirectory
             inDomains:NSUserDomainMask] lastObject];
  xrtl::Env::set_temp_path([[docsPath path] UTF8String]);
#endif  // XRTL_PLATFORM_IOS

  // Setup flags.
  xrtl::flags::SetUsageMessage(std::string("\n$ ") + argv[0]);
  xrtl::flags::ParseCommandLineFlags(&argc, &argv, true);

  // Setup gtest.
  ::testing::InitGoogleTest(&argc, argv);

  // Load runfiles manifest.
  xrtl::testing::FileUtil::LoadFileManifest(std::string(argv[0]));

  // TODO(benvanik): benchmark.
  // ::benchmark::Initialize(&argc, argv);

  // TODO(benvanik): free command line flags.
}

+ (void)tearDown {
  xrtl::flags::ShutDownCommandLineFlags();
}

- (void)testRunAllTests {
  XCTAssertEqual(RUN_ALL_TESTS(), 0);
}

// TODO(benvanik): benchmark.
// - (void)testRunAllBenchmarks {
//   ::benchmark::RunSpecifiedBenchmarks();
// }

@end
