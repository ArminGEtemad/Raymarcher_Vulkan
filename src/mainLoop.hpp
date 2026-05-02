#pragma once

// add header files
#include "pipeline.hpp"
#include "setup.hpp"
#include "swapchain.hpp"
#include "window_handling.hpp"
#include <memory>
#include <vulkan/vulkan.h>

namespace miniEngine {
class makeApp {
public:
  static constexpr int WIDTH = 1000;
  static constexpr int HEIGHT = 800;

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

  // sync
  VkSemaphore imageAvailableSemaphore;
  VkSemaphore renderFinishedSemaphore;
  VkFence inFlightFence;

  // functions
  void createSyncObjects();
  void allocateCommandBuffer();
  void drawFrame();
};
} // namespace miniEngine
