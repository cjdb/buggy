# Copyright (c) Christopher Di Bella.
# SPDX-License-Identifier: Apache-2.0
#
cxx_library(
  TARGET window
  FILENAME window.cpp
  LINK_TARGETS glfw glm::glm
  DEFINITIONS GLFW_INCLUDE_VULKAN
)
cxx_library(
  TARGET vulkan_graphics
  FILENAME vulkan.cpp
  LINK_TARGETS Vulkan::Vulkan cjdb::constexpr-contracts
  DEFINITIONS GLFW_INCLUDE_VULKAN BUGGY_VULKAN_GRAPHICS
)
cxx_binary(
  TARGET xtest
  FILENAME test.cpp
  LINK_TARGETS glfw glm::glm Vulkan::Vulkan window vulkan_graphics
  DEFINITIONS GLFW_INCLUDE_VULKAN BUGGY_VULKAN_GRAPHICS
)
