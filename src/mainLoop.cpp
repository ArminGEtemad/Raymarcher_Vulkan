#include "mainLoop.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
namespace miniEngine {

makeApp::makeApp() {
  startTime = static_cast<float>(glfwGetTime());
  // PipelineConfigInfo cachedConfigInfo{};

  PipelineEngine::defaultPipelineConfigInfo(cachedConfigInfo);
  cachedConfigInfo.colorAttachmentFormat = swapChain.getFormat();

  pipeline = std::make_unique<PipelineEngine>(device, cachedConfigInfo);
  lastShaderWriteTime =
      std::filesystem::last_write_time("shaders/raymarch.frag");

  allocateCommandBuffer();
  createSyncObjects();
  generateNoise();
}
makeApp::~makeApp() {
  vkDeviceWaitIdle(device.getDevice());
  vkDestroySampler(device.getDevice(), noiseSampler, nullptr);
  vkDestroyImage(device.getDevice(), noiseImage, nullptr);
  vkDestroyImageView(device.getDevice(), noiseImageView, nullptr);
  vkFreeMemory(device.getDevice(), noiseImageMemory, nullptr);
  vkDestroySemaphore(device.getDevice(), renderFinishedSemaphore, nullptr);
  vkDestroySemaphore(device.getDevice(), imageAvailableSemaphore, nullptr);
  vkDestroyFence(device.getDevice(), inFlightFence, nullptr);
}

void makeApp::run() {
  while (!createWindow.shouldClose()) {
    glfwPollEvents();
    checkShaderUpdate();
    camera.update(createWindow);
    drawFrame();
  }
  vkDeviceWaitIdle(device.getDevice());
}

void makeApp::allocateCommandBuffer() {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = device.getCommandPool();
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = 1;

  if (vkAllocateCommandBuffers(device.getDevice(), &allocInfo,
                               &commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

void makeApp::createSyncObjects() {
  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start signaled so first
                                                  // frame doesn't wait forever

  vkCreateSemaphore(device.getDevice(), &semaphoreInfo, nullptr,
                    &imageAvailableSemaphore);
  vkCreateSemaphore(device.getDevice(), &semaphoreInfo, nullptr,
                    &renderFinishedSemaphore);
  vkCreateFence(device.getDevice(), &fenceInfo, nullptr, &inFlightFence);
}

// allocating image
void makeApp::generateNoise() {
  // memory and image
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_3D;
  imageInfo.extent = {256, 256, 256}; // TODO is testing for now
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateImage(device.getDevice(), &imageInfo, nullptr, &noiseImage) !=
      VK_SUCCESS) {
    throw std::runtime_error("faild to create noise image");
  }

  VkMemoryRequirements memRequirements{};
  vkGetImageMemoryRequirements(device.getDevice(), noiseImage,
                               &memRequirements);

  VkMemoryAllocateInfo memAllocInfo{};
  memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAllocInfo.allocationSize = memRequirements.size;
  memAllocInfo.memoryTypeIndex = device.findMemoryType(
      memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  if (vkAllocateMemory(device.getDevice(), &memAllocInfo, nullptr,
                       &noiseImageMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate memory for noise image");
  }

  vkBindImageMemory(device.getDevice(), noiseImage, noiseImageMemory, 0);

  // image View
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = noiseImage;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
  viewInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  if (vkCreateImageView(device.getDevice(), &viewInfo, nullptr,
                        &noiseImageView) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image view for nosie");
  }

  // descriptors
  VkDescriptorSetLayoutBinding computeLayoutBinding{};
  computeLayoutBinding.binding = 0;
  computeLayoutBinding.descriptorCount = 1;
  computeLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  computeLayoutBinding.pImmutableSamplers = nullptr;
  computeLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = &computeLayoutBinding;

  VkDescriptorSetLayout computeDescriptorSetLayout;
  vkCreateDescriptorSetLayout(device.getDevice(), &layoutInfo, nullptr,
                              &computeDescriptorSetLayout);
  // allocate and update
  VkDescriptorSetAllocateInfo descAllocInfo{};
  descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  descAllocInfo.descriptorPool = device.getDescriptorPool();
  descAllocInfo.descriptorSetCount = 1;
  descAllocInfo.pSetLayouts = &computeDescriptorSetLayout;

  vkAllocateDescriptorSets(device.getDevice(), &descAllocInfo,
                           &computeDescriptorSet);
  VkDescriptorImageInfo descriptorImageInfo{};
  descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
  descriptorImageInfo.imageView = noiseImageView;
  descriptorImageInfo.sampler = VK_NULL_HANDLE;

  VkWriteDescriptorSet descriptorWrite{};
  descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet = computeDescriptorSet;
  descriptorWrite.dstBinding = 0;
  descriptorWrite.dstArrayElement = 0;
  descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pImageInfo = &descriptorImageInfo;

  vkUpdateDescriptorSets(device.getDevice(), 1, &descriptorWrite, 0, nullptr);

  // build pipeline
  auto computeShaderCode =
      PipelineEngine::readFile("build/shaders/noise_gen.comp.spv");

  VkShaderModuleCreateInfo shaderCreateInfo{};
  shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderCreateInfo.codeSize = computeShaderCode.size();
  shaderCreateInfo.pCode =
      reinterpret_cast<const uint32_t *>(computeShaderCode.data());
  VkShaderModule computeShaderModule;
  vkCreateShaderModule(device.getDevice(), &shaderCreateInfo, nullptr,
                       &computeShaderModule);

  VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
  computeShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  computeShaderStageInfo.module = computeShaderModule;
  computeShaderStageInfo.pName = "main";

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &computeDescriptorSetLayout;

  VkPipelineLayout computePipelineLayout;
  vkCreatePipelineLayout(device.getDevice(), &pipelineLayoutInfo, nullptr,
                         &computePipelineLayout);

  VkComputePipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipelineInfo.stage = computeShaderStageInfo;
  pipelineInfo.layout = computePipelineLayout;

  VkPipeline computePipeline;
  vkCreateComputePipelines(device.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo,
                           nullptr, &computePipeline);

  // Buffer
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = device.getCommandPool();
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer computeCmd;
  vkAllocateCommandBuffers(device.getDevice(), &allocInfo, &computeCmd);

  // buffer info
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(computeCmd, &beginInfo);

  // image to writing
  VkImageMemoryBarrier writeBarrier{};
  writeBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  writeBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  writeBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
  writeBarrier.image = noiseImage;
  writeBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  writeBarrier.srcAccessMask = 0;
  writeBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

  vkCmdPipelineBarrier(computeCmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &writeBarrier);

  // bind compute
  vkCmdBindPipeline(computeCmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                    computePipeline);
  vkCmdBindDescriptorSets(computeCmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                          computePipelineLayout, 0, 1, &computeDescriptorSet, 0,
                          nullptr);

  // dispatch 256 / 8 = 32
  vkCmdDispatch(computeCmd, 32, 32, 32);

  // texture for reading
  VkImageMemoryBarrier readBarrier{};
  readBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  readBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
  readBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  readBarrier.image = noiseImage;
  readBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  readBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  readBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(computeCmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &readBarrier);

  vkEndCommandBuffer(computeCmd);

  // submit
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &computeCmd;

  vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(device.getGraphicsQueue());

  vkFreeCommandBuffers(device.getDevice(), device.getCommandPool(), 1,
                       &computeCmd);

  std::cout << "Noise Generated Successfully"
            << "\n";

  // clean up after the noise is generated. done once. no need to be in the
  // object
  vkDestroyPipeline(device.getDevice(), computePipeline, nullptr);
  vkDestroyPipelineLayout(device.getDevice(), computePipelineLayout, nullptr);
  vkDestroyShaderModule(device.getDevice(), computeShaderModule, nullptr);
  vkDestroyDescriptorSetLayout(device.getDevice(), computeDescriptorSetLayout,
                               nullptr);

  // now we can sample
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;

  if (vkCreateSampler(device.getDevice(), &samplerInfo, nullptr,
                      &noiseSampler) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture sampler");
  }

  VkDescriptorSetLayout graphicsLayout = pipeline->getDescriptorSetLayout();
  VkDescriptorSetAllocateInfo allocInfoGraphics{};
  allocInfoGraphics.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfoGraphics.descriptorPool = device.getDescriptorPool();
  allocInfoGraphics.descriptorSetCount = 1;
  allocInfoGraphics.pSetLayouts = &graphicsLayout;

  if (vkAllocateDescriptorSets(device.getDevice(), &allocInfoGraphics,
                               &graphicDescriptorSet) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate graphics descriptor set");
  }

  VkDescriptorImageInfo graphicsImageInfo{};
  graphicsImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  graphicsImageInfo.imageView = noiseImageView;
  graphicsImageInfo.sampler = noiseSampler;

  VkWriteDescriptorSet graphicsDescriptorWrite{};
  graphicsDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  graphicsDescriptorWrite.dstSet = graphicDescriptorSet;
  graphicsDescriptorWrite.dstBinding = 0;
  graphicsDescriptorWrite.dstArrayElement = 0;
  graphicsDescriptorWrite.descriptorType =
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // to read
  graphicsDescriptorWrite.descriptorCount = 1;
  graphicsDescriptorWrite.pImageInfo = &graphicsImageInfo;

  vkUpdateDescriptorSets(device.getDevice(), 1, &graphicsDescriptorWrite, 0,
                         nullptr);
}

void makeApp::drawFrame() {
  // Wait for the GPU to finish the previous frame
  vkWaitForFences(device.getDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);
  vkResetFences(device.getDevice(), 1, &inFlightFence);

  uint32_t imageIndex;
  vkAcquireNextImageKHR(device.getDevice(), swapChain.getSwapChain(),
                        UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE,
                        &imageIndex);

  vkResetCommandBuffer(commandBuffer, 0);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  // --- Dynamic Rendering Transition ---
  VkRenderingAttachmentInfo colorAttachment{};
  colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  colorAttachment.imageView = swapChain.getImageView(imageIndex);
  colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.clearValue = {{{0.01f, 0.01f, 0.1f, 1.0f}}}; // Dark blue-ish

  VkRenderingInfo renderingInfo{};
  renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  renderingInfo.renderArea = {{0, 0}, swapChain.getExtent()};
  renderingInfo.layerCount = 1;
  renderingInfo.colorAttachmentCount = 1;
  renderingInfo.pColorAttachments = &colorAttachment;

  // Transition image layout manually for Dynamic Rendering
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = swapChain.getImage(imageIndex);
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.srcAccessMask = 0;
  barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);

  vkCmdBeginRendering(commandBuffer, &renderingInfo);

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipeline->getGraphicsPipeline());
  // noise
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipeline->getPiplineLayout(), 0, 1,
                          &graphicDescriptorSet, 0, nullptr);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(swapChain.getExtent().width);
  viewport.height = static_cast<float>(swapChain.getExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = swapChain.getExtent();
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  Camera cameraData = camera.getCameraPushConstants();
  DynamicScene sceneData{};
  sceneData.time = static_cast<float>(glfwGetTime()) - startTime;

  PushConstants constants{};
  constants.cemra = cameraData;
  constants.dynamicScene = sceneData;

  vkCmdPushConstants(commandBuffer, pipeline->getPiplineLayout(),
                     VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants),
                     &constants);
  vkCmdDraw(commandBuffer, 3, 1, 0, 0);

  vkCmdEndRendering(commandBuffer);
  barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  barrier.dstAccessMask = 0;

  vkCmdPipelineBarrier(commandBuffer,
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                       VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);

  vkEndCommandBuffer(commandBuffer);

  // --- Submit to Queue ---
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

  vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, inFlightFence);

  // --- Present to Screen ---
  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
  VkSwapchainKHR swapChains[] = {swapChain.getSwapChain()};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;

  vkQueuePresentKHR(device.getPresentQueue(), &presentInfo);
}

// hot reload logic
void makeApp::checkShaderUpdate() {
  try {
    auto currentWriteTime =
        std::filesystem::last_write_time("shaders/raymarch.frag");
    if (currentWriteTime > lastShaderWriteTime) {
      lastShaderWriteTime = currentWriteTime;
      std::cout << "Shader has been changed. Compiling... (Hot reaload)"
                << "\n";
      reloadShader();
    }

  } catch (const std::filesystem::filesystem_error &e) {
  }
}

void makeApp::reloadShader() {
  vkDeviceWaitIdle(device.getDevice());
  // call cmake
  int compiled = std::system("cmake --build build --target CompileShaders");
  if (compiled != 0) {
    std::cerr << "Compilation failed (Hot Reload)\n";
    return;
  }

  pipeline.reset();
  pipeline = std::make_unique<PipelineEngine>(device, cachedConfigInfo);

  std::cout << "Pipeline successfully rebuilt!\n";
}

} // namespace miniEngine