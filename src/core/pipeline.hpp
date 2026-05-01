#pragma once

// add header files
#include "setup.hpp"

namespace miniEngine {
class PipelineEngine {
public:
  PipelineEngine(SetupDevice &device);
  ~PipelineEngine();

  PipelineEngine(const PipelineEngine &) = delete;
  PipelineEngine &operator=(const PipelineEngine &) = delete;

private:
  SetupDevice &device;
  static std::vector<char> readFile(const std::string &filepath);
  void createGraphicsPipeline();
  VkShaderModule createShaderModule(const std::vector<char> &code);
  VkShaderModule vertShaderModule;
  VkShaderModule fragShaderModule;
};
} // namespace miniEngine