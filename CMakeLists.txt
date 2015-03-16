cmake_minimum_required(VERSION 2.8.11)
cmake_policy(SET CMP0020 NEW)

project(objed-dev)

# Set global CMake properties
# -----------------------------------------------------------------------
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmake targets")
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# Determine the platform
# -----------------------------------------------------------------------
if(WIN32)
  set(PLATFORM_NAME "win")
elseif(UNIX)
  set(PLATFORM_NAME "unix")
else()
  message(fatal_error "Unsupported platform")
endif()

# Determine platform bits
# -----------------------------------------------------------------------
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(PLATFORM_BITNESS "32")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(PLATFORM_BITNESS "64")
else()
  message(fatal_error "Unsupported bitness")
endif()

# Platform is the pair of system and its bitness
# -----------------------------------------------------------------------
set(PLATFORM "${PLATFORM_NAME}${PLATFORM_BITNESS}")
message( "Platform is ${PLATFORM}")

# Configuration
# -----------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "release")
endif()
# -----------------------------------------------------------------------

# C Runtime Environment.
# -----------------------------------------------------------------------
if(MSVC)
  set(crt "vc")
elseif(CMAKE_COMPILER_IS_GNUCXX)
  set(crt "gnu")
elseif(MAC_OS OR APPLE)
  set(CRT "gnu")
else()
  message(fatal_error "Unsupported CRT")
endif()

# Setting output directories
# -----------------------------------------------------------------------

# Runtime output directory
# -----------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY         "${CMAKE_SOURCE_DIR}/bin.${PLATFORM}.${CMAKE_BUILD_TYPE}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/bin.${PLATFORM}.debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin.${PLATFORM}.release")

# Library output directory
# -----------------------------------------------------------------------
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY         "${CMAKE_SOURCE_DIR}/bin.${PLATFORM}.${CMAKE_BUILD_TYPE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/bin.${PLATFORM}.debug")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin.${PLATFORM}.release")

# Archive output directory
# -----------------------------------------------------------------------
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY         "${CMAKE_SOURCE_DIR}/lib/${PLATFORM}.${crt}/${CMAKE_BUILD_TYPE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/lib/${PLATFORM}.${crt}/debug")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/lib/${PLATFORM}.${crt}/release")

# Setup special definitions
# -----------------------------------------------------------------------
if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE)
endif(MSVC)

# Find Packages
# -----------------------------------------------------------------------
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Concurrent REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Concurrent_INCLUDE_DIRS})

set(OpenCV_STATIC OFF)
find_package(OpenCV REQUIRED)

find_package(OpenMP REQUIRED)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

# Add project collections to build tree
# -----------------------------------------------------------------------
include_directories(${CMAKE_SOURCE_DIR}/prj.thirdparty)
add_subdirectory(prj.thirdparty)

include_directories(${CMAKE_SOURCE_DIR}/prj.objed)
include_directories(${CMAKE_BINARY_DIR}/prj.objed)
add_subdirectory(prj.objed)