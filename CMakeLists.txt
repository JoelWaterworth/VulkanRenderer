cmake_minimum_required(VERSION 3.1)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

project (VulkanRender)

include_directories(external)
include_directories(external/stb)
include_directories(external/glm)
include_directories(external/assimp)
include_directories(external/gli)
if(NOT WIN32)
	include_directories(/usr/local/include)
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
	IF (NOT Vulkan_LIBRARY)
		find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib")
		IF (Vulkan_LIBRARY)
			set(Vulkan_FOUND ON)
			MESSAGE("Vulkan library is found")
			include_directories( $ENV{VULKAN_SDK}/include)
			link_libraries(${Vulkan_LIBRARY})
		ENDIF()
	ENDIF()
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
	find_package(ASSIMP REQUIRED)
	find_package(Threads REQUIRED)
	include_directories(${ASSIMP_INCLUDE_DIRS})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_XLIB_KHR")
ENDIF()

IF (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
ELSE()
	message(STATUS ${Vulkan_LIBRARY})
ENDIF()

IF (NOT Assimp_FOUND)
	message(FATAL_ERROR "Could not find Assimp library!")
ELSE()
	message(STATUS ${ASSIMP_LIBRARIES})
ENDIF()

add_definitions(-D_DEBUG)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DVK_PROTOTYPES)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wextra -Wundef")
endif(CMAKE_COMPILER_IS_GNUCXX)

find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})

file(GLOB SOURCES
    "src/*.cpp"
    "src/vulkanRender/*.cpp"
    "src/vulkanRender/resources/*.cpp"
)

include_directories(
    include
    include/vulkanRender
    include/vulkanRender/resources
)

IF(WIN32)
	cmake_minimum_required(VERSION 3.1)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

project (VulkanRender)

include_directories(external)
include_directories(external/stb)
include_directories(external/glm)
include_directories(external/gli)
include_directories(external/assimp)
if(NOT WIN32)
	include_directories(/usr/local/include)
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
	IF (NOT Vulkan_LIBRARY)
		find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib")
		IF (Vulkan_LIBRARY)
			set(Vulkan_FOUND ON)
			MESSAGE("Vulkan library is found")
			include_directories( $ENV{VULKAN_SDK}/include)
			link_libraries(${Vulkan_LIBRARY})
		ENDIF()
	ENDIF()
ENDIF()

if (WIN32)
	find_library(ASSIMP_LIBRARIES NAMES assimp HINTS "${CMAKE_SOURCE_DIR}/lib")
	IF(ASSIMP_LIBRARIES)
		set(Assimp_FOUND ON)
		MESSAGE(${ASSIMP_LIBRARIES})
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
	ENDIF()	
ELSE(WIN32)
	find_package(ASSIMP REQUIRED)
	find_package(Threads REQUIRED)
	include_directories(${ASSIMP_INCLUDE_DIRS})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_XLIB_KHR")
ENDIF()

IF (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
ELSE()
	message(STATUS ${Vulkan_LIBRARY})
ENDIF()

IF (NOT Assimp_FOUND)
	message(FATAL_ERROR "Could not find Assimp library!")
ELSE()
	message(STATUS ${ASSIMP_LIBRARIES})
ENDIF()

add_definitions(-D_DEBUG)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DVK_PROTOTYPES)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wextra -Wundef")
endif(CMAKE_COMPILER_IS_GNUCXX)

find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})

file(GLOB SOURCES
    "src/*.cpp"
    "src/vulkanRender/*.cpp"
    "src/vulkanRender/resources/*.cpp"
)

include_directories(
    include
    include/vulkanRender
    include/vulkanRender/resources
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")

IF(WIN32)
	add_executable(VulkanRender ${SOURCES})
	target_link_libraries(VulkanRender ${Vulkan_LIBRARY} ${ASSIMP_LIBRARIES} ${WINLIBS})
ELSE(WIN32)
	add_executable(VulkanRender ${SOURCES})
	target_link_libraries(VulkanRender ${ASSIMP_LIBRARIES}  stdc++fs)
	link_libraries(${XLIB_LIBRARIES} ${Vulkan_LIBRARY} ${Vulkan_LIBRARY} ${ASSIMP_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
ENDIF(WIN32)

ELSE(WIN32)
target_link_libraries(VulkanRender
  ${ASSIMP_LIBRARIES}  stdc++fs)
	link_libraries(${XLIB_LIBRARIES} ${Vulkan_LIBRARY} ${Vulkan_LIBRARY} ${ASSIMP_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
ENDIF(WIN32)