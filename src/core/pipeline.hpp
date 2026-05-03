#pragma once

// add header files
#include "setup.hpp"
#include <glm/glm.hpp>

namespace miniEngine {
// camera position to be pushed into the gpu
struct alignas(16) CameraPushConstants {
  glm::vec3 position;
};

struct PipelineConfigInfo {
  PipelineConfigInfo() = default;

  PipelineConfigInfo(const PipelineConfigInfo &) = delete;
  PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

  VkPipelineViewportStateCreateInfo viewportInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  std::vector<VkDynamicState> dynamicStateEnables;
  VkPipelineDynamicStateCreateInfo dynamicStateInfo;
  VkFormat colorAttachmentFormat;
  VkPipelineLayout pipelineLayout = nullptr;
};
class PipelineEngine {
public:
  PipelineEngine(SetupDevice &device, const PipelineConfigInfo &configInfo);
  ~PipelineEngine();

  PipelineEngine(const PipelineEngine &) = delete;
  PipelineEngine &operator=(const PipelineEngine &) = delete;

  static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);
  VkPipeline getGraphicsPipeline() { return graphicsPipeline; }
  VkPipelineLayout getPiplineLayout() { return pipelineLayout; }

private:
  SetupDevice &device;
  static std::vector<char> readFile(const std::string &filepath);
  void createGraphicsPipeline(const PipelineConfigInfo &configInfo);
  VkShaderModule createShaderModule(const std::vector<char> &code);
  VkShaderModule vertShaderModule;
  VkShaderModule fragShaderModule;
  VkPipeline graphicsPipeline;
  VkPipelineLayout pipelineLayout;
};
} // namespace miniEngine