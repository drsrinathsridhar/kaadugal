CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "possible configurations" FORCE)
# In case the user does not setup CMAKE_BUILD_TYPE, assume it's Debug
IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING "build type default to Debug, set to Release to improve performance" FORCE)
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")


# Project mergetrees for merging trees without knowing anything else
SET(PROJECT3 mergetrees)
PROJECT(${PROJECT3})
# OpenMP
FIND_PACKAGE(OpenMP)
IF(OPENMP_FOUND)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  MESSAGE(STATUS "Found OpenMP and finished setting up appropriate compiler flags for C and C++")
ELSE()
  MESSAGE(WARNING "OpenMP NOT found. Parallelization features will not be available.")
ENDIF()

IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") # GCC on Linux
  # We need C++11 support
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -O3") # Enable lots of warning flags and optimization
  IF(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x") # C++11 support on gcc v 4.7--
  ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") # C++11 support on gcc v 4.7+
  ENDIF()
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel") # Intel C++ compiler
  # We need C++11 support
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wno-unused-parameter -O2") # Enable lots of warning flags and optimization
  # Other optimizations. See https://software.intel.com/en-us/articles/step-by-step-optimizing-with-intel-c-compiler
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -xAVX -ipo")
  #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -xAVX -ipo -prof-gen -prof-dir=.")
  #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -xAVX -ipo -prof-use -prof-dir=.")
ENDIF()
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../include)
SET(CPP_FILES mergetrees/mergetrees.cpp)
ADD_EXECUTABLE(${PROJECT3} ${CPP_FILES})
TARGET_LINK_LIBRARIES(${PROJECT3})
