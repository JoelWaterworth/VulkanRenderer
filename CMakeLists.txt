cmake_minimum_required(VERSION 3.1)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wextra -Wundef")
endif(CMAKE_COMPILER_IS_GNUCXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")

project (VulkanRender)

if(NOT WIN32)
	include_directories(/usr/local/include)
	include_directories(/usr/include)
ENDIF()

if(WIN32)
	find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan HINTS "$ENV{VULKAN_SDK}/lib")
		IF (Vulkan_LIBRARY)
				set(Vulkan_FOUND ON)
				MESSAGE("Using bundled Vulkan library version")
				include_directories( $ENV{VULKAN_SDK}/Include)
				link_libraries(${Vulkan_LIBRARY})
		ENDIF()
ELSE(WIN32)
	find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib")
	IF (Vulkan_LIBRARY)
		set(Vulkan_FOUND ON)
		MESSAGE("Vulkan library is found")
		include_directories( $ENV{VULKAN_SDK}/include)
		link_libraries(${Vulkan_LIBRARY})
	ENDIF()
ENDIF()

IF (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
ENDIF()

if (WIN32)
	find_library(ASSIMP_LIBRARIES NAMES assimp assimp HINTS ${CMAKE_SOURCE_DIR}/lib)
	IF(ASSIMP_LIBRARIES)
		set(Assimp_FOUND ON)
		MESSAGE(${ASSIMP_LIBRARIES})
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
		link_libraries(${ASSIMP_LIBRARIES})
	ENDIF()	
ELSE(WIN32)
	find_library(ASSIMP_LIBRARIES NAMES libassimp assimp HINTS /usr/local/lib)
	find_package(Threads REQUIRED)
	IF(ASSIMP_LIBRARIES)
	  set(Assimp_FOUND ON)
	  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_XLIB_KHR")
	ENDIF()
ENDIF()

IF (NOT Assimp_FOUND)
	message(FATAL_ERROR "Could not find Assimp library!")
ELSE()
	message(STATUS ${ASSIMP_LIBRARIES})
ENDIF()

add_definitions(-D_DEBUG)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DVK_PROTOTYPES)

file(GLOB SOURCES
    "src/*.cpp"
    "src/vulkanRender/*.cpp"
    "src/vulkanRender/resources/*.cpp"
)

file(GLOB HEADERS
    "include/*.h"
    "include/vulkanRender/*.h"
    "include/vulkanRender/resources/*.h"
)

set (INCLUDE_DIRS "")
foreach (_headerFile ${HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)

include_directories(
    include
    include/vulkanRender
    include/vulkanRender/resources
)

include_directories(
  external
  external/stb
  external/glm
  external/gli
  external/assimp
)

IF(WIN32)
	add_executable(VulkanRender ${SOURCES})
	target_link_libraries(VulkanRender ${Vulkan_LIBRARY} ${ASSIMP_LIBRARIES} ${WINLIBS})
ELSE(WIN32)
  find_package(X11 REQUIRED)
  include_directories(${X11_INCLUDE_DIR})
	add_executable(VulkanRender ${SOURCES})
	target_link_libraries(VulkanRender  ${X11_LIBRARIES} ${XLIB_LIBRARIES} ${Vulkan_LIBRARY} ${ASSIMP_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}  stdc++fs)
ENDIF(WIN32)
