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

#include "xrtl/gfx/es3/es3_context_factory.h"

#include <utility>

#include "xrtl/base/logging.h"
#include "xrtl/gfx/es3/es3_context.h"

namespace xrtl {
namespace gfx {
namespace es3 {

bool ES3ContextFactory::IsSupported() {
  // TODO(benvanik): verify.
  return true;
}

ES3ContextFactory::ES3ContextFactory() {
  // Initialize EGL and setup a dummy display.
  if (!Initialize()) {
    LOG(ERROR) << "Unable to initialize EGL connection";
    return;
  }

  // Perform a query of all devices now. If this fails we get no devices and
  // the caller should gracefully handle that.
  if (!QueryDevices()) {
    LOG(ERROR) << "Unable to query devices";
    return;
  }
}

ES3ContextFactory::~ES3ContextFactory() = default;

bool ES3ContextFactory::Initialize() {
  // Attempt to create the platform context.
  // This will be shared amongst created contexts.
  shared_context_ = ES3PlatformContext::Create();
  if (!shared_context_) {
    LOG(ERROR) << "Unable to initialize GL platform context";
    return false;
  }

  return true;
}

bool ES3ContextFactory::QueryDevices() {
  // Create the default device.
  // There are GL extensions to get multiple devices, but this is fine for now.
  ES3PlatformContext::ExclusiveLock context_lock(shared_context_);
  auto default_device = make_ref<ES3Device>();
  if (!default_device->AdoptCurrentContext()) {
    LOG(ERROR) << "Unable to query default device properties";
    return false;
  }

  default_device_ = default_device;
  devices_.push_back(default_device);

  return true;
}

ContextFactory::CreateResult ES3ContextFactory::CreateContext(
    ArrayView<ref_ptr<Device>> devices, Device::Features required_features,
    ref_ptr<Context>* out_context) {
  if (!shared_context_) {
    LOG(ERROR) << "Context factory has no platform context";
    return CreateResult::kUnknownError;
  }
  if (devices.empty()) {
    LOG(ERROR) << "No devices specified for context use";
    return CreateResult::kIncompatibleDevices;
  }

  // Ensure all devices are in the same group.
  int multi_device_group_id = devices[0]->multi_device_group_id();
  for (const auto& device : devices) {
    if (device->multi_device_group_id() != multi_device_group_id) {
      LOG(ERROR) << "One or more devices are incompatible for multi-device use";
      return CreateResult::kIncompatibleDevices;
    }
  }

  // Ensure all devices are compatible with the required features.
  for (const auto& device : devices) {
    if (!device->IsCompatible(required_features)) {
      LOG(ERROR) << "One or more devices do not support all required features";
      return CreateResult::kUnsupportedFeatures;
    }
  }

  // Create the underlying platform context.
  auto platform_context = ES3PlatformContext::Create(shared_context_);
  if (!platform_context) {
    LOG(ERROR) << "Unable to create new platform context";
    return CreateResult::kUnknownError;
  }

  // Wrap the platform context in our top level context type.
  auto context =
      make_ref<ES3Context>(ref_ptr<ContextFactory>(this), devices,
                           required_features, std::move(platform_context));
  *out_context = context.As<Context>();

  return CreateResult::kSuccess;
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
