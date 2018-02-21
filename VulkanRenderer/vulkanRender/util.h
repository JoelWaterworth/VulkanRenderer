#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>

#define VK_CHECK_RESULT(f)																				\
{																										\
	vk::Result res = (f);																					\
	if (res != vk::Result::eSuccess)																				\
	{																									\
		std::cout << "Fatal : VkResult is \"" << res << "\" in " << __FILE__ << " at line " << __LINE__ << std::endl; \
		assert(res == vk::Result::eSuccess);																		\
	}																									\
}