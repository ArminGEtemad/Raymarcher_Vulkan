#include "mainLoop.hpp"
#include "swapchain.hpp"
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vulkan/vulkan_core.h>
namespace miniEngine {

makeApp::makeApp() {
  PipelineConfigInfo configInfo{};

  PipelineEngine::defaultPipelineConfigInfo(configInfo);
  configInfo.colorAttachmentFormat = swapChain.getFormat();

  pipeline = std::make_unique<PipelineEngine>(device, configInfo);

  allocateCommandBuffer();
  createSyncObjects();
}
makeApp::~makeApp() {
  vkDestroySemaphore(device.getDevice(), renderFinishedSemaphore, nullptr);
  vkDestroySemaphore(device.getDevice(), imageAvailableSemaphore, nullptr);
  vkDestroyFence(device.getDevice(), inFlightFence, nullptr);
}

void makeApp::run() {
  while (!createWindow.shouldClose()) {
    glfwPollEvents();
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

void makeApp::drawFrame() {
  // Wait for the GPU to finish the previous frame
  vkWaitForFences(device.getDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);
  vkResetFences(device.getDevice(), 1, &inFlightFence);

  uint32_t imageIndex;
  vkAcquireNextImageKHR(device.getDevice(), swapChain.getSwapChain(),
                        UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE,
                        &imageIndex);

  vkResetCommandBuffer(commandBuffer, 0);

  VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
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

  auto pushConstants = camera.getCameraPushConstants();
  vkCmdPushConstants(commandBuffer, pipeline->getPiplineLayout(),
                     VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                     sizeof(CameraPushConstants), &pushConstants);
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
} // namespace miniEngine