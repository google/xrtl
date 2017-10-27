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

#ifndef XRTL_GFX_VK_VK_CONTEXT_FACTORY_H_
#define XRTL_GFX_VK_VK_CONTEXT_FACTORY_H_

#include <vector>

#include "absl/types/optional.h"
#include "xrtl/gfx/context_factory.h"
#include "xrtl/gfx/vk/vk_common.h"
#include "xrtl/gfx/vk/vk_device.h"

namespace xrtl {
namespace gfx {
namespace vk {

// A bitmask of well-known Vulkan instance extensions.
enum class InstanceExtension {
  kNone = 0,
  // VK_EXT_debug_report
  kDebugReport = 1 << 0,
};
XRTL_BITMASK(InstanceExtension);

class VKContextFactory : public ContextFactory {
 public:
  VKContextFactory();
  ~VKContextFactory() override;

  // Returns true if the context factory is supported.
  // This is used for run-time checks that may require querying process
  // permissions or dll presence.
  static bool IsSupported();

  // A bitmask indicating which instance extensions have been enabled.
  InstanceExtension enabled_extensions() const { return enabled_extensions_; }

  absl::Span<const ref_ptr<Device>> devices() const override {
    return devices_;
  }

  ref_ptr<Device> default_device() const override { return default_device_; }

  CreateResult CreateContext(absl::Span<const ref_ptr<Device>> devices,
                             Device::Features required_features,
                             ref_ptr<Context>* out_context) override;

 private:
  struct LayerInfo {
    VkLayerProperties layer_properties = {{0}};
    std::vector<VkExtensionProperties> extension_properties;
  };

  // Initialize Vulkan and get an application instance.
  bool InitializeInstance();

  // Queries all extensions available for the given layer.
  // If layer_name is nullptr the global non-layer extensions are queried.
  // Returns true if the extensions were successfully queried.
  bool QueryLayerExtensions(
      const char* layer_name,
      std::vector<VkExtensionProperties>* out_extension_properties) const;

  // Queries all layers and extensions available for instance creation.
  // Returns a list of instances if queried successfully. If this fails it's
  // likely that the loader is misconfigured or one or more extension metadata
  // files/regkeys/etc are bad.
  absl::optional<std::vector<LayerInfo>> QueryInstanceLayerExtensions() const;

  // Selects the layers to be enabled based on compilation mode and flags.
  // Returns a list of layer names or nullopt if one or more required layers
  // are not available.
  absl::optional<std::vector<const char*>> SelectInstanceLayers(
      const std::vector<LayerInfo>& available_layer_infos) const;

  // Selects the extensions to be enabled based on compilation mode and flags.
  // Returns a list of extension names or nullopt if one or more required
  // extensions are not available.
  absl::optional<std::vector<const char*>> SelectInstanceExtensions(
      const std::vector<VkExtensionProperties>& available_extension_properties)
      const;

  // Queries and populates available devices.
  // Returns false if no devices are available or an error occurred.
  bool QueryDevices();

  VkInstance instance_ = nullptr;
  InstanceExtension enabled_extensions_ = InstanceExtension::kNone;

  std::vector<ref_ptr<Device>> devices_;
  ref_ptr<VKDevice> default_device_;
};

}  // namespace vk
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_VK_VK_CONTEXT_FACTORY_H_
