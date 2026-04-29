// add header files
#include "setup.hpp"
#include <GLFW/glfw3.h>

// add libraries
#include <iostream>
#include <stdexcept>
#include <vector>

namespace miniEngine {
SetupDevice::SetupDevice() { createInstance(); }
SetupDevice::~SetupDevice() { vkDestroyInstance(instance, nullptr); }

void SetupDevice::createInstance() {
  // extension support
  hasInstanceExtension();

  // App Info
  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Vulkan Raymarching grapher";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "MiniEngine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_3;

  // Instance Info
  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  // TODO for now until I add validation layer
  // ---------- To be changed when validation layers are added ----------
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  createInfo.enabledExtensionCount = glfwExtensionCount;
  createInfo.ppEnabledExtensionNames = glfwExtensions;
  createInfo.enabledLayerCount = 0;
  createInfo.pNext = nullptr;
  // ---------------------------------------------------------------------

  // if failed to do Instance throw runtime error
  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
    throw std::runtime_error("failed to create instance");
  }
}
// checking for extension support
void SetupDevice::hasInstanceExtension() {
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                         extensions.data());
  std::cout << "available extensions:\n";
  for (const auto &extension : extensions) {
    std::cout << '\t' << extension.extensionName << '\n';
  }
}

} // namespace miniEngine