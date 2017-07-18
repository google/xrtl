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

#ifndef XRTL_GFX_CONTEXT_FACTORY_H_
#define XRTL_GFX_CONTEXT_FACTORY_H_

#include <string>
#include <utility>
#include <vector>

#include "xrtl/base/array_view.h"
#include "xrtl/base/ref_ptr.h"
#include "xrtl/gfx/context.h"
#include "xrtl/gfx/device.h"

namespace xrtl {
namespace gfx {

// Graphics context factory.
// Factories are implemented per graphics API backend and enable device
// enumeration and context creation.
//
// Usage:
//  auto factory = CreateContextFactorySomehow();
//  auto device = factory->default_device();
//  Device::Features features;
//  features.foo = true;
//  ref_ptr<Context> context;
//  factory->CreateContext(device, features, &context);
class ContextFactory : public RefObject<ContextFactory> {
 public:
  virtual ~ContextFactory() = default;

  // Creates a context factory with the given backend name.
  // Pass empty string to get the default platform backend factory.
  // Returns nullptr if the backend is not compiled in or supported on the
  // current platform.
  //
  // Valid values:
  //   '': platform default or --gfx= flag value, possibly nop.
  //   'nop': no-op (null) backend; performs no rendering.
  //   'es3': OpenGL ES 3.X (Android/Emscripten/Linux/iOS/MacOS/Windows)
  //   'metal': Metal (iOS/MacOS only)
  //   'vulkan': Vulkan (Android/Linux/Windows)
  static ref_ptr<ContextFactory> Create(std::string name = "");

  // TODO(benvanik): add API name/version/etc.

  // Returns a list of all devices currently available for use by this API.
  // Note that not all backends support all devices that may be present in
  // a system.
  virtual const std::vector<ref_ptr<Device>>& devices() const = 0;

  // Returns the device that can be used for the best performance on the system.
  // May return nullptr if there are no devices available for use.
  // For example on a system with both an integrated and discrete GPU this will
  // return the discrete one.
  virtual ref_ptr<Device> default_device() const = 0;

  // Defines the result of a CreateContext operation.
  enum class CreateResult {
    // Context was created successfully and is ready for use.
    kSuccess,
    // Context could not be created for a reason not covered by the other
    // errors.
    kUnknownError,
    // One or more of the features requested was not available.
    kUnsupportedFeatures,
    // The devices provided were not compatible with each other. All devices
    // must have the same multi-device compatibility group.
    kIncompatibleDevices,
    // Too many contexts were allocated and no more are available.
    kTooManyContexts,
    // Driver reported it was out of memory or unable to allocate system
    // resources.
    kOutOfMemory,
    // Device has been lost and may require user intervention (reboot, etc).
    kDeviceLost,
  };

  // Creates a new graphics context using the given devices.
  // All devices specified must be in a multi-device compatibility group as
  // indicated by multi_device_group_id. The required features provided will be
  // used to enable context features if supported and otherwise will cause
  // creation to fail. All requested features must be supported across all
  // devices.
  //
  // Returns a result indicating whether creation succeeded or the reason it
  // failed. Failures may happen for many reasons and may happen upon creation
  // with parameters that have previously succeeded (such as if the system is
  // out of resources).
  virtual CreateResult CreateContext(ArrayView<ref_ptr<Device>> devices,
                                     Device::Features required_features,
                                     ref_ptr<Context>* out_context) = 0;
  CreateResult CreateContext(ref_ptr<Device> device,
                             Device::Features required_features,
                             ref_ptr<Context>* out_context) {
    return CreateContext(std::array<ref_ptr<Device>, 1>{{device}},
                         std::move(required_features), out_context);
  }

 protected:
  ContextFactory() = default;
};

std::ostream& operator<<(std::ostream& stream,
                         const ContextFactory::CreateResult& value);

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_CONTEXT_FACTORY_H_
