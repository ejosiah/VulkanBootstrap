#include "AppState.hpp"

uint32 AppState::screenWidth = 0;
uint32 AppState::screenHeight = 0;
uint32 AppState::colorBufferCount = 0;
VkFormat AppState::screenFormat = VK_FORMAT_UNDEFINED;
VkFormat AppState::screenDepthFormat = VK_FORMAT_UNDEFINED;
uint32 AppState::numFramesInFlight = 0;
VkSampleCountFlagBits AppState::screenSampleCount = VK_SAMPLE_COUNT_1_BIT;
uint32 AppState::currentFrame = 0;