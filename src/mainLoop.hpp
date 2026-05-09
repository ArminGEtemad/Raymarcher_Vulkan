#pragma once

// add header files
#include "cameraLogic.hpp"
#include "pipeline.hpp"
#include "setup.hpp"
#include "swapchain.hpp"
#include "window_handling.hpp"
#include <vulkan/vulkan.h>

// add libraries
#include <filesystem>
#include <memory>

namespace miniEngine {
class makeApp {
public:
  static constexpr int WIDTH = 1000;
  static constexpr int HEIGHT = 1000;

  makeApp();
  ~makeApp();

  makeApp(const makeApp &) = delete;
  makeApp &operator=(const makeApp &) = delete;

  void run();

private:
  // initializations
  WindowHandling createWindow{WIDTH, HEIGHT, "Raymarcher Grapher"};
  SetupDevice device{createWindow};
  SwapChainEngine swapChain{createWindow, device};
  std::unique_ptr<PipelineEngine> pipeline;
  VkCommandBuffer commandBuffer;

  // precalculated noise varaiables
  VkImage noiseImage;
  VkDeviceMemory noiseImageMemory;
  VkImageView noiseImageView;
  VkDescriptorSet computeDescriptorSet;
  VkSampler noiseSampler;
  VkDescriptorSet graphicDescriptorSet;

  // sync
  VkSemaphore imageAvailableSemaphore;
  VkSemaphore renderFinishedSemaphore;
  VkFence inFlightFence;

  // camera
  CameraLogic camera;

  // dynamic scene
  float startTime;

  // functions
  void createSyncObjects();
  void allocateCommandBuffer();
  void drawFrame();

  // hot reload
  std::filesystem::file_time_type lastShaderWriteTime;
  PipelineConfigInfo cachedConfigInfo;
  void reloadShader();
  void checkShaderUpdate();

  // precalculate the noise
  void generateNoise();
};
} // namespace miniEngine
