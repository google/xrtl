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

#ifndef XRTL_GFX_ES3_ES3_CONTEXT_FACTORY_H_
#define XRTL_GFX_ES3_ES3_CONTEXT_FACTORY_H_

#include <vector>

#include "xrtl/gfx/context_factory.h"
#include "xrtl/gfx/es3/es3_common.h"
#include "xrtl/gfx/es3/es3_device.h"
#include "xrtl/gfx/es3/es3_platform_context.h"

namespace xrtl {
namespace gfx {
namespace es3 {

class ES3ContextFactory : public ContextFactory {
 public:
  ES3ContextFactory();
  ~ES3ContextFactory() override;

  // Returns true if the context factory is supported.
  // This is used for run-time checks that may require querying process
  // permissions or dll presence.
  static bool IsSupported();

  absl::Span<const ref_ptr<Device>> devices() const override {
    return devices_;
  }

  ref_ptr<Device> default_device() const override { return default_device_; }

  CreateResult CreateContext(absl::Span<const ref_ptr<Device>> devices,
                             Device::Features required_features,
                             ref_ptr<Context>* out_context) override;

 private:
  // Initialize EGL and get an EGLDisplay.
  // This is required to make EGL calls.
  bool Initialize();

  // Queries and populates available devices.
  // Returns false if no devices are available or an error occurred.
  bool QueryDevices();

  std::vector<ref_ptr<Device>> devices_;
  ref_ptr<ES3Device> default_device_;

  // Created at startup and used for object allocation by all created Contexts.
  ref_ptr<ES3PlatformContext> shared_context_;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_CONTEXT_FACTORY_H_
