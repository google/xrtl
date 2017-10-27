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

#include "xrtl/gfx/vk/vk_context_factory.h"

#include <utility>

#include "xrtl/base/flags.h"
#include "xrtl/base/logging.h"
// #include "xrtl/gfx/vk/vk_context.h"

DEFINE_bool(vk_debug_reporting, true,
            "Enable enhanced Vulkan error reporting.");
DEFINE_int32(vk_debug_verbosity, 3,
             "Verbosity level; 0=error+warning, 1=info, 2=debug, 3=perf.");
DEFINE_bool(vk_debug_validation, true, "Enable all debug layers available.");

namespace xrtl {
namespace gfx {
namespace vk {

namespace {

VKAPI_ATTR VkBool32 VKAPI_CALL VKContextDebugReportCallback(
    VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type,
    uint64_t object, size_t location, int32_t message_code,
    const char* layer_prefix, const char* message, void* user_data) {
  // TODO(benvanik): reporting.
  LOG(ERROR) << message;
  return VK_FALSE;
}

}  // namespace

bool VKContextFactory::IsSupported() {
  // TODO(benvanik): verify.
  return true;
}

VKContextFactory::VKContextFactory() {
  // Initialize Vulkan.
  if (!InitializeInstance()) {
    LOG(ERROR) << "Unable to initialize Vulkan";
    return;
  }

  // Perform a query of all devices now. If this fails we get no devices and
  // the caller should gracefully handle that.
  if (!QueryDevices()) {
    LOG(ERROR) << "Unable to query devices";
    return;
  }
}

VKContextFactory::~VKContextFactory() {
  if (instance_) {
    vkDestroyInstance(instance_, nullptr);
    instance_ = nullptr;
  }
}

bool VKContextFactory::InitializeInstance() {
  // Query and select the instance layers to enable.
  auto layer_extensions_opt = QueryInstanceLayerExtensions();
  if (!layer_extensions_opt) {
    LOG(ERROR) << "Failed to query instance layers and extension properties";
    return false;
  }
  auto enabled_layers_opt = SelectInstanceLayers(layer_extensions_opt.value());
  if (!enabled_layers_opt) {
    LOG(ERROR) << "One or more required instance layers are not available";
    return false;
  }

  // Query and select the instance extensions to enable.
  std::vector<VkExtensionProperties> extension_properties;
  if (!QueryLayerExtensions(nullptr, &extension_properties)) {
    LOG(ERROR) << "Failed to query global instance extension properties";
    return false;
  }
  auto enabled_extensions_opt = SelectInstanceExtensions(extension_properties);
  if (!enabled_extensions_opt) {
    LOG(ERROR) << "One or more required instance extensions are not available";
    return false;
  }

  // Map extension bitmask.
  enabled_extensions_ = InstanceExtension::kNone;
  for (const char* extension_name : enabled_extensions_opt.value()) {
    if (std::strcmp(extension_name, VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0) {
      enabled_extensions_ |= InstanceExtension::kDebugReport;
    }
  }

  VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
  app_info.pNext = nullptr;
  app_info.pApplicationName = "XRTL";  // TODO(benvanik): plumb from top level?
  app_info.applicationVersion = 1;
  app_info.pEngineName = "XRTL";
  app_info.engineVersion = 1;
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo instance_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  instance_info.pNext = nullptr;
  instance_info.pApplicationInfo = &app_info;
  instance_info.enabledLayerCount = enabled_layers_opt.value().size();
  instance_info.ppEnabledLayerNames = enabled_layers_opt.value().data();
  instance_info.enabledExtensionCount = enabled_extensions_opt.value().size();
  instance_info.ppEnabledExtensionNames = enabled_extensions_opt.value().data();

  // Enable debug reporting, if available/enabled.
  VkDebugReportCallbackCreateInfoEXT debug_report_info = {
      VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT};
  if (any(enabled_extensions_ & InstanceExtension::kDebugReport)) {
    debug_report_info.pNext = nullptr;
    debug_report_info.flags =
        VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    if (FLAGS_vk_debug_verbosity > 0) {
      debug_report_info.flags |= VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
    }
    if (FLAGS_vk_debug_verbosity > 1) {
      debug_report_info.flags |= VK_DEBUG_REPORT_DEBUG_BIT_EXT;
    }
    if (FLAGS_vk_debug_verbosity > 2) {
      debug_report_info.flags |= VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    }
    debug_report_info.pfnCallback = VKContextDebugReportCallback;
    debug_report_info.pUserData = this;
    instance_info.pNext = &debug_report_info;
  }

  // Create the instance. This will fail if we have a version mismatch, the
  // driver rejects us, etc.
  VkResult err = vkCreateInstance(&instance_info, nullptr, &instance_);
  if (err) {
    LOG(ERROR) << "Failed to create Vulkan instance: " << to_string(err);
    return false;
  }

  // Instance is ready for use.
  return true;
}

bool VKContextFactory::QueryLayerExtensions(
    const char* layer_name,
    std::vector<VkExtensionProperties>* out_extension_properties) const {
  // Enumerate the extension properties.
  // Guard for the count changing between calls to vkEnum* by looping until
  // complete.
  out_extension_properties->clear();
  VkResult err = VK_INCOMPLETE;
  while (err == VK_INCOMPLETE) {
    // Query count.
    uint32_t extension_count = 0;
    err = vkEnumerateInstanceExtensionProperties(layer_name, &extension_count,
                                                 nullptr);
    if (err) {
      LOG(ERROR) << "Failed to query layer " << layer_name
                 << " extension property count: " << to_string(err);
      return false;
    }

    // Reserve storage and query properties.
    out_extension_properties->resize(extension_count);
    err = vkEnumerateInstanceExtensionProperties(
        layer_name, &extension_count, out_extension_properties->data());
    if (err && err != VK_INCOMPLETE) {
      LOG(ERROR) << "Failed to query layer " << layer_name
                 << " extension properties: " << to_string(err);
      return false;
    }
    out_extension_properties->resize(extension_count);
  }
  return true;
}

absl::optional<std::vector<VKContextFactory::LayerInfo>>
VKContextFactory::QueryInstanceLayerExtensions() const {
  // Enumerate the layer properties.
  // Guard for the count changing between calls to vkEnum* by looping until
  // complete.
  std::vector<VkLayerProperties> layer_properties_list;
  VkResult err = VK_INCOMPLETE;
  while (err == VK_INCOMPLETE) {
    // Query count.
    uint32_t layer_count = 0;
    err = vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    if (err) {
      LOG(ERROR) << "Failed to query instance layer count: " << to_string(err);
      return absl::nullopt;
    }

    // Reserve storage and query properties.
    layer_properties_list.resize(layer_count);
    err = vkEnumerateInstanceLayerProperties(&layer_count,
                                             layer_properties_list.data());
    if (err && err != VK_INCOMPLETE) {
      LOG(ERROR) << "Failed to query instance layer properties: "
                 << to_string(err);
      return absl::nullopt;
    }
    layer_properties_list.resize(layer_count);
  }

  // Query the extensions available for each layer.
  std::vector<LayerInfo> layer_info_list;
  layer_info_list.resize(layer_properties_list.size());
  for (size_t i = 0; i < layer_properties_list.size(); ++i) {
    auto& layer_info = layer_info_list[i];
    std::memcpy(&layer_info.layer_properties, &layer_properties_list[i],
                sizeof(layer_info.layer_properties));
    if (!QueryLayerExtensions(layer_info.layer_properties.layerName,
                              &layer_info.extension_properties)) {
      LOG(ERROR) << "Failed to query instance layer "
                 << layer_info.layer_properties.layerName << " properties";
      return absl::nullopt;
    }
  }

  return layer_info_list;
}

absl::optional<std::vector<const char*>> VKContextFactory::SelectInstanceLayers(
    const std::vector<LayerInfo>& available_layer_infos) const {
  // Select the layers we want to enable based on compilation mode and runtime
  // flags.
  std::vector<const char*> required_layers;
  std::vector<const char*> optional_layers;
  if (FLAGS_vk_debug_validation) {
    optional_layers.push_back("VK_LAYER_LUNARG_standard_validation");
  }

  // Filter desired layers to those we have available.
  std::vector<const char*> enabled_layers;
  for (const LayerInfo& layer_info : available_layer_infos) {
    for (auto it = required_layers.begin(); it != required_layers.end(); ++it) {
      if (std::strcmp(*it, layer_info.layer_properties.layerName) == 0) {
        enabled_layers.push_back(*it);
        required_layers.erase(it);
        break;
      }
    }
    for (const char* layer_name : optional_layers) {
      if (std::strcmp(layer_name, layer_info.layer_properties.layerName) == 0) {
        enabled_layers.push_back(layer_name);
      }
    }
  }

  // Bail if we were unable to find some required layers.
  if (!required_layers.empty()) {
    LOG(ERROR) << "Failed to find the following required instance layers:";
    for (const char* layer_name : required_layers) {
      LOG(ERROR) << "  " << layer_name;
    }
    return absl::nullopt;
  }

  if (!enabled_layers.empty()) {
    VLOG(0) << "Enabling instance layers:";
    for (const char* layer_name : enabled_layers) {
      VLOG(0) << "  " << layer_name;
    }
  }

  return enabled_layers;
}

absl::optional<std::vector<const char*>>
VKContextFactory::SelectInstanceExtensions(
    const std::vector<VkExtensionProperties>& available_extension_properties)
    const {
  // Select the extensions we want to enable on the instance based on
  // compilation mode and runtime flags.
  std::vector<const char*> required_extensions;
  std::vector<const char*> optional_extensions;
  if (FLAGS_vk_debug_reporting || FLAGS_vk_debug_validation) {
    optional_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
  }

  // Enable support for surfaces and on-screen rendering (if not headless).
  // TODO(benvanik): headless mode.
  bool is_headless = false;
  if (!is_headless) {
    required_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    optional_extensions.push_back(
        VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
#ifdef VK_USE_PLATFORM_WIN32_KHR
    required_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif VK_USE_PLATFORM_XCB_KHR
    required_extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif VK_USE_PLATFORM_XLIB_KHR
    required_extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif VK_USE_PLATFORM_WAYLAND_KHR
    required_extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif VK_USE_PLATFORM_ANDROID_KHR
    required_extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif  // VK_USE_PLATFORM_*
  }

  // Filter desired extensions to those we have available.
  std::vector<const char*> enabled_extensions;
  for (const VkExtensionProperties& extension_properties :
       available_extension_properties) {
    for (auto it = required_extensions.begin(); it != required_extensions.end();
         ++it) {
      if (std::strcmp(*it, extension_properties.extensionName) == 0) {
        enabled_extensions.push_back(*it);
        required_extensions.erase(it);
        break;
      }
    }
    for (const char* extension_name : optional_extensions) {
      if (std::strcmp(extension_name, extension_properties.extensionName) ==
          0) {
        enabled_extensions.push_back(extension_name);
      }
    }
  }

  // Bail if we were unable to find some required extensions.
  if (!required_extensions.empty()) {
    LOG(ERROR) << "Failed to find the following required instance extensions:";
    for (const char* extension_name : required_extensions) {
      LOG(ERROR) << "  " << extension_name;
    }
    return absl::nullopt;
  }

  if (!enabled_extensions.empty()) {
    VLOG(0) << "Enabling instance extension:";
    for (const char* extension_name : enabled_extensions) {
      VLOG(0) << "  " << extension_name;
    }
  }

  return enabled_extensions;
}

bool VKContextFactory::QueryDevices() {
  // // Create the default device.
  // // There are GL extensions to get multiple devices, but this is fine for
  // now. VKPlatformContext::ExclusiveLock context_lock(shared_context_); auto
  // default_device = make_ref<VKDevice>(); if
  // (!default_device->AdoptCurrentContext()) {
  //   LOG(ERROR) << "Unable to query default device properties";
  //   return false;
  // }

  // default_device_ = default_device;
  // devices_.push_back(default_device);

  LOG(ERROR) << "QUERY DEVICES";

  return true;
}

ContextFactory::CreateResult VKContextFactory::CreateContext(
    absl::Span<const ref_ptr<Device>> devices,
    Device::Features required_features, ref_ptr<Context>* out_context) {
  // if (!shared_context_) {
  //   LOG(ERROR) << "Context factory has no platform context";
  //   return CreateResult::kUnknownError;
  // }
  // if (devices.empty()) {
  //   LOG(ERROR) << "No devices specified for context use";
  //   return CreateResult::kIncompatibleDevices;
  // }

  // // Ensure all devices are in the same group.
  // int multi_device_group_id = devices[0]->multi_device_group_id();
  // for (const auto& device : devices) {
  //   if (device->multi_device_group_id() != multi_device_group_id) {
  //     LOG(ERROR) << "One or more devices are incompatible for multi-device
  //     use"; return CreateResult::kIncompatibleDevices;
  //   }
  // }

  // // Ensure all devices are compatible with the required features.
  // for (const auto& device : devices) {
  //   if (!device->IsCompatible(required_features)) {
  //     LOG(ERROR) << "One or more devices do not support all required
  //     features"; return CreateResult::kUnsupportedFeatures;
  //   }
  // }

  // // Create the underlying platform context.
  // auto platform_context = VKPlatformContext::Create(shared_context_);
  // if (!platform_context) {
  //   LOG(ERROR) << "Unable to create new platform context";
  //   return CreateResult::kUnknownError;
  // }

  // // Wrap the platform context in our top level context type.
  // auto context =
  //     make_ref<VKContext>(ref_ptr<ContextFactory>(this), devices,
  //                         required_features, std::move(platform_context));
  // *out_context = context.As<Context>();

  return CreateResult::kSuccess;
}

}  // namespace vk
}  // namespace gfx
}  // namespace xrtl
